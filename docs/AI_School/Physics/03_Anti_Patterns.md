# 03 - Anti-Patterns

## 1. Setting Collision By Trial And Error Without Defining The Affordance

If the object role is unclear, the collision setup becomes random and hard to maintain.

## 2. Using Overlap, Block, And Query Behavior Interchangeably

Those modes mean different things to gameplay.
Mixing them carelessly produces inconsistent interaction logic.

## 3. Ignoring Reusable Presets

One-off per-actor tweaks multiply quickly.
If a behavior is recurring, it should usually become a deliberate preset or profile.

## 4. Assuming A Destructible Object Is Good Because It Fractures

Fracture is only the first step.
The real question is whether the break pattern, timing, debris, and cleanup support gameplay.

## 5. Creating Too Many Pieces For No Reason

Over-fracturing can make destruction noisy, expensive, and hard to read.

## 6. Iterating Destruction Without Snapshots

If you cannot restore the authored state quickly, destruction tuning becomes slow and error-prone.

## 7. Building Constraint Behavior Without A Mental Model

If a joint is "sort of hinge-like" but the intended motion is not explicit, the result usually feels broken instead of physical.

## 8. Trusting Collision Assumptions Without Trace Proof

Interaction systems often fail because the hit path was never actually tested.

## Key Takeaway

Most bad physics setups are not missing features.

They are missing a clear affordance model, trace proof, or rollback-safe iteration.
