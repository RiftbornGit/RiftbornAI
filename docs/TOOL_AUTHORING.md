# RiftbornAI — Tool Authoring SDK

**Audience**: plugin developers shipping **new tools** on top of the RiftbornAI
editor plugin. First-party contributors editing this repo should read
[`TOOL_SYSTEM.md`](TOOL_SYSTEM.md) instead.

**Status**: SDK surface v1 (schema version `FClaudeTool::CurrentSchemaVersion = 1`).

---

## 1. How it works

Your plugin is a regular Unreal plugin with a runtime module that depends on
`RiftbornAI`. On `PostEngineInit` you register one or more tools with the
core registry. The RiftbornAI MCP server already running in the editor picks
them up on its next `/tools` fetch — no server rebuild, no core edits.

```
Your .uplugin
  └── Source/YourTools/ (runtime module)
        └── StartupModule()
              └── FClaudeToolRegistry::Get().RegisterTool(Tool, Handler)
                    │
                    ▼
              RiftbornAI core dispatches calls from Claude/MCP to Handler
```

---

## 2. Supported SDK surface

These are the **only** headers whose stability we guarantee across minor
releases. Everything else under `Public/` is internal and may change.

| Header | Purpose |
|---|---|
| `Core/ClaudeToolUse.h` | Re-exports the types + registry. Include this one header in your tool `.cpp`. |
| `Core/ClaudeToolUse_Types.h` | `FClaudeTool`, `FClaudeToolParameter`, `FClaudeToolCall`, `FClaudeToolResult`, risk/cost/visibility enums. |
| `Core/ClaudeToolUse_Registry.h` | `FClaudeToolRegistry::Get()` and `RegisterTool`. |
| `Tools/ToolModuleBase.h` | `IToolModule` interface, `TToolModuleBase<T>` helper, `DECLARE_TOOL_MODULE` macro. |
| `Agent/IAIProvider.h` | Optional: implement a custom LLM provider. |

Headers inside `Public/Agent/`, `Public/Governance/`, `Public/Bridge/`,
`Public/Providers/` that are **not listed above** are implementation-visible
but **not part of the SDK contract** — do not include them from your plugin.

---

## 3. Minimal example

### 3.1 .uplugin manifest

```json
{
  "FileVersion": 3,
  "Version": 1,
  "VersionName": "0.1.0",
  "FriendlyName": "My RiftbornAI Tools",
  "EngineVersion": "5.7.0",
  "Plugins": [
    { "Name": "RiftbornAI", "Enabled": true }
  ],
  "Modules": [
    {
      "Name": "MyRiftbornTools",
      "Type": "Editor",
      "LoadingPhase": "PostEngineInit"
    }
  ]
}
```

`LoadingPhase: PostEngineInit` is required — it guarantees RiftbornAI is
loaded and the registry is live before your `StartupModule` runs.

### 3.2 Module Build.cs

```csharp
using UnrealBuildTool;

public class MyRiftbornTools : ModuleRules
{
    public MyRiftbornTools(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine" });
        PrivateDependencyModuleNames.AddRange(new[] { "RiftbornAI" });
    }
}
```

### 3.3 Module startup + one tool

```cpp
#include "Modules/ModuleManager.h"
#include "ClaudeToolUse.h"

static FClaudeToolResult Tool_Echo(const FClaudeToolCall& Call)
{
    const FString* Msg = Call.Arguments.Find(TEXT("message"));
    return FClaudeToolResult::Success(
        FString::Printf(TEXT("echo: %s"), Msg ? **Msg : TEXT(""))
    );
}

class FMyRiftbornToolsModule : public IModuleInterface
{
public:
    virtual void StartupModule() override
    {
        FClaudeTool Tool;
        Tool.Name          = TEXT("my_echo");
        Tool.Description   = TEXT("Echo a message back. Demonstrates a read-only tool.");
        Tool.Category      = TEXT("Example");
        Tool.Visibility    = EToolVisibility::Public;
        Tool.Risk          = EToolRisk::Safe;
        Tool.Cost          = EToolCost::Cheap;
        Tool.UndoStrategy  = EToolUndoStrategy::NoUndo;
        Tool.bGameThreadRequired = false; // pure string op, background-safe

        FClaudeToolParameter P;
        P.Name = TEXT("message"); P.Type = EClaudeToolParamType::String;
        P.Description = TEXT("Text to echo."); P.bRequired = true;
        Tool.Parameters.Add(P);

        FClaudeToolRegistry::Get().RegisterTool(
            Tool, FOnExecuteTool::CreateStatic(&Tool_Echo)
        );
    }
};

IMPLEMENT_MODULE(FMyRiftbornToolsModule, MyRiftbornTools)
```

Build, launch the editor, and `my_echo` appears in MCP clients automatically.

---

## 4. `FClaudeTool` field reference

Declared in [`ClaudeToolUse_Types.h`](../Source/RiftbornAI/Public/Core/ClaudeToolUse_Types.h).
Required fields: `Name`, `Description`. Strongly recommended: `Category`,
`Visibility`, `Risk`, `Cost`, `UndoStrategy`. Leaving `UndoStrategy` at
`NotDeclared` logs a warning on registration.

| Field | What it controls |
|---|---|
| `SchemaVersion` | Auto-set to `CurrentSchemaVersion`. Core rejects tools with a higher version than it understands. |
| `Visibility` | `Public` tools are exposed to LLMs; `Internal` is for tool-to-tool calls only. |
| `Risk` | Drives governance gates (`Safe`, `Elevated`, `Mutation`, `Dangerous`, `Destructive`). |
| `Cost` | Routing hint; also picks default timeout (`Cheap` 5s / `Moderate` 30s / `Expensive` 120s). |
| `TimeoutMs` | Override the cost-derived timeout. `0` = use default. |
| `UndoStrategy` | `NoUndo`, `InverseOperation` (+ `InverseToolName`), or `Snapshot` (UE transaction). |
| `bGameThreadRequired` | `true` for any code touching UObjects. `false` only for pure reads / background-safe work. |

`FOnExecuteTool` is a `DECLARE_DELEGATE_RetVal_OneParam(FClaudeToolResult, ..., const FClaudeToolCall&)`.
Return via `FClaudeToolResult::Success(...)` / `FClaudeToolResult::Failure(...)`.
Arguments arrive as `TMap<FString, FString>` with rich JSON in `RawArgumentsJson`.

---

## 5. Grouping tools with `IToolModule`

For more than a handful of tools, prefer the module pattern:

```cpp
#include "ToolModuleBase.h"

DECLARE_TOOL_MODULE(MyExample)
    static FClaudeToolResult Tool_Echo(const FClaudeToolCall& Call);
END_TOOL_MODULE()

void FMyExampleTools::RegisterTools(FClaudeToolRegistry& Registry)
{
    FClaudeTool T; /* ... fill schema ... */
    RegisterToolInternal(Registry, T, FOnExecuteTool::CreateStatic(&Tool_Echo));
}

// In StartupModule:
FToolModuleRegistry::Get().RegisterModule(MakeShared<FMyExampleTools>());
FToolModuleRegistry::Get().RegisterAllModuleTools(FClaudeToolRegistry::Get());
```

The macro generates the class, name accessor, and tool-count tracking for you.

---

## 6. Governance contract

Every call is dispatched through the governance pipeline. In PROOF mode the
core rejects mutating tools that:

- declare `Risk >= Mutation` without an `UndoStrategy`
- don't populate `FClaudeToolResult::Witness` with state-change evidence
- execute off the game thread when `bGameThreadRequired == true`

For your first tools, stay at `Risk = Safe` or `Elevated` with
`UndoStrategy = NoUndo`. Read-only tools pass governance trivially.

---

## 7. Schema version compatibility

```cpp
static constexpr int32 CurrentSchemaVersion = 1; // in FClaudeTool
```

If RiftbornAI core bumps `CurrentSchemaVersion` in a future release and your
tool was compiled against an older SDK, the registry logs a warning and the
tool is treated as `Visibility = Internal` (not exposed to LLMs) until you
recompile. Your plugin keeps loading — no hard break.

---

## 8. Distribution

- **Ship source or compiled**: your plugin can be open-source on GitHub or
  shipped as a binary-only plugin on Fab. RiftbornAI core doesn't care.
- **License**: your tool plugin is yours. The BSL 1.1 on RiftbornAI covers
  the engine, not plugins that merely depend on it.
- **MCP discovery is automatic**: as long as the user's RiftbornAI is running,
  an `/tools` fetch lists your tools with no extra setup.

For a complete working example, see the `RiftbornAI-ExampleTool` companion
plugin shipped alongside RiftbornAI.
