# 01 - Force Collision And Physical Readability

Players read physics through contact.

They learn:

- what stops them
- what can be pushed
- what can be broken
- what is dangerous to touch

If those rules are inconsistent, the world feels unfair even when the simulation is technically working.

## Physics Is An Affordance Language

Before setting any collision or constraint, define the object's job:

- block movement
- allow overlap
- support interaction traces
- hinge, swing, or slide
- hold until enough force breaks it
- fracture and become debris

That job should drive the setup.

## Collision Communicates Trust

Good collision tells the player the truth.

Examples:

- a solid wall should always feel solid
- a trigger volume should not behave like a wall
- a destructible crate should feel like an obstacle until the break condition is met

When collision lies, the player stops trusting the space.

## Constraints Need Readable Behavior

A hinge, spring, or fixed joint should support a clear mental model.

Ask:

- what movement is allowed
- what movement is resisted
- when should the joint fail

If the answer is fuzzy, the constraint will feel chaotic rather than expressive.

## Destruction Needs Intent

Not every broken object should:

- explode into too many pieces
- simulate forever
- create unreadable debris

Destruction should support:

- impact
- feedback
- state change
- cleanup

The player should understand what changed and why.

## Traces Are Proof Tools

Line and sphere traces are not only gameplay mechanics.
They are also verification tools for checking whether the collision world matches the intended design.

If interaction logic depends on a hit, prove that hit path.

## Key Takeaway

Good physics work starts from readable world rules.

Simulation is the implementation layer, not the design goal.
