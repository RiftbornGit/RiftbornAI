# Python SDK Boundary

This document explains what the Python-facing surfaces in this repository are,
what they are for, and what they are not.

RiftbornAI ships an Unreal-integrated editor product plus local bridge and MCP
supporting code. That supporting Python code is useful for local tooling,
tests, and bridge-side workflows, but it is not a separately versioned Python
SDK product with a stable external compatibility promise.

## What Exists Today

The repository contains Python code for:

- local bridge execution and diagnostics
- governance and proof helpers
- CI and test harnesses
- MCP-adjacent workflow support
- repo maintenance utilities

These surfaces are real and used, but they should be treated as repository
implementation and local tooling unless a path explicitly documents a stronger
public contract.

## Product Boundary

The shipped Unreal Editor product boundary is the governed in-editor plugin and
its curated MCP surface.

Python bridge helpers support that product boundary, but they are not the shipped Unreal Editor product boundary themselves.

That matters for documentation and user expectations:

- do not describe every Python helper as a supported end-user SDK feature
- do not imply long-term semver compatibility for internal helper modules
- do not present historical internal research paths as callable customer
  surfaces

## Historical Internal Work

Historical internal self-improvement experiments existed around bridge-side
automation, self-repair, and self-research loops.

Those experiments are not part of the supported customer contract.

Unsupported self-modifying surfaces remain off-contract, and the shipped MCP
surface hard-blocks stale or unsupported names rather than advertising them as
product features.

## Practical Guidance

If you are extending RiftbornAI locally:

- treat Python bridge modules as implementation surfaces first
- prefer documented tool contracts and curated MCP surfaces for durable
  integration points
- expect internal helper modules to change when governance, transport, or test
  architecture changes

If a future standalone Python SDK is introduced, it should be documented as a
separate surface with its own stability and compatibility policy.
