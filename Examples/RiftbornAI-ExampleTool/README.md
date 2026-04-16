# RiftbornAI Example Tool

Minimal reference plugin for authoring community tools on top of
[RiftbornAI](https://github.com/RiftbornGit/RiftbornAI). Read this alongside
[`docs/TOOL_AUTHORING.md`](../../docs/TOOL_AUTHORING.md).

## What it does

Registers two tools into the RiftbornAI tool registry so they appear in any
connected MCP client:

| Tool | Purpose |
|---|---|
| `example_echo` | Echoes a `message` string back. Demonstrates the minimal read-only tool. |
| `example_get_project_name` | Returns the active project name via `FApp::GetProjectName()`. |

Both are `Risk = Safe`, `Cost = Cheap`, background-thread safe. Neither
mutates project state.

## Try it

```
1. Copy this folder into <YourProject>/Plugins/
2. Regenerate project files (right-click your .uproject → "Generate Visual Studio project files")
3. Build the editor target. RiftbornAI and RiftbornAIExampleTool will be compiled together.
4. Launch the editor. In the output log, look for:
       LogRiftbornExample: RiftbornAI Example Tools: registering 2 tools
5. Connect an MCP client and ask it to list tools — both example_* entries appear.
```

## File layout

```
RiftbornAI-ExampleTool/
├── RiftbornAIExampleTool.uplugin    ← declares dependency on RiftbornAI
└── Source/RiftbornAIExampleTool/
    ├── RiftbornAIExampleTool.Build.cs
    ├── Public/RiftbornAIExampleTool.h
    └── Private/RiftbornAIExampleTool.cpp   ← registration + tool handlers
```

All of the SDK surface you need is reached by `#include "ClaudeToolUse.h"`
and `#include "ToolModuleBase.h"`.

## Next steps

Extend `RegisterTools()` with your own `FClaudeTool` entries. For tools that
mutate the project, bump `Risk` to `Mutation` and declare an `UndoStrategy`
(see section 6 of `TOOL_AUTHORING.md`).
