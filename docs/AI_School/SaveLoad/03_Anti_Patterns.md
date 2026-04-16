# 03 - Anti-Patterns

## 1. Saving Before Defining What Must Restore

If the team cannot say what success looks like after load, the saved artifact is not meaningful.

## 2. Calling A Snapshot A Full Save System

Snapshots are useful, but they do not automatically equal player-facing persistence.

## 3. Mixing Rollback And Shipping Persistence Without Saying So

Editor rollback, test checkpoints, and runtime save slots are related but different jobs.

## 4. Restoring Blind Without Verification

A restore path is not proven just because a file exists.

## 5. Capturing State Too Late

If the checkpoint happens after the risky mutation, rollback value is already reduced.

## 6. Ignoring World-State Evidence

Without a digest or other proof artifact, it becomes harder to know whether restore actually matched the intended state.

## 7. Treating Save-Level Operations As Sufficient Persistence Design

Saving a map file and designing restore semantics are not the same task.

## 8. Forgetting That Destruction And Branching Need Restore Discipline

State-heavy iteration becomes slow fast if restore artifacts are not captured deliberately.

## Key Takeaway

Most bad save/load work is not caused by missing files.

It is caused by unclear restore semantics and weak verification.
