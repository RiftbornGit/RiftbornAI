# RiftbornAI Readiness Truth

**Last updated**: 2026-04-16

This document defines the current readiness-truth contract for the shipped
product.

The roadmap goal is a single generation path for readiness. The repo is not
fully there yet, so this document describes the phase-one contract that CI now
enforces.

## Canonical Generator Root

`Bridge/toolbook/public_surface.json` is the canonical shipped-surface
membership root.

If a tool is not declared there, it is not part of the shipped readiness story
regardless of what other generated files or hand-maintained maps say.

## Current Downstream Chain

| File | Current role |
| ---- | ------------ |
| `Bridge/toolbook/public_surface.json` | Canonical shipped-surface membership root |
| `Bridge/toolbook/contracts.json` | Runtime contract bundle consumed by governance and execution paths |
| `mcp-server/src/tool-readiness.ts` | MCP visibility projection and tier filter |
| `TOOL_REGISTRY.md` | Generated route catalog in the private source repo (not shipped in the public beta package) |
| `ci/gates/alive_readiness_gate.py` | CI enforcement for the readiness truth contract |

## Rules

- Membership starts in `Bridge/toolbook/public_surface.json`.
- `production_tools` in `Bridge/toolbook/public_surface.json` project to the MCP `PRODUCTION` tier by default.
- `mcp-server/src/tool-readiness.ts` may filter or tier tools for MCP exposure,
  but it must not widen manifest membership. Its explicit override map should
  stay focused on non-default exceptions rather than duplicating the production
  manifest.
- `Bridge/toolbook/contracts.json` and `TOOL_REGISTRY.md` are downstream
  artifacts, not independent truth sources for shipped readiness. The public
  beta package ships `contracts.json`; `TOOL_REGISTRY.md` remains a source-repo
  generated artifact.
- `ci/gates/alive_readiness_gate.py` is the enforcement lane for drift between
  shipped membership, contracts, route proof, governance routes, tiers, and the
  readiness-truth docs.
- Public docs should explain the chain above and should not publish hardcoded
  tool counts as if they were durable truth.

## Transition State

This is a deliberate phase-one step from the 90-day execution board.

It does not claim that every readiness field already lives in one generated
schema. It does make the current split explicit, bounded, and machine-checked
so later migration work has one contract to attach to instead of multiple
competing stories.

The next migration step is to move more readiness metadata toward the canonical
generator root so `mcp-server/src/tool-readiness.ts` becomes a thinner
projection layer rather than a parallel hand-maintained truth source.
