# Security Policy

## Reporting Vulnerabilities

If you discover a security vulnerability in RiftbornAI, please report it responsibly:

**Email:** thesmallthingsmusic@gmail.com

Do **not** open a public GitHub issue for security vulnerabilities.

## What to Include

- Description of the vulnerability
- Steps to reproduce
- Potential impact
- Suggested fix (if any)

## Response Timeline

- **Acknowledgment**: Within 48 hours
- **Assessment**: Within 7 days
- **Fix**: Depends on severity, typically within 30 days for critical issues

## Scope

This policy covers:
- The C++ plugin (`Source/`)
- The MCP server (`mcp-server/`)
- The Python bridge (`Bridge/`)
- The governance and proof bundle system

Out of scope:
- Unreal Engine itself (report to Epic Games)
- Third-party dependencies (report to their maintainers)
- AI model behavior (LLM outputs are inherently unpredictable)

## Governance Model

RiftbornAI includes a built-in governance kernel:
- All mutations require signed execution context tokens (HMAC-SHA256)
- Destructive operations are fail-closed and require confirmation
- Tool execution produces auditable proof bundles
- See [docs/GOVERNANCE_AND_SECURITY.md](docs/GOVERNANCE_AND_SECURITY.md) for details
