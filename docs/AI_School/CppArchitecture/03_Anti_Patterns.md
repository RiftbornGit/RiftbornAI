# 03 — Anti-Patterns

Most bad Unreal C++ architecture starts with the wrong class type, then gets entrenched by momentum.

## Anti-Pattern: Giant Actor Syndrome

Symptoms:

- one actor owns many unrelated systems
- reusable logic is embedded directly in actor code
- every new feature gets added to the same class

Why it is bad:

- reuse drops
- testability drops
- future refactors become class surgery

Do instead:

- keep world identity in the actor
- move reusable behavior into components or scoped services

## Anti-Pattern: GameMode As The Main Dumping Ground

Symptoms:

- systems unrelated to match rules end up in GameMode
- game-wide services depend on the current mode class

Why it is bad:

- wrong lifetime
- wrong ownership
- harder reuse across modes

Do instead:

- keep GameMode focused on match flow and rules
- move durable services to the appropriate subsystem type

## Anti-Pattern: Subsystem For Everything

Symptoms:

- every new manager becomes a subsystem
- actor-owned or player-owned behaviors are centralized without reason

Why it is bad:

- lifetime boundaries blur
- ownership becomes abstract instead of explicit

Do instead:

- use subsystems only when the engine lifetime actually matches the service

## Anti-Pattern: Copying Behavior Across Actors Instead Of Using Components

Symptoms:

- multiple actors reimplement similar health, inventory, or interaction logic
- “we’ll abstract it later” never happens

Why it is bad:

- bugs and feature drift multiply

Do instead:

- use `generate_component_class_from_description` when the behavior is reusable

## Anti-Pattern: Choosing Character When You Mean Actor

Symptoms:

- classes inherit Character because movement might be useful
- non-character world objects inherit locomotion-heavy classes

Why it is bad:

- extra complexity
- misleading class semantics

Do instead:

- use Character only for real pawn locomotion cases

## Anti-Pattern: Blind Live Coding Retries

Symptoms:

- every compile failure gets another Live Coding attempt
- header/new-class/UHT changes are treated like safe hot reloads

Why it is bad:

- wastes time
- obscures the real failure mode

Do instead:

- use `run_ubt` for new classes and UHT-sensitive changes
- use `get_build_errors` and `get_build_status` before choosing the next step

## Anti-Pattern: Treating Blueprint Conversion As Automatic Parity

Symptoms:

- `convert_blueprint_to_cpp` is run without checking converter coverage
- unsupported nodes are ignored

Why it is bad:

- migrated code appears complete while missing important behavior

Do instead:

- use `check_blueprint_convertibility` first
- treat unsupported nodes as real follow-up work

## Anti-Pattern: Emitting Code Before The Class Boundary Is Decided

Symptoms:

- generator choice is made because it is convenient
- responsibility questions are asked after files already exist

Why it is bad:

- the wrong architecture gets frozen into source quickly

Do instead:

- choose Actor, Character, Component, GameMode, or Subsystem first
- then choose the generator

## Anti-Pattern: Declaring Success At Compile Green

Symptoms:

- no runtime test after gameplay-facing code changes
- build success is treated as behavior success

Why it is bad:

- many gameplay bugs are not compile bugs

Do instead:

- run `run_quick_playtest`
- verify the real runtime consequence of the new code

## Key Takeaway

The common C++ failure is not missing syntax knowledge. It is putting behavior in the wrong place and then choosing the wrong build loop around it.
