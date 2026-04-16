# RiftbornAI Copilot — Slate-Compatible Symbols

## Usage in Slate C++
```cpp
// Preferred: STextBlock with Unicode (works in most UE fonts)
STextBlock::FArguments().Text(FText::FromString(TEXT("▶"))).Font(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"), 14))

// Or FSlateIcon for editor-style glyphs (better consistency)
FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Plus")
```

Use in `SButton`, `STextBlock`, or `SRichTextBlock`. Test rendering in copilot panel (dark theme works best with white/colored glyphs). Avoid overly complex emoji (rendering varies).

## Arrows (Navigation, Send, Expand/Collapse)
- **Right/Send**: → (U+2192), ➔ (U+2794), ➜ (U+279C), ➤ (U+27A4), ▶ (U+25B6), ▷ (U+25B7)
- **Left/Back**: ← (U+2190), ◀ (U+25C0)
- **Up**: ↑ (U+2191), ▲ (U+25B2)
- **Down/Expand**: ↓ (U+2193), ▼ (U+25BC)
- **Double**: ↔ (U+2194), ⇄ (U+21C4), ⟲ (U+27F2 loop)

**Copilot fit**: Use `➤` or `▶` for "Send" button (replaces old glyph). `▼` for dropdowns/timeline expand.

## Squares & Geometric (Status, Buttons, Cards)
- **Filled Square**: ■ (U+25A0), ◼ (U+25FC), ◆ (U+25C6)
- **Outline Square**: □ (U+25A1), ◻ (U+25FB), ◇ (U+25C7)
- **Rounded/Other**: ▣ (U+25A3), ▤ (U+25A4 patterned), ● (U+25CF circle), ○ (U+25CB)
- **Stop/Block**: ⏹ (U+23F9), ■ (solid stop)

**Copilot fit**: `■` or `⏹` for Stop button (centered as in recent commit). `◆` for active task cards. `□` for unchecked items in timeline.

## Status & Action Icons
- **Success/Check**: ✓ (U+2713), ✔ (U+2714), ✅ (U+2705)
- **Error/X**: ✕ (U+2715), ✖ (U+2716), ❌ (U+274C)
- **Warning**: ⚠ (U+26A0), △ (U+25B3)
- **Info/Help**: ℹ (U+2139), ⓘ (U+24D8)
- **Star/Priority**: ★ (U+2605), ☆ (U+2606)
- **Refresh/Retry**: ↻ (U+21BB), 🔄 (U+1F504)
- **Menu/More**: ⋮ (U+22EE vertical), ☰ (U+2630 hamburger)
- **Spark/AI**: ✦ (U+2726), ⚡ (U+26A1), ✨ (U+2728)

**Copilot fit**: 
- Send/Execute: `➤` or `✦`
- Stop: `⏹` or `■`
- Success (completed plan): `✓`
- Error/escalation: `⚠` or `✕`
- Timeline marker: `▶` or `◆`
- Git/PR status: `★` or `↻`

## Recommendations for Copilot Panel
1. **Send button**: `➤` or `▶` (matches recent glyph rebuild).
2. **Stop button**: `⏹` or `■` (centered Slate box as in b2a7318).
3. **Task states**: `✦` (running AI), `✓` (done), `⚠` (needs escalation), `⏹` (stopped).
4. **Timeline**: `▶` for steps, `▼` for expand, `◆` for current.
5. **Theme**: Use white `#FFFFFF` or accent blue on dark background (`CopilotPanelColors.h`).
6. **Font**: Stick to `FCoreStyle::Get().GetFontStyle("NormalFont")` or Roboto for best Unicode support. Add custom font asset if needed for more icons (e.g. Font Awesome).

Add to `SRiftbornCopilotPanel.cpp` or `SAgentTimeline.cpp` via `STextBlock` inside buttons. All tested symbols render cleanly in UE5 Slate.

Update `docs/` or `CopilotPanel_MessageWidgets.cpp` with any new glyphs.
