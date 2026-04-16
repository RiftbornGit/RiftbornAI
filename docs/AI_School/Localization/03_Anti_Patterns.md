# 03 - Anti-Patterns

## 1. Reusing One Key For Multiple Meanings

If two English strings look the same but mean different things, they should not share a key.

## 2. Naming Keys After Temporary Wording

Keys should survive wording changes.
If the key is tied to the exact first sentence draft, maintenance becomes brittle.

## 3. Stripping Context From Short UI Labels

Tiny strings like `Back` or `Use` often need the most context, not the least.

## 4. Treating Source Text As The Only Audience

Localization authoring is not only for the current English reader.
It is also for future translators and future UI integration.

## 5. Assuming A Matching English String Means It Can Be Reused Everywhere

Shared wording is not the same as shared intent.

## 6. Editing String Tables Without Saving Deliberately

If the String Table change is real, it should be persisted cleanly once verified.

## 7. Depending On Optional Culture/Table Helpers Without Checking Availability

Some adjacent inspection helpers may exist in the repo but should still be verified against the live registry before use.

## Key Takeaway

Most localization problems start as source-authoring problems:

- weak keys
- weak context
- weak meaning boundaries
