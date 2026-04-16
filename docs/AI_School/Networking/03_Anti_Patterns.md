# 03 - Anti-Patterns

## 1. Replicating Before Defining Authority

If nobody can say who owns a piece of state, replication decisions become inconsistent immediately.

## 2. Replicating Everything "Just To Be Safe"

That is not safety.
That is unbounded bandwidth and review debt.

## 3. Using Groups As A Substitute For Design

Replication groups can shape relevance, but they cannot rescue a bad authority model.

## 4. Assuming An Actor Is Fine Because It "Probably Replicates"

Use actor-level inspection instead of memory.

## 5. Treating Static Audit As Runtime Proof

`audit_net_replication` is a strong evidence source, but it does not prove gameplay correctness under actual play conditions.

## 6. Ignoring Large Arrays And Heavy Property Sets

Replication cost often hides in bulky property shapes, not only in actor count.

## 7. Creating Groups Without A Real Net Context

Iris grouping only matters when the replication system is actually active for the current world or test context.

## 8. Confusing Local Presentation With Shared Truth

A nice local effect does not always belong on the network.
If that boundary is unclear, the game wastes bandwidth and still confuses players.

## Key Takeaway

Most networking failures start with weak truth ownership, then spread into cost and relevance mistakes.
