# 01 — Architecture And Communication

Blueprint architecture is mostly about making future changes safe.

A graph that “works today” but cannot be reasoned about tomorrow is technical debt, not progress.

## Start With Responsibility

Every Blueprint should answer a simple question:

- what is this class responsible for?

Examples:

- pickup behavior
- door interaction
- simple hazard zone
- enemy controller logic
- reusable UI widget

If the answer is “a little bit of everything,” the class boundary is already wrong.

## Prefer Composition Over Accumulation

A Blueprint class becomes hard to maintain when more and more unrelated concerns are added to one graph.

Prefer:

- components for reusable capability
- variables for explicit state
- functions for local logic reuse
- child classes only when behavior truly specializes the parent

Do not keep adding branches to a giant Event Graph when the real need is another component or helper function.

## The Event Graph Is For Causality, Not Storage

The Event Graph should answer:

- what happened?
- what should respond?
- what function or state change should occur next?

It should not become the only place where all game logic lives.

Good Event Graphs:

- receive events
- route to clear functions
- keep execution chains short enough to read

Bad Event Graphs:

- contain every piece of class logic inline
- branch deeply across unrelated concerns
- require scrolling through the whole graph to understand one interaction

## Use Functions To Name Intent

Functions turn graph shape into understandable behavior.

A function name like:

- `ApplyDamageToTarget`
- `CanOpenDoor`
- `RefreshHealthBar`
- `AwardPickup`

is more valuable than another 30 loose nodes in the Event Graph.

Pure functions are useful for calculation and decision logic. Impure functions are useful when state changes or side effects should be explicit.

## Communication Patterns

When one Blueprint needs to affect another, choose the lightest coupling that still makes the behavior clear.

### Direct Reference

Use when:

- the relationship is stable
- one object clearly owns or points at the other

Do not use direct references everywhere by habit. They create rigid dependencies quickly.

### Event-Driven Flow

Use when:

- something happened and other logic should respond
- cause and effect are more important than constant polling

This is often the cleanest gameplay scripting model.

### Interfaces Or Loosely Coupled Contracts

Use when:

- multiple classes may respond to the same kind of interaction
- the sender should not care about exact concrete class

The point is not “use interfaces because they are advanced.” The point is to avoid unnecessary hard dependencies.

### Dispatchers Or Broadcast-Style Patterns

Use when:

- one source event needs multiple listeners
- the listeners should not all be hard-wired into one graph

Broadcast patterns are powerful, but they need naming discipline and ownership clarity.

## Tick Is Expensive In More Ways Than Performance

Tick is not just a frame-cost problem. It is a reasoning-cost problem.

Tick-based logic often hides:

- state checks that should be event-driven
- transitions that should happen once
- conditions that should be expressed as gameplay events

Use Tick when continuous updates are genuinely required, not because it is convenient.

## Variables Should Represent Real State

Every variable should answer:

- what real state does this hold?
- who is allowed to change it?
- when should it change?

If a variable exists only because the graph became hard to wire cleanly, that is a smell.

## Readability Beats Cleverness

Blueprints are maintained visually. Optimize for:

- obvious execution flow
- consistent node spacing and grouping
- named functions over repeated patterns
- small focused event chains

A clever graph is often just an unreadable graph with confidence.

## Key Takeaway

At every Blueprint edit ask:

- does this class still have one clear job?
- is this logic in the right graph?
- should this become a function?
- should this be event-driven instead of polled?
- am I increasing or reducing graph debt?

If you do not ask those questions, you will build spaghetti one “small” node change at a time.
