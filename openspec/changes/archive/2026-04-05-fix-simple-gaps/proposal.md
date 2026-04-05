# Proposal: Fix Simple Gaps

## Why

Five bugs and missing features were identified during initial codebase review of GlucoseGuard. None require architectural changes, but left unfixed they would cause incorrect data display (mmol/L delta), silent data loss (NaN timestamps), a broken settings page (dead URL), and a missing user preference (graph window).

## What Changes

- Negative mmol/L delta values display correctly (e.g. "-0.4" instead of "0.-4")
- lastRead timestamp no longer silently becomes NaN when `latest.date` is absent
- The config page URL is a named constant, clearly marked for update before publishing
- Dexcom username is preserved when the user reopens settings
- Graph Window (1hr/2hr/3hr) is exposed in the settings UI

## Capabilities

### Fixed Capabilities

- `display-delta-mmol`: Negative delta in mmol/L mode now renders with correct sign and magnitude
- `data-freshness-tracking`: lastRead timestamp uses a safe fallback when the `date` field is absent
- `settings-page`: Config URL is a maintainable constant; Dexcom username and graph window survive a settings round-trip

### New Capabilities

- `graph-window-setting`: User can choose 1hr, 2hr, or 3hr graph history from settings

## Impact

- `src/c/main.c`: Fix negative mmol/L delta math
- `src/pkjs/index.js`: Fix timestamp fallback, extract CONFIG_URL, add dexcomUser + graphWindow to config URL params
- `src/pkjs/config.html`: Load dexcomUser on open, add Graph Window select, include graphWindow in save()
