# Steady: Pebble App Store Publishing Guide

## Status

Build: ✓ Complete (Steady-watchface.pbw, 154KB)
Screenshots: ✓ Complete (3 assets)
Metadata: ✓ Complete (package.json)
SDK: ⚠ Requires installation

---

## What's Ready

### Build Artifact
- **File**: `build/Steady-watchface.pbw` (154 KB)
- **Platforms**: emery, gabbro, basalt, diorite, chalk (all 5 platforms)
- **Last built**: 2026-04-16 19:09

### Screenshots (in `resources/screenshots/`)
1. `screenshot_T2_simple_dark.png` — Time 2 Simple layout (200×228)
2. `screenshot_R2_simple_dark.png` — Round 2 Simple layout (260×260)

### Metadata
- **Display Name**: Steady
- **Short Description** (package.json): "A clean watchface for Pebble Time 2 and Round 2. Large clock, 4 configurable slots, and a built-in CGM widget. Glucose monitoring that fits in."
- **Long Description** (package.json): Clean watchface framing with CGM as a natural widget, not a medical device identity
- **UUID**: 552fd91e-ad93-4d0f-ae44-74bc9d3108d6 (unchanged)
- **Version**: 1.0.0
- **Target Platforms**: Time 2 (emery), Round 2 (gabbro), Time (basalt), Steel (diorite), Round (chalk)

---

## Prerequisites: Pebble SDK Installation

The `pebble publish` command requires the **Pebble SDK** (v4.x or compatible).

### Check if Installed
```bash
pebble --version
```

If this fails, install the SDK:

### macOS (using Homebrew)
```bash
brew install pebble-sdk
```

### Other Platforms
Download from https://github.com/pebble/pebble-sdk-release or use the official installer.

Once installed, verify:
```bash
pebble --version
```

---

## Publishing Workflow

### Step 1: Authenticate with Pebble Account
```bash
cd /path/to/Steady-watchface
pebble login
```

This opens a browser window for Firebase OAuth. You'll need a Pebble account (or create one).

Verify login status:
```bash
pebble login --status
```

### Step 2: Publish to App Store
```bash
pebble publish
```

The command will:
1. Read metadata from `package.json`
2. Read the built PBW from `build/Steady-watchface.pbw`
3. Prepare screenshots from `resources/screenshots/`
4. Upload to Pebble App Store

Expected output:
```
Uploading app...
[... progress ...]
App published successfully!
UUID: 552fd91e-ad93-4d0f-ae44-74bc9d3108d6
View at: https://apps.repebble.com/applications/552fd91e-ad93-4d0f-ae44-74bc9d3108d6
```

### Step 3: Verify Listing
Visit the returned URL (or check https://apps.repebble.com) and confirm:
- ✓ App name: "Steady"
- ✓ All 3 screenshots display correctly
- ✓ Short description visible
- ✓ Long description complete
- ✓ Platform list includes: Time 2, Round 2, Time, Steel, Round
- ✓ Author: "Michaël Blancon Tardi"

---

## Submission Details for App Store

### Key Details
| Field | Value |
|-------|-------|
| App Name | Steady |
| Version | 1.0.0 |
| UUID | 552fd91e-ad93-4d0f-ae44-74bc9d3108d6 |
| Category | Health / Utilities |
| Author | Michaël Blancon Tardi |

### Supported Platforms
- Pebble Time 2 (emery) — 200×228 color e-paper
- Pebble Round 2 (gabbro) — 260×260 circular color e-paper
- Pebble Time (basalt) — 144×168 color
- Pebble Steel (diorite) — 144×168 rectangular
- Pebble Round (chalk) — 180×180 circular

### Feature Summary
- Clean watchface design. Large clock and 4 configurable widget slots.
- Built-in CGM widget. Glucose stays visible alongside other data. It does not dominate the face.
- Each slot is assignable to one of the following:
  - Battery
  - Weather
  - Heart rate
  - Steps
  - Glucose
- Color-coded glucose zones:
  - Urgent low/high: red
  - Low: orange
  - In range: accent color
  - High: yellow
- Haptic and visual alerts on urgent zones
- CGM sources: Nightscout and Dexcom Share
- Weather via OpenMeteo. No API key required.
- Quick View (compact mode) support on all platforms

---

## After Publishing

### Immediate
The app becomes available in the Pebble App Store within minutes. Users can install via their Pebble phone app.

### Visibility
- Listed under Health category
- Searchable by "Steady", "CGM", "glucose", "diabetes", "weather"
- Visible in rePebble app store (https://apps.repebble.com)

### Contest (April 2-19, 2026)
This app qualifies for the Pebble Spring 2026 Contest:
- Team Judging categories: Creativity, Cleverness, New Platform Use, Design
- Both new platforms represented (T2 and R2)
- Quick View support included
- Visual polish demonstrated

---

## Troubleshooting

### "pebble: command not found"
→ Install Pebble SDK (see Prerequisites section)

### "not logged in" error
```bash
pebble login
```

### "PBW file not found" error
Ensure `build/Steady-watchface.pbw` exists:
```bash
ls -lh build/Steady-watchface.pbw
```

If missing, rebuild:
```bash
pebble build
```

### Screenshots not uploading
Check that these files exist and are valid PNG:
- `resources/screenshots/screenshot_T2_simple_dark.png`
- `resources/screenshots/screenshot_T2_dashboard_dark.png`
- `resources/screenshots/screenshot_R2_simple_dark.png`

---

## Next Steps (Post-Publishing)

1. Share app link in Pebble community forums
2. Update personal Pebble app store listing with release notes
3. Monitor community feedback for bug reports
4. Plan v2.1 with deferred features: light mode, color themes, audio indicator

---

## Reference

- **Pebble SDK Docs**: https://pebble.github.io/
- **rePebble App Store**: https://apps.repebble.com/
- **Package Manifest**: `package.json` (sdkVersion: 3, all metadata)
- **App UUID**: 552fd91e-ad93-4d0f-ae44-74bc9d3108d6
