# 01 — Interface Hierarchy And Player Focus

UI exists to make decisions easier under time pressure.

A good game interface answers four questions quickly:

- what is happening
- what matters most right now
- what can the player do
- what changed because of the player's action

If the player has to hunt for those answers, the interface is failing.

## Start With The Decision, Not The Decoration

Before building any widget, define:

- the player task (what are they deciding or confirming)
- the urgency level (split-second combat read vs. relaxed inventory browse)
- the input context (mouse/keyboard, gamepad, touch)
- the failure cost (misreading health in combat = death; misreading a flavor text = nothing)

Examples:

- a combat HUD must support split-second threat reading — health, ammo, ability cooldowns
- an inventory screen must support comparison and sorting — slot grids, stat highlights, equip confirmation
- a confirmation prompt must prevent the wrong action without slowing every safe action — clear confirm/cancel, readable consequence text

The UI surface should be shaped by that job. If the job is unclear, the layout will drift toward decoration.

## Hierarchy Is The Core Craft

Players should not read every element with equal effort. Hierarchy tells them where to look first.

Use hierarchy through:

- **position** — top and center = high importance; edges and bottom = peripheral
- **scale** — larger elements register first
- **contrast** — bright against dark, saturated against neutral
- **motion** — animated elements draw attention (use sparingly or everything fights for focus)
- **grouping** — related elements should be visually clustered; unrelated elements should have clear separation
- **spacing** — tight grouping implies relationship; wide spacing implies independence

The highest-priority information should be easiest to find, easiest to decode, and hardest to confuse with decorative context.

If every element is loud, then nothing is loud. If every element is the same size, the player scans linearly instead of jumping to what matters.

## Different UI Surfaces Need Different Logic

### HUD

The HUD exists during gameplay. It must:

- prioritize immediate survival or success state (health, ammo, threat indicators)
- show near-term action availability (ability cooldowns, weapon status)
- provide world-feedback confirmation (damage numbers, interaction prompts, objective markers)

It should NOT:

- expose every game system simultaneously
- use large opaque panels that block gameplay view
- animate in ways that compete with gameplay VFX

Design rule: the HUD should be almost invisible when everything is normal. It should get louder only when the player needs to act.

### Menus

Menus exist in non-combat contexts. They should prioritize:

- scanability — the player should find the option they want quickly
- predictable navigation — up/down/left/right must behave consistently
- clear current selection — the selected element must be visually distinct
- obvious confirm and back behavior — what happens on Accept, what happens on Cancel

Avoid deep nesting. Three levels of menu depth is usually the maximum before players lose orientation.

### Prompts And Overlays

These interrupt the current flow. They should:

- clearly state what is being asked
- clearly show the available responses
- not require reading a paragraph to understand the choice
- respect the urgency of the interruption — a death confirmation can be simple; a permanent deletion should have friction

### Status And Feedback Widgets

Damage numbers, toast notifications, XP gains, achievement popups:

- should communicate state change, not just data
- should have limited screen time — appear, communicate, disappear
- should not stack or overlap into unreadable clutter
- should use consistent visual language (red = negative, green = positive, white = neutral)

## State Communication Is The Real Job

UI is not static. The most important thing it does is communicate state changes:

- health went down — flash, pulse, color shift
- ability is ready — highlight, glow, border change
- item acquired — toast, icon animation, sound cue
- danger approaching — edge vignette, directional indicator, color desaturation

Without state-change feedback, the player has to poll the UI visually — constantly checking numbers instead of feeling changes. Good state communication is felt, not read.

## Resolution And Scale Safety

Game UI ships to many screen sizes. Design considerations:

- anchor elements to screen regions (center, edges, corners) instead of absolute pixel positions
- test at multiple resolutions — at minimum 1080p and 4K
- text must be readable at intended gameplay distance — too small at 4K makes important info invisible
- touch targets (for mobile or console) need minimum sizes for reliable input

## Navigation Is Design, Not Afterthought

For gamepad and keyboard users, navigation IS the interface:

- every interactive screen needs explicit focus order
- starting focus should land on the most common action
- back/cancel must always have a predictable path
- directional navigation should match visual layout (left group → focuses left, right group → focuses right)

If navigation is not authored, the interface only works for mouse users. That is half a UI.

## Key Takeaway

UI quality is not measured by visual polish. It is measured by how fast the player can answer: what is happening, what matters, what can I do, and what changed. Hierarchy, state feedback, and authored navigation are the tools that answer those questions.
- why it matters
- which action confirms or cancels

Do not let prompts become mini menus.

## Navigation Is Part Of The Design

Mouse-driven layouts can hide broken focus order for a long time.

Controller-friendly UI needs:

- an obvious starting focus target
- predictable directional movement
- no dead ends unless intentional
- clear escape and back behavior

Navigation is not cleanup after the visual design. It is part of the interaction model.

## Show Less, But Show It Better

Strong UI usually removes more than it adds.

Ask:

- does this element help a real player decision
- does it need to be persistent
- is it readable at gameplay camera distance
- does it compete with something more important

A quieter interface with stronger priority cues is usually better than a busier interface with more data.

## State Changes Need Clear Feedback

When the player acts, the UI should answer:

- did the action register
- what changed
- what is now unavailable or newly available

This is where timing, visibility, and transition clarity matter more than ornament.

## Key Takeaway

Good UI is an attention-management system.

The goal is not to fill space. The goal is to shorten the path from game state to correct player action.
