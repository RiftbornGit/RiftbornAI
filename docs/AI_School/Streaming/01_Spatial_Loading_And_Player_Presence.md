# 01 - Spatial Loading And Player Presence

Streaming decides when the world becomes real for the player.

That means streaming quality is not only about frame time. It is also about continuity, anticipation, and trust.

## The Player Should Not Feel The Streaming Model

The player should feel:

- place
- distance
- transition
- consequence

They should not feel:

- arbitrary appearance
- contradictory world states
- important content arriving too late

If streaming reveals itself as a system, immersion drops fast.

## Separate Spatial And State Logic

Two different questions often get mixed together:

- what loads because of where the player is
- what changes because of world state

These are not always the same.

Examples:

- a distant district loading because the player approaches is spatial
- a city switching from festival to siege is state-based

Good streaming design keeps those ideas explicit.

## Data Layers Should Mean Something

Data layers are strongest when they encode real world distinctions:

- day versus night state
- intact versus destroyed version
- editor-only dressing versus runtime content
- phase-based city or mission state

If layers are just random buckets, the world becomes hard to reason about.

## Streaming Sources Should Match Player-Relevant Presence

What matters is not only where assets exist.
What matters is where the player can perceive or reach next.

Streaming sources should support:

- approach paths
- expected camera travel
- likely arrival points

If important content loads only after the player already needs it, the system is late.

## Texture Residency Is Part Of The Read

Streaming is not only actor presence.
Texture residency also affects whether the world feels finished.

If landmarks and hero surfaces are present but still blur or pop at the wrong moment, presence breaks anyway.

## Key Takeaway

Good streaming preserves the illusion of a continuous world.

The loading model should support the player's movement and the world's state changes without drawing attention to itself.
