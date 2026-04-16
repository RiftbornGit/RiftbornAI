# Commercial Licensing & Founding Supporter Program

RiftbornAI is distributed under the [Business Source License 1.1](LICENSE).
The license text is authoritative; this document is a practical summary
plus a description of the paid Founding Supporter tiers.

---

## Pricing model

**During the beta (now — v0.x):** the plugin is **free**. Download it,
install it, use it in your games. No DRM, no expiry, no registration
required. A signed license just lights up a "Licensed to …" credit in
the copilot panel.

**At v1.0 stable:** the plugin becomes paid. Pricing:

- **$49 — Personal, lifetime.** One developer.
- **$99 — Studio, lifetime.** Multi-seat within one legal entity.

Both tiers are one-time purchases and cover all updates across v0.x and v1.x.

## Buy now = lock in before v1.0

Founding Supporters buy today at the v1.0 price, keep it forever, and
support continued development during the beta. Checkout is on
[riftbornai.com](https://riftbornai.com) or via direct Stripe links below.

### Founding Supporter — $49 (Personal)

- Perpetual personal commercial-use license (signed Ed25519 + JSON + PDF)
- Lifetime updates across v0.x and v1.x
- Early access to new platform builds (macOS / Linux) and new UE version ports
- Priority support via `founders@riftbornai.com` — 48-hour response target
- Founders Discord — direct line, monthly office hours
- Optional "Licensed to …" credit in the copilot panel
- 30-day no-questions refund

Buy: [buy.stripe.com/6oU8wPcBkdpPbNO7Al14400](https://buy.stripe.com/6oU8wPcBkdpPbNO7Al14400)

### Founding Studio — $99 (Studio)

Everything in Founding Supporter, plus:

- Multi-seat use across your studio (one legal entity, unlimited seats)
- Priority studio support channel
- Named studio credit on request
- Early architecture input on v1.x milestones
- 30-day no-questions refund

Buy: [buy.stripe.com/7sY00j9p899z8BCcUF14401](https://buy.stripe.com/7sY00j9p899z8BCcUF14401)

---

## What the license *is*, mechanically

A small signed JSON (and a human-readable PDF receipt) emailed within 24
hours of purchase. The JSON drops into `<YourProject>/Config/RiftbornLicense.json`
and the plugin verifies it with an embedded Ed25519 public key at editor
startup.

The signature is cryptographically tamper-proof — anyone can see what's
in the payload, but nobody can forge a valid one without the private
signing key that stays with Riftborn Studios.

## What the license is *not*

- **Not DRM during beta.** Through v0.x, the plugin is fully functional
  whether a license is installed or not.
- **Not a rental.** Both tiers are perpetual for the major-version line
  you bought into. v0.x and v1.x are covered. Future v2.0+ may be a
  separate purchase (with Founding Supporter discounts).
- **Not a resale grant.** See below.

## Refund policy

Email `founders@riftbornai.com` within 30 days of purchase for a full
refund. No questions asked. The signed license is retained as a
supporter badge regardless of refund — it stops being *your* license in
the v1.0+ enforcement sense, but the Discord and credit remain
historically.

---

## When you likely do *not* need a separate commercial license

Under the BSL 1.1 Additional Use Grant you may already:

- build, test, and ship your own games or interactive experiences
- use the software internally within your studio or company
- use the software to deliver work for clients
- modify the software for internal or project-specific use

During the beta this is truly free. At v1.0 the plugin becomes paid,
and a Founding Supporter or Studio license is how you unlock
continued use. Both tiers are honored against the BSL 1.1 Additional
Use Grant.

## When you likely *do* need a separate custom license

Contact Riftborn Studios before proceeding if you want to:

- sell RiftbornAI itself, or a renamed or repackaged fork of it
- offer RiftbornAI as a hosted, managed, or white-label service
- embed RiftbornAI into a commercial Unreal AI tooling product sold to
  third parties
- provide a competing plugin, SaaS, OEM, or enterprise offering that
  substantially overlaps with RiftbornAI's commercial surface
- obtain alternative commercial terms such as warranties, indemnities,
  SLAs, or private licensing arrangements

These uses fall outside both the BSL 1.1 Additional Use Grant and the
Founding Supporter / Studio tiers — we need a custom agreement.

## Contact

- All licensing, enterprise, and support: `founders@riftbornai.com`

Include when emailing about a custom license:

- your company name
- the product or service you want to build
- whether the use is internal, client-facing, hosted, embedded, OEM, or
  resale
- your target timeline
