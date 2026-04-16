# 01 - Text Intent Keys And Cultural Clarity

Localized text succeeds when the player understands the same gameplay meaning, even if the words change.

That means localization work starts with intent, not with string count.

## Meaning Comes Before Wording

Before creating a localization key, define:

- what the line means
- where the player sees it
- what action or understanding it should support
- whether tone matters as much as literal content

If this is unclear, translators or downstream systems are forced to guess.

## Keys Should Represent Stable Meaning

A good localization key is:

- stable over time
- specific enough to one meaning
- reusable only when the exact meaning is truly the same

A bad key is:

- tied to transient wording
- vague about context
- reused because two strings happen to match in English

## UI Text Needs Context Too

Short UI labels are often the most dangerous strings because they look simple.

A word like:

- `Back`
- `Close`
- `Use`
- `Ready`

can mean different things in different screens or flows.

The localization system should preserve those distinctions.

## Gameplay Clarity Must Survive Translation

The translated line still needs to:

- fit the UI
- preserve urgency
- preserve contrast between similar actions
- avoid misleading the player

Localization quality is therefore tied to design clarity, not only language quality.

## Key Takeaway

Good localization starts with stable meaning, stable keys, and clear usage context.

If the source system is ambiguous, translation only spreads that ambiguity.
