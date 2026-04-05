# Design: Fix Simple Gaps

## Context

All five issues are isolated to three files. No new AppMessage keys, no new persist keys, no architectural changes.

## Goals / Non-Goals

**Goals:**
- Correct mmol/L negative delta rendering
- Safe timestamp fallback with no NaN
- Named config URL constant
- Dexcom username round-trip through settings
- Graph Window exposed in settings UI

**Non-Goals:**
- Show Graph toggle (would require a new AppMessage key and main.c layer logic)
- Vibrate on Range Return (new feature, separate change)
- nsToken / dexcomPass re-population (intentional: secrets don't repopulate)

## Decisions

### Decision 1: Negative delta - use abs() and explicit sign

Compute `whole` and `frac` from `abs(delta_mgdl)`, then prefix with "+" or "-" based on sign.
Avoids the signed integer division truncation that produces "0.-4".

### Decision 2: dateString fallback - new Date().getTime()

`new Date(isoString).getTime()` is the standard JS parse for ISO 8601.
Wrap in `|| 0` so a completely unparseable string degrades to 0 rather than NaN.

### Decision 3: CONFIG_URL constant

Place `var CONFIG_URL = '...';` near the top of index.js with a prominent comment.
No framework abstraction needed - a simple var is sufficient.

### Decision 4: graphWindow values - points not hours

The JS already uses point counts (37 = 3hr). Map the UI labels to point counts:
- "1 hour" = 13 points (12 intervals × 5min + 1)
- "2 hours" = 25 points
- "3 hours" = 37 points (default)

Store as the point count string so existing JS logic (`parseInt(settings.graphWindow)`) needs no change.
