# 04 - Workflow

Networking work should move from authority definition to replication audit to actor inspection to grouping.

## Phase 1: Define The Truth Model

Before touching a network tool, answer:

- who is authoritative
- what state must be shared
- what can remain local
- what latency or update budget the feature can tolerate

## Phase 2: Audit Project Cost

Use:

- `audit_net_replication`

to inspect the current replication cost landscape before making new grouping decisions.

## Phase 3: Inspect Concrete Actors

Use:

- `inspect_actor_replication`

to check the actual state of important actors involved in the feature.

## Phase 4: Shape Relevance With Groups

Use:

- `create_replication_group`
- `add_actor_to_replication_group`
- `set_replication_group_filter_status`

when a valid replication system is active and the grouping logic is understood.

## Phase 5: Plan Runtime Proof

The current tool lane gives strong evidence for planning and inspection.
For gameplay truth, combine it with the domain that actually implements the feature:

- Blueprint
- C++
- GAS
- playtest or network session validation

## Recommended Sequences

### New Multiplayer Feature Review

`define authority` -> `audit_net_replication` -> inspect the key actor classes or instances -> grouping decisions if needed

### Existing Replication Cost Investigation

`audit_net_replication` -> identify heavy classes -> `inspect_actor_replication` on relevant actors -> revise grouping plan

### Iris Group Pass

valid net context confirmed -> `create_replication_group` -> `add_actor_to_replication_group` -> `set_replication_group_filter_status`

## Key Takeaway

The safe order is:

design truth first, inspect cost second, group relevance third.

If you start from groups, the system usually gets optimized before it is understood.
