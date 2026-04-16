# 01 — Motion Intent And Readability

Players read animation as prediction.

Before an attack lands, before a dodge completes, before a turn finishes, the motion is already telling the player what is about to happen. That means animation quality is not only about visual polish. It is about gameplay clarity.

## Motion Has Different Jobs

Game animation falls into distinct roles with different authoring requirements:

- **continuous locomotion** — idle, walk, run, sprint, strafe. Parameter-driven, always blending.
- **transitions** — start, stop, turn, pivot, land. These sell weight and responsiveness.
- **one-shot actions** — attacks, reloads, interactions, emotes, hit reactions. Discrete and interruptible.
- **additive layers** — breathing, aim offset, lean, flinch. Layered on top of a base pose.
- **contact moments** — footsteps, weapon impacts, landing. Sell grounding and consequence.

If you mix these roles into one authoring lane, the runtime graph becomes fragile. Locomotion belongs in blend spaces and state machines. One-shots belong in montages. Additives belong in layered blend nodes. Keeping the roles separate keeps the graph maintainable.

## Weight And Grounding

Weight is the single biggest difference between animation that feels good and animation that feels placeholder.

Weight comes from:

- **settle time** — heavier characters decelerate slower, overshoot slightly, compress joints on landing
- **anticipation duration** — a heavy swing needs visible wind-up; a light jab does not
- **center-of-mass shift** — before a step, the hips shift over the planted foot; before a swing, the torso coils
- **ground contact** — feet must not slide, float, or penetrate the ground. IK or root-motion alignment is usually required.

Without weight, characters feel like they are skating on the surface instead of inhabiting it.

## Anticipation And Recovery

Readable motion has three phases:

- **anticipation** — the body prepares. A sword pulls back, knees bend before a jump, weight shifts before a turn.
- **action** — the event happens. The strike, the leap, the dodge.
- **recovery** — the body settles back to neutral. Overshoot, recoil, stumble, or stance reset.

These phases also serve gameplay directly:

- anticipation gives the player time to read enemy intent and react
- action delivers the gameplay event and feedback
- recovery communicates commitment — a heavy attack with long recovery is punishable; a light one is not

Skipping anticipation makes actions feel instant and unreadable. Skipping recovery makes the character feel weightless and uncommitted.

## Root Motion vs In-Place Animation

Two fundamental approaches to locomotion:

- **root motion** — the animation clip drives the character's world position. Movement speed and direction come from the clip, not code. Guarantees foot-ground alignment but requires careful clip authoring and capsule sync.
- **in-place** — the character moves by code; animation plays on top. Easier to control programmatically but requires IK or careful blend to avoid foot sliding.

Root motion is usually better for melee combat, traversal, and characters where ground contact must be precise. In-place is usually better for responsive multiplayer movement and characters that need tight code-driven control.

The choice affects every downstream system: blend spaces, state machines, montages, transitions, and IK. Decide early.

## Locomotion Must Stay Legible In Motion

Locomotion communicates:

- speed — is the character walking, jogging, or sprinting
- direction — forward, strafe, backward
- turn intent — lean, foot plant, hip rotation
- acceleration and deceleration — starts and stops must be visible, not instant
- groundedness — feet contact the surface convincingly

The quality of locomotion lives in the transitions, not in the loops. A perfect walk cycle with instant direction changes feels worse than a simple walk cycle with readable pivots, starts, and stops.

## One-Shot Actions Need Ownership

Attacks, interacts, emotes, and hit reactions should feel discrete. They need:

- clear start conditions (input, trigger, gameplay event)
- clear interruption rules (can this be cancelled? when?)
- readable timing windows (when does the damage frame occur?)
- a return path back into locomotion or idle

One-shots also need **notify events** — animation notifies that fire gameplay logic at specific frames: spawn projectile, play sound, enable damage collision, spawn VFX. The animation is the timing authority for these events, not arbitrary delays in code.

## Additive Layers Separate Concerns

Additive animation layers allow:

- aim offset on top of any locomotion state
- breathing on top of idle
- lean on top of strafe
- flinch on top of any base pose
- weapon sway independent of body movement

The key rule: the additive should modify the base pose without breaking its silhouette or ground contact. If an additive layer causes foot sliding or breaks the root-motion contract, it needs to be constrained.

## Camera Distance Changes The Standard

Animation that looks expressive in asset preview can disappear at gameplay distance.

Test at three distances:

- **close-up** — facial and hand detail readable (cinematic use)
- **gameplay camera** — silhouette, timing, and intent readable (the standard that matters)
- **far distance** — overall body shape and movement direction still communicate

If the gameplay camera cannot distinguish an attack from an idle fidget, the animation is not serving its job regardless of close-up quality.

## Retargeting Is Not Automatic Taste

A technically successful retarget can still produce:

- wrong posture (slouch on a rigid character, rigid pose on a loose character)
- wrong stride length (tall character taking child-sized steps)
- broken hand placement (weapons floating, hands clipping)
- odd timing (weight distribution that does not match the new body proportions)

Retargeting solves transport between rigs. It does not guarantee believable motion. Every retargeted clip needs visual review at gameplay camera distance on the target character.

## Blend Transitions Are Where Quality Lives

The difference between professional and placeholder animation is rarely the individual clips. It is the transitions between them.

Bad transitions produce:

- pops (instant pose changes between states)
- swimming (over-long blend times that lose responsiveness)
- foot sliding (blend interpolating between two different foot contacts)
- weight loss (blend averaging away the anticipation/recovery phases)

Good transitions use:

- appropriate blend times (0.1-0.2s for responsive, 0.3-0.5s for smooth)
- transition animations (dedicated start/stop/pivot clips instead of raw blend)
- sync markers (blend between clips at matching phase points)

## Key Takeaway

Good animation systems are readable motion systems.

The goal is not to accumulate clips. The goal is to make character state and intent legible in play. That requires separating motion roles, committing to weight and grounding, handling transitions deliberately, and judging everything at gameplay camera distance.
