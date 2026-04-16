# 02 - UE5 String Table And Culture Systems

RiftbornAI's current localization surface is thin and centered on String Table authoring.

## String Tables Are The Main Supported Container

Use:

- `create_string_table`
- `add_string_table_entry`

for the main production lane.

This is the right path for:

- creating a localization table asset
- adding or updating stable key/value entries

## String Table Work Is Still Real Authoring

The tool surface may be smaller than other domains, but the consequences are still important:

- keys become contracts
- entry values become source text
- later translation and integration depend on them remaining coherent

## Adjacent Inspection Helpers May Exist

The repo also contains generated helpers such as:

- `get_string_table_entries`
- `get_supported_cultures`

Treat these as optional adjacent helpers rather than guaranteed default-surface lanes.

Before planning around them, verify with:

- `list_all_tools`
- `describe_tool`

## Saving Still Matters

String Table work should usually be followed by:

- `save_asset`
- `save_dirty_assets`

when the change is confirmed and should persist cleanly.

## Current Strength Of The Surface

The localization surface is strongest for:

- creating String Table assets
- adding or updating entries
- saving those authored changes

It is not a full translation-management platform, glossary system, or localization QA suite by itself.

## Key Takeaway

Treat the current localization lane as:

design keys -> create table -> add entries -> save intentionally

That is enough to require discipline even before broader localization tooling exists.
