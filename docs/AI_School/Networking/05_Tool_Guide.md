# 05 - Tool Guide

Use exact registered networking tool names. If a name is uncertain, verify with `describe_tool`.

## Networking Tool Map

| Goal | Primary Tools | Notes |
|------|---------------|-------|
| Audit replicated actor classes project-wide | `audit_net_replication` | Static, safe inspection lane for replication cost and review points |
| Inspect a concrete actor's replication state | `inspect_actor_replication` | Useful for actor-level truth and Iris handle inspection |
| Create a named Iris group | `create_replication_group` | Only useful when a valid replication system is active |
| Assign an actor to a group | `add_actor_to_replication_group` | Explicit relevance grouping step |
| Allow or disallow a group | `set_replication_group_filter_status` | Shapes group visibility through filter status |

## Default Lanes

### Audit Lane

Use for:

- project-wide replication review
- identifying heavy or risky classes

Main tools:

- `audit_net_replication`

### Actor Inspection Lane

Use for:

- verifying a concrete actor's replication state

Main tools:

- `inspect_actor_replication`

### Iris Group Lane

Use for:

- explicit relevance-group control

Main tools:

- `create_replication_group`
- `add_actor_to_replication_group`
- `set_replication_group_filter_status`

## Proven Sequences

### Replication Review Pass

`audit_net_replication` -> inspect key actors -> decide whether grouping is needed

### Grouping Pass

valid net context -> `create_replication_group` -> `add_actor_to_replication_group` -> `set_replication_group_filter_status`

## Rules

- Do not replicate blind before defining authority.
- Do not treat audit output as full runtime proof.
- Do not use group tools without a valid replication system context.
- Do not assume actor-level truth from memory when it can be inspected.
- Do not optimize network relevance before deciding who actually needs the data.

## Key Takeaway

The networking surface is strongest when you separate:

- authority design
- replication audit
- actor inspection
- relevance grouping

If those are collapsed together, cost and truth usually get tangled.
