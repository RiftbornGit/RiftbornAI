# 03 — Anti-Patterns

## 1. Using Montages As The Main Locomotion System

Symptoms:

- locomotion driven by playing montages in sequence
- direction changes require cancelling one montage and starting another
- movement feels laggy and unresponsive

Why it is bad:

- montages are designed for discrete one-shots, not continuous parameter-driven blending
- no smooth speed/direction interpolation
- interruption rules fight against fluid locomotion needs

Do instead:

- use blend spaces for parameter-driven locomotion (speed, direction)
- use AnimBP state machines for locomotion state flow
- reserve montages for discrete actions that interrupt or overlay locomotion

## 2. Building A State Machine That Mirrors Every Clip

Symptoms:

- one state per animation asset
- 40+ states in a single state machine
- transition rules become an unmanageable web

Why it is bad:

- the graph represents a clip library, not gameplay logic
- debugging transitions requires tracing dozens of paths
- adding a new clip means rewiring the whole graph

Do instead:

- states should represent gameplay states, not individual clips
- use blend spaces inside states to handle variation
- keep state count under 10-15 per machine; use sub-state machines for deeper branching

## 3. Ignoring Starts, Stops, And Turns

Symptoms:

- character snaps to walk cycle instantly from idle
- stopping is an immediate blend to idle with no deceleration
- 180-degree turns happen without a pivot animation

Why it is bad:

- the character feels weightless and robotic
- players cannot predict movement commitment
- the quality gap is immediately obvious at gameplay camera distance

Do instead:

- author or retarget dedicated start, stop, and turn clips
- use transition states between idle and locomotion
- consider turn-in-place animations for large direction changes

## 4. Retargeting Before Rig Assumptions Are Clear

Symptoms:

- retarget runs before source and target bone hierarchies are compared
- chain mapping is guessed instead of verified
- output looks "close enough" in editor but breaks in motion

Why it is bad:

- bone-count or hierarchy differences create silent joint errors
- proportional differences produce wrong stride, reach, and posture
- fixing after the fact means redoing the retarget chain from scratch

Do instead:

- compare source and target skeletons explicitly
- set up IK rigs with matching chains before retargeting
- verify output on the actual gameplay character at gameplay distance

## 5. Calling A Retarget Successful Because It Imported

Symptoms:

- retarget completed without errors
- clips play on the target skeleton without crashing
- nobody reviewed the actual motion quality

Why it is bad:

- posture may be wrong (stiff character gets floppy source motion)
- stride length may mismatch the character's proportions
- hand placement, weapon alignment, or ground contact may be broken
- timing that worked on the source proportions looks off on the target

Do instead:

- review retargeted clips on the target character in gameplay context
- check ground contact, weapon alignment, and silhouette
- treat retarget as a starting point that needs visual approval, not a finished result

## 6. Verifying Animation Only In Asset Preview

Symptoms:

- all review happens in the animation editor or persona viewport
- clips look good zoomed in on the character
- gameplay feels wrong but nobody connects it to animation

Why it is bad:

- close-up preview hides foot sliding, weight problems, and transition pops
- camera distance changes what is readable
- animation that looks rich in preview can be muddy at gameplay distance

Do instead:

- verify at gameplay camera distance using `set_viewport_location` and `capture_viewport_sync`
- use `run_quick_playtest` to test motion in actual gameplay context
- judge transitions, not just individual clips

## 7. Building A Pose Search Database Without A Real Motion-Matching Plan

Symptoms:

- database created and populated but no animation graph consumes it
- clips added to the database without considering pose coverage
- motion matching treated as "just add more clips"

Why it is bad:

- a database is infrastructure, not a complete system
- without a proper chooser graph, the database does nothing
- incomplete pose coverage creates jarring jumps between clips

Do instead:

- design the motion-matching animation graph first
- curate clips to cover the needed pose space (locomotion, transitions, actions)
- build and index the database only when the runtime consumer is real

## 8. Treating Every Character As A Unique Animation Pipeline

Symptoms:

- each character gets its own AnimBP built from scratch
- shared locomotion logic is copy-pasted across characters
- bug fixes require editing multiple AnimBPs

Why it is bad:

- maintenance cost multiplies with character count
- behavior diverges silently between characters
- testing coverage drops as each character is a unique snowflake

Do instead:

- use a shared base AnimBP for common locomotion logic
- specialize only the behavior that truly differs (unique attacks, size-specific transitions)
- use template or child AnimBP patterns to inherit shared structure

## 9. Setting Blend Times By Guessing

Symptoms:

- all transitions use the same default blend time (usually 0.2s)
- fast actions feel sluggish, slow actions feel snappy
- responsiveness does not match the character's weight class

Why it is bad:

- blend time directly affects game feel and responsiveness
- wrong blend times create foot sliding, floating, or pops
- "one size fits all" guarantees some transitions feel wrong

Do instead:

- tune blend times per transition type: fast for combat responsiveness (0.1-0.15s), moderate for locomotion changes (0.2-0.3s), slow for relaxed idles (0.3-0.5s)
- test blend times in gameplay, not just preview
- use transition animations instead of raw blends for important state changes

## 10. Declaring Animation Done Without Playtest

Symptoms:

- all clips authored and imported
- AnimBP compiles and states are connected
- nobody tested the character in an actual gameplay loop

Why it is bad:

- compile success does not mean motion reads well
- state machine logic may produce unexpected transitions during real gameplay
- interaction with other systems (physics, collisions, camera) only shows up at runtime

Do instead:

- use `run_quick_playtest` after every significant animation system change
- observe the character from gameplay camera at real movement speed
- test interruption scenarios: start running then immediately attack, land then immediately dodge

## Key Takeaway

Most bad animation systems are not missing assets. They are using the wrong asset type for the motion role, skipping transition work, ignoring weight and grounding, or skipping runtime proof.
