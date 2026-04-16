# 01 - Authority Visibility And Network Truth

In a multiplayer game, the hardest question is not "can this value replicate."

It is:

- who owns the truth
- who needs to see it
- how fast it must become visible

If those answers are unclear, networking work becomes expensive and unstable quickly.

## Authority Comes First

Before thinking about replication groups or bandwidth, define:

- which machine is authoritative for the state
- which state is shared
- which state is local presentation only

If authority is fuzzy, replication becomes patchwork.

## Visibility Is Not The Same As Ownership

A client may need to see:

- actor position
- animation state
- damage results
- gameplay tags

without owning the authority that decides them.

Networking quality depends on keeping those ideas separate.

## Relevance Is A Cost Decision

Not every client needs every update.

Ask:

- which actors matter to which players
- which updates are frequent enough to hurt bandwidth
- which data is expensive but low value remotely

This is where grouping and filtering matter.

## Shared Truth Needs Stable Semantics

The remote player should not be forced to guess whether:

- an object is really interactable
- an ability actually resolved
- damage actually landed

That means replication choices should favor clear gameplay truth, not accidental partial visibility.

## Audits Matter Because Memory Lies

Most large projects accumulate replicated properties over time.
People remember the intent vaguely, but not the actual cost.

A replication audit turns "I think this is fine" into inspectable evidence.

## Key Takeaway

Good networking starts from authority and relevance, then uses tooling to measure and shape the cost of that truth.
