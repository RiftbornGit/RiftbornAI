# RiftbornAI Rule Templates

These are optional rule files you can copy into your project to guide AI behavior.

## How to use

1. Create a `Config/RiftbornAI/Rules/` directory in your project root
2. Copy any templates you want into that directory
3. Edit them to match your project's conventions
4. The AI will read them on the next session (or run `RiftbornAI.ReloadProjectRules` in the console)

## Available templates

| Template | Description |
|----------|-------------|
| `environment_workflow.md` | Bottom-up environment building, foliage instancing, terrain-first workflow |
| `verification_ladder.md` | Multi-stage verification: structure → compile → in-world → runtime |
| `preflight_checklist.md` | Pre-task validation: state the mutation, check editor state, pick the lane |
| `code_standards.md` | C++ and Blueprint conventions (customize to your project) |

## Writing your own rules

Rules are plain `.md` files. Write them as instructions to the AI:

```markdown
# Our Combat System Rules

- All damage calculations go through UDamageSubsystem — never apply damage directly
- GAS abilities must have cooldown GameplayEffects, never timer-based cooldowns
- Combo windows use AnimNotifyState, not hardcoded frame counts
```

Keep rules specific and actionable. The AI treats them as strong guidance
that the user can override explicitly.

## Settings

In Project Settings > Plugins > RiftbornAI > Project Rules:
- **Enable Project Rules**: Toggle injection on/off
- **Project Rules Directory**: Change the rules directory path
- **Max Characters**: Budget cap to prevent context window bloat (default: 16,000)
