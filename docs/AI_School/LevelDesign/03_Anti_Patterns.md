# 03 — Anti-Patterns

Most bad levels are not ruined by missing detail. They are ruined by unreadable decisions.

## Anti-Pattern: Polishing Before The Blockout Works

Symptoms:

- lighting, props, and materials arrive before route flow is proven
- the team becomes attached to a space that is still wrong structurally

Why it is bad:

- visual investment makes needed layout change harder
- playability problems hide under decoration

Do instead:

- prove the blockout first
- use `generate_blockout`, manual blockout pieces, and playtest before art pass decisions

## Anti-Pattern: Branching Without Purpose

Symptoms:

- many paths exist but feel equivalent
- exploration produces confusion instead of choice

Why it is bad:

- larger maps feel smaller when decisions do not matter

Do instead:

- assign each route a clear gameplay job
- make branch differences legible through sightline, risk, speed, or reward

## Anti-Pattern: No Landmark Hierarchy

Symptoms:

- every room uses similar shape and focal emphasis
- players lose orientation after one or two turns

Why it is bad:

- the level stops teaching itself

Do instead:

- establish memorable anchors
- make the critical path and major hubs visually distinct

## Anti-Pattern: Flat Arena Syndrome

Symptoms:

- symmetrical open floor
- cover pieces repeated at one scale
- no meaningful high/low pressure zones

Why it is bad:

- encounters collapse into generic movement and trading space

Do instead:

- vary engagement bands
- create clear pushes, retreats, flanks, and focal pressure points

## Anti-Pattern: Corridor Chains With No Rhythm

Symptoms:

- room after room at similar size and intensity
- no compression/release pattern

Why it is bad:

- the level becomes monotonous even when technically functional

Do instead:

- alternate tight and open spaces
- stage reveals and safety pockets intentionally

## Anti-Pattern: Geometry That Looks Traversable But Is Not

Symptoms:

- stairs, ramps, edges, or doorways imply flow but fail in practice
- AI or player traversal breaks after geometry changes

Why it is bad:

- trust in the space collapses immediately

Do instead:

- rebuild and inspect navigation with `build_navmesh` and `get_navmesh_status`
- playtest movement, not just screenshots

## Anti-Pattern: Designing Only From Top-Down Or Beauty Angles

Symptoms:

- layout decisions come from plan view or dramatic camera framing only
- gameplay camera readability is ignored

Why it is bad:

- the player never sees the level from those idealized views

Do instead:

- review from likely gameplay camera heights and approach angles
- use `set_viewport_location`, `capture_viewport_sync`, and `look_at_and_capture`

## Anti-Pattern: Entrance And Exit Ambiguity

Symptoms:

- doors and route openings do not read as usable
- players hesitate or overshoot the intended path

Why it is bad:

- friction appears where there should be clarity

Do instead:

- differentiate usable routes with silhouette, light, framing, and sightline pull

## Anti-Pattern: Over-Reliance On Ad Hoc Placement

Symptoms:

- blockout is built from many unstructured one-off placements even when the repo has structured layout tools
- repeating a layout concept means rebuilding it manually

Why it is bad:

- slower iteration
- less consistency
- harder comparison between revisions

Do instead:

- use `generate_blockout` for supported room and arena shapes
- use `get_dungeon_layout` before `generate_dungeon` when planning procedural spaces

## Anti-Pattern: Declaring Success Before Movement Test

Symptoms:

- the layout looks good in stills
- nobody verifies traversal, combat spacing, or decision timing in motion

Why it is bad:

- level design quality is mostly felt over time, not seen in one frame

Do instead:

- use `run_quick_playtest`
- revise after actual movement and encounter pacing feedback

## Key Takeaway

The common level-design failure is not lack of content. It is lack of clarity.

Fight that by proving route meaning, landmark hierarchy, navigation, and pacing before polish.
