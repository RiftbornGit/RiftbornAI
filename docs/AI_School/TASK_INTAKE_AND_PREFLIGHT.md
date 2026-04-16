## AI School Task Intake And Editor Preflight

Use this before any non-trivial mutating task.

This document is the entrypoint into AI School when the agent still does not know:

- what the real mutation is
- which tracks should be loaded
- which editor surface should be used
- what proof will be required before the task is done

It exists to stop blind tool execution.

## Step 1: State The Real Mutation

Do not start from the user's theme word alone.

State the concrete mutation:

- what asset or world state will change
- what player-facing behavior or read should improve
- what system contract could break

Examples:

- not "make combat feel better"
- but "edit a dash ability, its cooldown UI, and impact feedback"

- not "improve the forest"
- but "reshape traversal flow in a forest clearing, then redress terrain and ambience"

If you cannot name the real mutation, you are not ready to choose tools.

## Step 2: Check For Editor Blockers Early

Before planning tool calls, check whether the editor is even in a safe state to accept them.

Minimum preflight:

- use `get_editor_gui_state`, or inspect the `gui` block from `observe_ue_project`
- watch for modal dialogs, restore prompts, save prompts, memory pressure warnings, and menu stacks
- if the task is asset-editor specific, confirm the correct editor surface is active before editing

If there is a blocker, clear it before continuing.

## Step 3: Choose The Actual Mutation Lane

Different tasks may touch the same feature from different implementation lanes.

Choose the lane deliberately:

- world-building lane
- modeling / Geometry Script lane
- asset-editor lane
- Blueprint lane
- C++ lane
- Sequencer lane
- UI/widget lane
- localization/String Table lane

Examples:

- an ability can be mostly `GAS` + `Blueprint` or `GAS` + `CppArchitecture`
- a cinematic can be mostly `Cinematics`, but may need `LevelDesign` if the space itself is still wrong
- a playable area may be mostly `LevelDesign`, `Environment`, and `Streaming`, not just art dressing

If the lane is uncertain, stop and inspect the current editor context first.

## Step 4: Load The Smallest Correct Track Set

After the lane is clear:

1. read `TRACK_SELECTION.md`
2. load the primary domain track
3. add only the supporting craft and contract tracks the task actually mutates
4. read `TASK_PLAYBOOKS.md` if the feature matches a common multi-domain slice
5. read `COVERAGE_BOUNDARIES.md` if the request mentions design-system names that may not have a public tool lane

Do not load tracks just because they are adjacent to the feature in a general sense.

## Step 5: Verify The Tool Surface Before Promising A Route

The live registry is authoritative.

Before committing to a workflow:

- use `list_all_tools`
- use `describe_tool`
- prefer the exact registered tool names
- verify whether a helper is on the public lane, beta-gated, or absent

Do not infer a callable workflow from:

- historical docs
- design-system names
- remembered tool aliases

## Step 6: Define The Proof Plan Up Front

Before the first mutating call, say what proof will make the task complete.

Use `VERIFICATION_LADDER.md` and choose the smallest sufficient proof set.

Typical proof plan questions:

1. what structure must be inspected before editing?
2. what compile or asset-health gate applies?
3. what in-world proof is needed?
4. what runtime interaction proof is mandatory?
5. what contract proof is required?
6. does this task need performance or regression proof?

If you cannot answer those questions, you are probably starting too early.

## Step 7: Identify Stop Conditions Before Building

Stop and reassess if any of these are true:

- the editor has unresolved GUI blockers
- the asset or map in focus is not the intended one
- the tool route is uncertain
- the task implies a design-system lane that is not on the verified public surface
- the proof plan is missing runtime or contract validation where the failure mode clearly lives
- the task crosses more domains than you initially loaded

Stopping early is cheaper than repairing blind mutations.

## Suggested Intake Template

Use a short internal checklist:

1. real mutation
2. editor preflight status
3. implementation lane
4. tracks to load
5. exact tool route to verify
6. proof plan
7. stop conditions

The output does not need to be verbose. It only needs to be explicit enough that the next tool call is defensible.

## Common Bad Starts

- using a tool before confirming the edited asset or map
- assuming a helper exists because the domain name exists in docs
- planning proof after the edits instead of before them
- loading too many tracks and still not knowing which asset is actually changing
- treating compile success as the finish line for runtime-facing work

## Operational Rule

No substantial editor mutation should begin without:

- blocker check
- lane choice
- track choice
- tool-surface check
- proof plan

That is the minimum safe intake bar for AI School work.
