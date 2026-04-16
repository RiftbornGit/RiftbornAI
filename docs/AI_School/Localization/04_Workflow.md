# 04 - Workflow

Localization work should move from meaning definition to key design to String Table authoring to verification.

## Phase 1: Define The Message Intent

Before touching a localization tool, answer:

- what the text means
- where the player sees it
- what decision or understanding it should support

## Phase 2: Design Stable Keys

Create keys that:

- map to one meaning
- survive wording changes
- preserve context boundaries

## Phase 3: Author The Table

Use:

- `create_string_table`

to create the String Table asset when needed.

Use:

- `add_string_table_entry`

to add or update source entries.

## Phase 4: Inspect Optional Helpers Only If Verified

If the task needs deeper inspection, first confirm availability of helpers like table-entry or culture inspection using:

- `list_all_tools`
- `describe_tool`

## Phase 5: Save Confirmed Changes

Use:

- `save_asset`
- `save_dirty_assets`

when the String Table changes are verified and should persist.

## Recommended Sequences

### New Localization Table

`define key structure` -> `create_string_table` -> `add_string_table_entry` -> `save_asset`

### Existing Table Extension

`review meaning and key choice` -> `add_string_table_entry` -> `save_asset` or `save_dirty_assets`

## Key Takeaway

The safe order is:

define the meaning first, choose the key second, write the table third.

If you reverse that order, localization starts from text fragments instead of intent.
