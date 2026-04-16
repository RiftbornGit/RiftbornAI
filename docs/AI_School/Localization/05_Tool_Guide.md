# 05 - Tool Guide

Use exact registered localization tool names. If a name is uncertain, verify with `describe_tool`.

## Localization Tool Map

| Goal | Primary Tools | Notes |
|------|---------------|-------|
| Create a String Table asset | `create_string_table` | Main localization container on the current supported lane |
| Add or update a localized source entry | `add_string_table_entry` | Core authoring tool for stable key/value pairs |
| Save a confirmed String Table change | `save_asset`, `save_dirty_assets` | Persist the authored table state cleanly |
| Check optional localization helpers | `list_all_tools`, `describe_tool` | Use before relying on helpers like entry or culture inspection |

## Default Lanes

### Table Authoring Lane

Use for:

- creating a new table
- adding or updating entries

Main tools:

- `create_string_table`
- `add_string_table_entry`

### Persistence Lane

Use for:

- saving confirmed localization edits

Main tools:

- `save_asset`
- `save_dirty_assets`

### Optional Inspection Lane

Use for:

- verifying whether adjacent localization helpers are actually registered in the current build

Main tools:

- `list_all_tools`
- `describe_tool`

## Proven Sequences

### New Table Pass

`create_string_table` -> `add_string_table_entry` -> `save_asset`

### Existing Entry Update

`add_string_table_entry` -> `save_asset` or `save_dirty_assets`

## Rules

- Do not design keys from transient wording.
- Do not reuse one key for multiple meanings.
- Do not rely on optional helpers without checking the live registry.
- Do not leave confirmed String Table edits unsaved.
- Do not call localization done just because the English source text exists.

## Key Takeaway

The localization surface is strongest when you separate:

- meaning design
- key design
- table authoring
- save/verification

Even a thin tooling surface needs strict discipline to stay usable.
