# 01 — Gameplay Feedback And Readability

You are not making a tech demo. You are helping a player understand game state in real time.

Good VFX answers gameplay questions immediately:

- What just happened?
- Where did it happen?
- How dangerous was it?
- Is it still active?
- What should the player do next?

If an effect cannot answer those questions from normal gameplay distance, it is not finished.

## The Jobs Of VFX

### 1. Telegraph

Warn the player before an important event.

- Charge-up glow before a heavy attack
- Ground decal before an area blast
- Wind-up sparks before a trap fires

Telegraphs must be early, readable, and spatially honest. The player should know where the danger zone begins and ends.

### 2. Impact Confirmation

Show that an action connected.

- Hit spark when a sword lands
- Flash and debris when a projectile impacts
- Healing burst when a support ability resolves

Impact VFX should be short, decisive, and tied to the exact contact point.

### 3. Persistent State

Indicate something that remains true after the initial action.

- Burning aura on a buffed enemy
- Poison cloud in an area
- Shield shimmer while a defense window is active

Persistent effects should be lower intensity than the initial trigger. The opening moment gets attention; the sustain phase preserves clarity.

### 4. Ambient Mood

Support place, tone, and atmosphere.

- Dust motes in abandoned halls
- Embers near braziers
- Mist over wetlands

Ambient effects must never compete with gameplay-critical cues.

## Visual Hierarchy

All VFX is not equally important.

### Highest Priority: Gameplay-Critical

These effects can change player decisions in under one second.

- Enemy telegraphs
- Damage zones
- Projectile paths
- Parry windows
- Pickup confirmation

They need strong silhouette, strong contrast, and clean timing.

### Middle Priority: Interactable Or State-Based

- Buff and debuff loops
- Objective markers
- Traversal helpers
- Crafting or harvest feedback

These should be readable without dominating the screen.

### Lowest Priority: Ambient

- Fog wisps
- Pollen
- Sparkles
- Decorative magical drift

These should fill space, not steal focus.

## Shape Language Matters

Players read motion and shape before they read detail.

- Fire rises, flickers, and tapers upward.
- Smoke expands, softens, and loses edge definition over time.
- Sparks travel fast, thin, and directional.
- Magic often needs a core layer plus orbiting or trailing detail.
- Poison usually reads better with low-to-ground spread than vertical fireworks.
- Explosions are not just "big fire." They are flash, pressure, debris, and then residual smoke.

If the motion language is wrong, better materials and more particles will not save the effect.

## Timing Matters More Than Count

Most bad VFX problems are timing problems disguised as content problems.

### Anticipation

The player sees buildup before payoff.

- Charge glow
- Expanding ring
- Tightening particles toward a focal point

### Contact

The peak moment. Usually the brightest and shortest phase.

- Flash
- Spark burst
- Shock ring

### Decay

The effect resolves and leaves the screen clean enough for the next decision.

- Smoke drift
- Ember fallout
- Dissolve

Without clear phase separation, effects feel mushy and noisy.

## Camera Distance Changes Everything

An effect that looks great from one meter away may disappear at gameplay distance.

Always judge:

- Is the silhouette still visible at normal camera height and FOV?
- Does the effect still read against bright and dark backgrounds?
- Can the player distinguish telegraph from impact from residual loop?
- Does the effect stay readable when several enemies or abilities overlap?

Tiny details should support the main read, not become the main read.

## Color And Contrast

Color is a gameplay tool, not just an art direction choice.

- Warm, bright, high-contrast colors pull attention fast.
- Cool or desaturated loops recede and are easier to live with over time.
- Friendly and hostile effects should be separable at a glance.
- A glowing effect needs a darker or simpler surrounding area to read cleanly.

Do not use the same brightness and saturation for everything. If everything shouts, nothing communicates.

## Layering Strategy

Most strong effects are 2-4 layers with different jobs:

1. Core read
   The main shape or energy source.
2. Motion accent
   Sparks, streaks, trails, or arcs that show force and direction.
3. Residual volume
   Smoke, haze, dust, mist, or glow that gives aftermath.
4. Environmental tie-in
   Ground splash, decal, light pulse, or local disturbance.

If one emitter tries to do all four jobs, it usually does none of them well.

## Performance Is Part Of Readability

An unreadable effect is bad. A readable effect that tanks the frame rate is also bad.

- More particles do not guarantee stronger reads.
- Bright full-screen translucency creates noise and overdraw.
- Too many lights make scenes look muddy and expensive.
- GPU-heavy effects are easy to overbuild because they look cheap in isolation.

Your goal is not "maximum spectacle." Your goal is "best communication per cost."

## Key Takeaway

At every step ask:

- What gameplay job is this effect doing?
- What is the single strongest read from gameplay camera distance?
- Which layer is carrying that read?
- Which layers are supporting it?
- What can be removed without harming clarity?

If you cannot answer those questions, you are decorating blindly.
