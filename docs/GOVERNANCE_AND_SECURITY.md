# RiftbornAI — Governance & Security Model

**Last updated**: 2026-02-21

This document describes how RiftbornAI protects users from AI-initiated damage. User-facing mutating operations are expected to go through a multi-layer governance pipeline that enforces risk classification, confirmation requirements, proof generation, and rollback capability.

---

## 1. Design Philosophy

RiftbornAI follows a **fail-closed** security model:

- **Default deny**: Unknown tools are blocked until classified
- **Explicit confirmation**: Destructive operations require user acknowledgment
- **Signed proofs**: Every mutation produces an HMAC-SHA256 audit trail
- **Taint tracking**: Sessions that bypass governance are permanently marked
- **Constitutional hierarchy**: Core governance files cannot be self-modified

---

## 2. Risk Tier Classification

Every tool is assigned a risk tier that determines its governance requirements:

| Tier | Examples | Confirmation? | Proof? | Undo? |
|------|----------|--------------|--------|-------|
| `READ_ONLY` | `get_project_info`, `list_assets`, `get_output_log` | No | Optional | N/A |
| `MUTATING_REVERSIBLE` | `spawn_actor`, `set_actor_transform`, `set_object_property_typed` | No (auto) | Yes | Tokenized undo |
| `MUTATING_PROJECT` | `create_blueprint`, `create_material`, `compile_blueprint` | Context-dependent | Yes | Git revert / backup |
| `DESTRUCTIVE` | `delete_asset`, `delete_level`, `purge_blueprints` | **Always required** | Yes | None — fail-closed |

Risk tiers are defined in two authoritative sources:
- **C++ side**: `ToolGovernance.cpp` and `BuiltinToolRegistry.cpp`
- **Python side**: `Bridge/core/tool_contract.py` (contracts.json)

The CI gate `ci/test_risk_tier_consistency.py` verifies both sides agree.

---

## 3. Tool Contracts

Every mutating tool must have a `ToolContract` declaration:

```python
from tool_contract import ToolContract, RiskLevel, ToolEffects, UndoStrategy

CONTRACT = ToolContract(
    name="spawn_actor_pie",
    risk=RiskLevel.MUTATION,
    effects=ToolEffects(world_mutation=True, runtime_only=True),
    requires_exec_ctx=ExecCtxRequirement(required=True, min_tier="mutating_reversible"),
    undo=UndoStrategy.TOKENIZED,
    determinism=Determinism.EXPECTED,
    scope=ToolScope.PIE_ONLY,
)
```

**Contract enforcement**:
- `Bridge/core/tool_contract.py` — Contract schema and registry
- `riftborn/kernel/contract_enforcer.py` — Runtime enforcement
- `ci/tests/test_contract_enforcement.py` — CI ratchet (floor only goes UP)

Contract coverage is enforced by CI and should remain complete for the governed mutating surface. Check the current CI state rather than relying on a hardcoded percentage in this document.

---

## 4. Execution Context (ExecCtx)

Every tool invocation mints an `ExecCtx` — a unique execution context that tracks:

- **Token ID**: Unique identifier for this execution
- **Session ID**: Parent session
- **Risk tier**: Governs what's allowed
- **Timestamp**: When execution started
- **Caller**: Who/what initiated (MCP, agent, manual)
- **Rollback state**: Snapshot for undo capability

**Flow**:
```
Request → ExecCtxValidator → Mint Token → Execute Tool → Record Proof → Return
```

**Key files**:
- `Bridge/core/exec_ctx/__init__.py` — Token minting and validation
- `Source/.../ExecCtxValidator.cpp` — C++ side validation
- `Bridge/core/execution_gateway.py` — Gateway that orchestrates the flow

---

## 5. Confirmation Tokens

Destructive operations require explicit user confirmation:

```
1. Tool classified as DESTRUCTIVE
2. System generates ConfirmationToken (single-use, time-limited)
3. Token presented to user with operation description
4. User acknowledges → token consumed
5. Operation executes
6. Token CANNOT be reused (single-use enforced)
```

**Key files**:
- `riftborn/kernel/contract_enforcer.py` — `ConfirmationToken`, `validate_confirmation()`
- `Source/.../ToolConfirmation.cpp` — C++ confirmation UI
- `Tests/governance/test_confirmation_token_single_use.py` — Proves tokens are single-use
- `Tests/governance/test_destructive_requires_confirmation.py` — Proves destructive ops need confirmation

---

## 6. Proof Bundles

Every mutation generates a cryptographically signed proof bundle:

```json
{
  "schema_version": "2.0.0",
  "session_id": "sess-abc123",
  "timestamp": "2026-02-21T10:30:00Z",
  "tool_name": "spawn_actor",
  "tool_args": {"class_name": "Cube", "location": [0, 0, 0]},
  "risk_tier": "mutating_reversible",
  "exec_ctx_id": "ctx-def456",
  "chain_hash": "prev_hash_sha256...",
  "result": {"ok": true, "actor_label": "Cube_1"},
  "digest_before": "sha256_of_world_state_before",
  "digest_after": "sha256_of_world_state_after",
  "gate_results": [...],
  "hmac_signature": "sha256_hmac_of_entire_payload"
}
```

**Binding requirements** — ALL must be present for a valid proof:
1. `chain_hash` — Links to previous proof (ledger chain)
2. `allowlist_hash` — What commands were permitted at execution time
3. `digest_before/after` — File/world state deltas
4. `gate_results` — Which predicates verified what
5. `tool_ids` — Which tools were invoked
6. `session_id` — Run identity
7. Repository commit hash — Code version
8. `timestamp` — Wall clock + monotonic

**Key files**:
- `riftborn/proof/bundle.py` — Proof bundle creation and binding
- `riftborn/proof/canonical_serialization.py` — Deterministic JSON hashing
- `Source/.../ProofWriter.cpp` — C++ proof generation
- `Source/.../ToolExecutionProof.cpp` — Per-tool proof recording
- `tools/verify_proof.py` — CLI proof verification

---

## 7. PROOF Mode vs DEV Mode

| Aspect | DEV Mode (default) | PROOF Mode |
|--------|-------------------|------------|
| `execute_python()` direct calls | ⚠️ Allowed, logged | ❌ Blocked |
| Governance bypass | Flagged, session tainted | Hard blocked |
| Proof bundles | Generated but tainted | Full integrity |
| Research module imports | Allowed | Must use `governed_executor` |
| Human approval | Not required | Required for CONSTITUTION files |

Set via environment:
```bash
export RIFTBORN_PROOF_MODE=1  # Strict
export RIFTBORN_PRODUCT_MODE=1  # Production (implies PROOF)
```

---

## 8. The Constitution

The governance system has a constitutional hierarchy:

### CONSTITUTION (immutable without human approval)
- `governance/constitution.py` — The constitution definition itself
- `governance/self_update_protocol.py` — How self-updates are gated
- `governance/kernel_protection.py` — Kernel file protection
- `core/tcp_client.py` — Contains execute_python guard
- `PredicateEvaluator.cpp/.h` — What counts as "passing"
- `PredicateMutationGuard.cpp/.h` — Predicate enforcement

### PROTECTED (auto-approvable if gates pass)
- Sensor/actuator boundary files
- Risk tier definitions
- Core contracts

### POLICY (normal code, standard gates)
- Tool implementations
- Test files
- Documentation

### GENERATED (freely regenerable)
- `generated-tools.ts`
- Build artifacts

**Key files**:
- `Bridge/governance/constitution.py` — Classification definitions
- `Bridge/governance/self_update_protocol.py` — Self-modification gates
- `Bridge/governance/trust_foundation.py` — Trust scoring

---

## 9. Session Taint Tracking

A session becomes **tainted** when any governance bypass occurs:

- Direct `execute_python()` call from ungoverned code path
- Confirmation token reuse attempt
- Risk tier mismatch
- Gateway bypass detection

Tainted sessions:
- Continue to function (fail-open for development)
- All proof bundles marked as tainted
- Taint state is irreversible within a session
- Logged with full context for audit

**Key files**:
- `Source/.../SessionTaint.h/.cpp` — C++ taint tracking
- `ci/taint_gate.py` — CI taint detection

---

## 10. Autonomy Levels (L0-L5)

The system supports graduated autonomy:

| Level | Name | Permissions |
|-------|------|-------------|
| L0 | **Manual** | All operations require human approval |
| L1 | **Assisted** | Read-only tools auto-approved |
| L2 | **Supervised** | Reversible mutations auto-approved |
| L3 | **Autonomous** | Project mutations auto-approved with gates |
| L4 | **Self-improving** | Can apply code fixes with MergeGuardian gates |
| L5 | **Fully autonomous** | Can modify governance (constitution still protected) |

**Key files**:
- `Bridge/governance/autonomy_policy.py` — Policy definitions
- `riftborn/kernel/gate.py` — Level enforcement

---

## 11. CI Governance Gates

The CI pipeline enforces a set of governance gates, including:

| Gate | File | What It Checks |
|------|------|----------------|
| Contract coverage | `ci/tests/test_contract_enforcement.py` | All mutating tools have contracts |
| Risk tier consistency | `ci/test_risk_tier_consistency.py` | C++ and Python risk tiers agree |
| Mutation choke point | `ci/tests/test_mutation_choke.py` | No `execute_python()` bypass |
| Registry drift | `ci/test_registry_drift.py` | Tool registry in sync across layers |
| Policy drift | `ci/gates/gate_policy_drift.py` | Governance policies unchanged |
| Secret scanning | `ci/hygiene_gate.py` | No committed API keys |
| Proof integrity | `ci/proof_mode_gate.py` | Proof bundles valid |
| Trust invariants | `ci/test_adversarial_trust_suite.py` | Trust system not bypassable |
| Bypass hunting | `ci/test_bypass_hunt.py` | No governance bypasses |
| Shipped surface | `ci/gates/alive_readiness_gate.py` | Production and beta-release tool surfaces stay aligned with contracts, route proofs, governance routes, and readiness tiers |
| Governance bars | `ci/test_governance_bars.py` | Governance thresholds met |
| Naming bypass | `ci/test_naming_bypass.py` | No tool name collision attacks |
| Boundary enforcement | `ci/test_boundary_enforcement.py` | Module boundaries respected |

---

## 12. Security Features

### HMAC Message Integrity
TCP bridge messages can be HMAC-SHA256 signed when `RIFTBORN_HMAC_KEY` is set:
- `Bridge/core/tcp_client.py` — `compute_message_hmac()`, `verify_message_hmac()`

### Input Sanitization
- `Source/.../InputSanitization.h` — All user inputs validated
- Path traversal protection
- SSRF guards on network calls

### Rate Limiting
- `Bridge/core/rate_limiter.py` — Per-tool rate limits
- `Bridge/core/http_rate_limit.py` — HTTP endpoint rate limiting
- `Source/.../BridgeRateLimiter.cpp` — C++ rate limiting

### Circuit Breakers
- `Bridge/core/circuit_breaker.py` — Per-tool circuit breakers
- 5 failures in 30s → 60s cooldown
- Metrics exported for monitoring

### Restricted Pickle
- `Bridge/core/restricted_pickle.py` — Safe unpickler for brain data
- Whitelist of allowed modules/classes
- No arbitrary code execution via deserialization
