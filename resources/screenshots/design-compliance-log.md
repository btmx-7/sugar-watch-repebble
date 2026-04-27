# Dashboard Design Compliance Log

Generated: 2026-04-23  
Scope: states d1–d5 vs design-export slides 1–5, both R2 (gabbro) and T2 (emery).

---

## BUG-01 · Slot 0 cropped — BOTH PLATFORMS · ALL STATES · CRITICAL

**Observed:** The leftmost slot badge (slot index 0) is clipped by the screen boundary on both platforms.

- R2 (round): circular clipping mask cuts ~30–40% of the left slot circle.
- T2 (rectangular): left edge of slot 0 runs off the screen left margin.

**Design intent:** All three slot badges are fully visible within the display area.

**Fix:** Increase left padding / shift slot 0 x-offset inward. On R2, account for the circular mask when computing the safe draw area for the leftmost slot.

---

## BUG-02 · Slot 2 cropped — BOTH PLATFORMS · ALL STATES · CRITICAL

**Observed:** The rightmost slot badge (slot index 2) is clipped symmetrically on the right edge.

- R2: circular mask cuts right side of slot 2.
- T2: slot 2 overflows the right screen margin.

**Design intent:** All three slots fully visible.

**Fix:** Mirror the fix for BUG-01 on the right side. Verify slot positions against display bounds before draw.

---

## BUG-03 · Unit label renders "ms/dL" instead of "mg/dL" — BOTH PLATFORMS · ALL STATES · HIGH

**Observed:** Every CGM reading in every state screenshot shows `ms/dL`. Consistent across both platforms and all 5 states inspected.

Examples:
- T2 d1: `Flat → 110 ms/dL`
- T2 d2: `Rapid fall ↓ 45 ms/dL`
- T2 d4: `Rise ↑ 195 ms/dL`
- R2 d1: `Flat ms/dL → 110`

**Cause (two candidates):**
1. String literal typo: `"ms/dL"` in source instead of `"mg/dL"`.
2. Font rendering: the Pebble bitmap `g` glyph at this size is indistinguishable from `s`.

**Fix:** Grep for unit string in source. If it's `"mg/dL"` (correct), switch to a font size where `g` is unambiguous, or test an alternative abbreviation. If it's `"ms/dL"`, correct the literal.

---

## BUG-04 · Clock colon missing / font mismatch — BOTH PLATFORMS · ALL STATES · HIGH

**Observed:** Design shows a clean `HH:MM` time string with a visible colon (`:`) separator. State builds show digits with no visible colon — appears as `H  MM` (wide gap) or the colon is absent entirely.

- Design R2-1: `00:01`
- State R2 d1: `1 5` (no separator visible)

Additional mismatch: the digit weight and stroke style differ from the design reference. The design uses a smooth, slightly condensed sans-serif; the build uses a heavier bitmap font.

**Fix:**
1. Verify the colon character is included in the font resource for the clock layer's text style.
2. Cross-check the font file used in `window_load` against the Figma-specified typeface. If a system font is being substituted, replace with the correct embedded resource.

---

## BUG-05 · d5 urgent_high: alert overlay missing — BOTH PLATFORMS · HIGH

**Observed:** Design slide 5 (urgent_high) shows an intentional critical-alert mode:
- All secondary slot values replaced with `--`.
- Graph nearly empty.
- Alert icon `⊕` shown at bottom.
- `-- mg/dL` in the CGM field (values suppressed to direct attention).

State build d5_urgent_high shows no alert overlay:
- Full data rendered: `270 mg/dL`, rapid rise trend, red dot on graph.
- No alert icon, no suppression of secondary data.

**Fix:** Implement the critical-alert overlay behavior for urgent_high (and presumably urgent_low). When glucose crosses the urgent threshold, switch to the simplified alert layout shown in the design. If this behavior was intentionally dropped, update the design export to match implementation.

---

## BUG-06 · Graph rendering style differs — BOTH PLATFORMS · ALL STATES · MEDIUM

**Observed:**
- Design: glucose history rendered as a **dot trail** — individual dots spanning the hyper–hypo band, giving a temporal read of the trend.
- State: rendered as a **solid filled rectangle** with a single circular position marker on a horizontal baseline.

The state graph conveys current position only; the design conveys history + trajectory.

**Fix:** Implement the dot-trail render mode. Each historical reading should plot as a dot along the y-axis within the hyper/hypo band. Confirm the number of history points available from the mock data feed.

---

## BUG-07 · Date position wrong on R2 — R2 ONLY · ALL STATES · MEDIUM

**Observed:**
- Design R2: date `24 02` (day + month) rendered inline, adjacent to the clock in the upper-center area.
- State R2: date digits (`23`, `4`) rendered as a vertical right-edge sidebar, stacked outside the clock zone.

The sidebar placement conflicts with graph label readability and is not present in the design.

**Fix:** Move date rendering to the top-center area alongside the clock, matching the design layout. Remove or refactor the right-sidebar date column.

---

## BUG-08 · Hyper/hypo scale values differ — BOTH PLATFORMS · ALL STATES · LOW

**Observed:**
- Design: hyper threshold `230`, hypo threshold `65`.
- State R2/T2: hyper `180`, hypo `70` (visible on some states).

This may reflect different threshold configs in mock data vs design. If thresholds are user-configurable, this is expected. If hardcoded in the design and should match the default, align the default values.

**Fix:** Confirm default threshold values in `config.h` or equivalent. If `180/70` is the correct default, update design reference. If `230/65` is intended, fix the mock.

---

## BUG-09 · Trend label includes name in state but not design (T2) — T2 ONLY · MEDIUM

**Observed:**
- Design T2 right sidebar: shows only arrow + value + unit (e.g., `→ 120 mg/dL`).
- State T2 right sidebar: prepends the trend name (e.g., `Flat → 110 ms/dL`, `Rapid fall ↓ 45 ms/dL`).

The trend name consumes vertical space and may be intentional (added post-design). If so, update design. If not, remove from the layout.

**Fix:** Align with PM on whether trend name belongs in the sidebar. If yes, update design. If no, strip the label string from the sidebar render and show only arrow + value + unit.

---

## States without design reference (no fix needed, for awareness)

The following state captures have no corresponding design export slide:

| State | Both platforms |
|---|---|
| d6_stale | No design slide |
| d7_btoff | No design slide |
| d8_error | No design slide |
| d9_slots_hr_steps_cgm | T2 only, no design slide |

Recommend adding design slides 6–8 (and 9–12 for slot variants) to the Figma file to enable full QA coverage.

---

## Issue summary

| ID | Description | Platforms | Severity |
|---|---|---|---|
| BUG-01 | Slot 0 cropped | R2 + T2 | Critical |
| BUG-02 | Slot 2 cropped | R2 + T2 | Critical |
| BUG-03 | `ms/dL` unit label | R2 + T2 | High |
| BUG-04 | Clock colon missing + font mismatch | R2 + T2 | High |
| BUG-05 | d5 urgent_high alert overlay absent | R2 + T2 | High |
| BUG-06 | Graph dot-trail not implemented | R2 + T2 | Medium |
| BUG-07 | Date in right sidebar instead of top-center | R2 | Medium |
| BUG-08 | Hyper/hypo threshold values differ | R2 + T2 | Low |
| BUG-09 | Trend name prepended in sidebar | T2 | Medium |
