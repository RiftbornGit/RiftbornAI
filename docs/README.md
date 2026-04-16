# RiftbornAI Documentation Index

Public documentation for the UE 5.7 editor plugin and companion MCP surface.

This index covers the docs that ship in the public beta package and public
repository. Maintainer-only architecture and workflow notes are intentionally
left out.

## Start Here

| Document | Description |
| -------- | ----------- |
| [GETTING_STARTED.md](GETTING_STARTED.md) | Install, configure, and run the first workflow |
| [BETA_STATUS.md](BETA_STATUS.md) | Current beta scope, limitations, and known issues |
| [USER_TUTORIAL.md](USER_TUTORIAL.md) | Practical user-facing walkthroughs |
| [CONFIGURATION_REFERENCE.md](CONFIGURATION_REFERENCE.md) | Settings, bridge config, and environment variables |

## Product And Integration

| Document | Description |
| -------- | ----------- |
| [../README.md](../README.md) | Public project overview |
| [GOVERNANCE_AND_SECURITY.md](GOVERNANCE_AND_SECURITY.md) | Risk tiers, proofs, and confirmation rules |
| [LLM_PROVIDERS.md](LLM_PROVIDERS.md) | Supported providers and failover policy |
| [READINESS_TRUTH.md](READINESS_TRUTH.md) | Canonical readiness-truth contract and downstream chain |
| [TOOL_AUTHORING.md](TOOL_AUTHORING.md) | SDK surface for third-party tool plugins |
| [../Bridge/toolbook/public_surface.json](../Bridge/toolbook/public_surface.json) | Shipped tool-surface membership manifest |
| [../Bridge/toolbook/contracts.json](../Bridge/toolbook/contracts.json) | Runtime contract bundle shipped with the beta |
| [../mcp-server/README.md](../mcp-server/README.md) | MCP server setup and runtime behavior |

## AI School

`docs/AI_School/` is the domain playbook set used by the editor agent and by
human operators. Most tracks follow the same structure:

1. `01` concepts and design goals
2. `02` UE systems and supported surfaces
3. `03` anti-patterns
4. `04` workflow
5. `05` tool guide

Track folders:

- [AI_School/](AI_School/): environment baseline track
- [AI_School/Blueprint/](AI_School/Blueprint/): Blueprint authoring
- [AI_School/GAS/](AI_School/GAS/): gameplay ability system
- [AI_School/UI/](AI_School/UI/): widgets and HUD flows
- [AI_School/Audio/](AI_School/Audio/): audio and MetaSound
- [AI_School/Animation/](AI_School/Animation/): animation and retargeting
- [AI_School/Input/](AI_School/Input/): Enhanced Input
- [AI_School/Cinematics/](AI_School/Cinematics/): Sequencer and cameras
- [AI_School/Physics/](AI_School/Physics/): collision, constraints, destruction
- [AI_School/Streaming/](AI_School/Streaming/): World Partition and data layers
- [AI_School/Networking/](AI_School/Networking/): replication and Iris
- [AI_School/SaveLoad/](AI_School/SaveLoad/): snapshots and restore flows
- [AI_School/Localization/](AI_School/Localization/): String Tables and text safety
- [AI_School/Modeling/](AI_School/Modeling/): geometry and mesh hardening
- [AI_School/LevelDesign/](AI_School/LevelDesign/): maps, blockouts, navigation
- [AI_School/CppArchitecture/](AI_School/CppArchitecture/): gameplay C++ boundaries
- [AI_School/VFX/](AI_School/VFX/): Niagara and gameplay VFX

Cross-track process docs:

- [AI_School/TASK_INTAKE_AND_PREFLIGHT.md](AI_School/TASK_INTAKE_AND_PREFLIGHT.md)
- [AI_School/TRACK_SELECTION.md](AI_School/TRACK_SELECTION.md)
- [AI_School/VERIFICATION_LADDER.md](AI_School/VERIFICATION_LADDER.md)
- [AI_School/TASK_PLAYBOOKS.md](AI_School/TASK_PLAYBOOKS.md)
- [AI_School/COVERAGE_BOUNDARIES.md](AI_School/COVERAGE_BOUNDARIES.md)

## Package, Legal, And Release Docs

| Document | Description |
| -------- | ----------- |
| [../CHANGELOG.md](../CHANGELOG.md) | Release notes |
| [../CONTRIBUTING.md](../CONTRIBUTING.md) | Contribution workflow |
| [../SECURITY.md](../SECURITY.md) | Security disclosure policy |
| [../EULA.md](../EULA.md) | Licensing notice |
| [../COMMERCIAL_LICENSE.md](../COMMERCIAL_LICENSE.md) | Commercial license guidance |
| [../Setup/README.md](../Setup/README.md) | Installer and uninstall behavior |
| [../Examples/RiftbornAI-ExampleTool/README.md](../Examples/RiftbornAI-ExampleTool/README.md) | Example third-party tool plugin |
