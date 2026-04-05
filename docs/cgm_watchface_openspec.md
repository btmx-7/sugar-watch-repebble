# GlucoseGuard — OpenSpec Framework
### Pebble Time 2 CGM Watchface · Spring 2026 Contest Entry

---

## 0. Meta

| Field | Value |
|---|---|
| **App Name** | GlucoseGuard |
| **App Kind** | Watchface |
| **Target Platform** | Emery (Pebble Time 2, 200×228 px, 64-color e-paper) |
| **Secondary Platform** | Gabbro (Pebble Round 2, 260×260 px) |
| **Contest** | rePebble Spring 2026 App Contest |
| **Prize Target** | Transparent Time 2 (Judge's Choice) + Most Hearts |
| **SDK** | Pebble SDK ≥ 4.9.148, C |
| **Data Source** | Nightscout (open protocol), Dexcom Share, LibreLink (via middleware) |
| **Version** | 1.0.0 |

---

## 1. Problem Statement

### 1.1 The User

People living with Type 1 or Type 2 diabetes who use a CGM (Continuous Glucose Monitor — Dexcom G6/G7, FreeStyle Libre, Medtronic) rely on glancing at data **dozens of times per day**. For these users, the watch is not a fashion accessory — it is a **life-critical at-a-glance medical instrument**.

Existing Pebble CGM watchfaces (Simple CGM, T1000, Dexcom Share CGM) were designed for the old 144×168 Basalt/Diorite screens. None are optimised for the **new Pebble Time 2 Emery platform** (200×228, 64 colors, touch, Quick View).

### 1.2 The Opportunity

The Pebble Time 2's larger 64-color e-paper display allows:
- A **proper 3-hour glucose trend graph** (was too small before)
- **Color-coded status zones** (green/amber/red) that are instantly readable
- **Rich secondary data** (time, date, battery, BT status, trend arrow, delta, last-read time)
- **Touch interaction** for a quick "calibration needed" dismiss or mode toggle

### 1.3 Why This Wins the Contest

The judging criteria is: **creativity, cleverness, good use of new platforms, and design.**

- ✅ Creativity: CGM on Pebble is a beloved but historically broken use case — reviving it properly for PT2 is both meaningful and emotionally resonant
- ✅ Cleverness: Uses color zones, adaptive layout for Quick View, touch dismiss for alerts — all new PT2 capabilities
- ✅ Good use of new platforms: Full use of Emery 200×228 color canvas; Round 2 (Gabbro) secondary support
- ✅ Design: Medical-grade clarity + beautiful minimalist aesthetic inspired by clinical UI (not toy/gaming)
- ✅ Hearts: The T1D/T2D community is large, organized, and vocal — high organic heart potential

---

## 2. User Needs (Jobs-to-be-Done)

### Primary User: Diabetic Adult / Teen

| Job | Outcome | Priority |
|---|---|---|
| See my current glucose value instantly | Large, readable number, color-coded by zone | MUST |
| Understand my trend direction | Trend arrow (↑↑ ↑ → ↓ ↓↓) next to value | MUST |
| See if I'm going low soon | Color/flash warning when predicted low in <20min | MUST |
| See the last 3 hours of glucose history | Miniature sparkline graph | MUST |
| Know how stale the data is | "Last read: 4 min ago" timestamp | MUST |
| Know my rate of change (delta) | "+12 mg/dL" or "-8 mg/dL" in last reading | MUST |
| Still see time and date | Time prominent, date secondary | MUST |
| Not miss an urgent alert when wrist is down | Vibration pattern for low/high | MUST |
| Silence an alert quickly | Touch anywhere on screen to dismiss | SHOULD |
| See battery / BT status | Small icons, non-intrusive | SHOULD |
| Use metric (mmol/L) OR imperial (mg/dL) | User setting | SHOULD |
| Personalize alert thresholds | High/low threshold settings | SHOULD |
| Work when Quick View is active | Adaptive layout | MUST |

### Secondary User: Parent of Diabetic Child ("Follower")

| Job | Outcome | Priority |
|---|---|---|
| Monitor child's glucose remotely via Nightscout | Same display as primary user | MUST |
| Get alerted when child's glucose is critical | Vibration alert pattern | MUST |

---

## 3. Design Specifications

### 3.1 Screen Layout — Emery (200×228 px) — DEFAULT STATE

```
┌─────────────────────────────┐  y=0
│  ⚡[battery]    [BT]  HH:MM │  y=0–20   STATUS BAR
├─────────────────────────────┤  y=20
│                             │
│   ████ 127 ↗ +8            │  y=25–85  GLUCOSE BLOCK
│   mg/dL  [IN RANGE]        │           (Large value, arrow, delta, unit, zone label)
│                             │
├─────────────────────────────┤  y=90
│  ╭──────────────────────╮  │
│  │  3-HOUR GRAPH        │  │  y=95–160  SPARKLINE
│  │  ══════──────        │  │            (with 70/180 threshold lines)
│  ╰──────────────────────╯  │
├─────────────────────────────┤  y=165
│  Last read: 4 min ago       │  y=168–185  STALENESS ROW
│  Wed Apr 09                 │  y=185–205  DATE
└─────────────────────────────┘  y=228
```

**Color Zones:**
| Range | Color | Label |
|---|---|---|
| < 55 mg/dL | GColorRed | URGENT LOW |
| 55–69 mg/dL | GColorOrange | LOW |
| 70–180 mg/dL | GColorGreen (or GColorMintGreen) | IN RANGE |
| 181–250 mg/dL | GColorChromeYellow | HIGH |
| > 250 mg/dL | GColorRed | URGENT HIGH |

### 3.2 Screen Layout — ALERT STATE

When glucose is < 70 or > 250:
- Background flashes between black and zone color (1Hz) via `app_timer`
- Large text "⚠ LOW" or "⚠ HIGH" replaces zone label
- Haptic: `vibes_enqueue_custom_pattern()` — triple pulse
- Touch anywhere (or SELECT button) → dismiss alert for 15 minutes

### 3.3 Screen Layout — STALE DATA STATE

When last reading > 15 minutes ago:
- Glucose number grays out (`GColorLightGray`)
- "⚠ NO DATA" shown below number
- "Last read: 17 min ago" shown in red

### 3.4 Quick View / Timeline Peek Adaptation

When Quick View panel is active (bottom ~51px covered):
- Date row hidden
- Staleness row moves up
- Graph height compressed (to ~50px)
- Glucose value remains at full size — it is the most critical element

### 3.5 Gabbro (Round, 260×260) Layout

- Glucose value centered at 50% y
- Trend arrow to the right of value
- Circular graph arc along bottom 40% of circle
- Time in top arc
- Date + staleness in bottom arc

### 3.6 Typography

| Element | Font | Size | Notes |
|---|---|---|---|
| Glucose value | FONT_KEY_LECO_38_BOLD_NUMBERS | 38px | Numbers only — Leco is perfect for medical numerics |
| Trend arrow / delta | FONT_KEY_GOTHIC_24_BOLD | 24px | |
| Zone label | FONT_KEY_GOTHIC_18_BOLD | 18px | Color-coded |
| Time | FONT_KEY_BITHAM_30_BLACK | 30px | |
| Date / staleness | FONT_KEY_GOTHIC_14 | 14px | |
| Alert text | FONT_KEY_BITHAM_42_BOLD | 42px | Alert state only |

### 3.7 Graph Specification

- **X axis**: 3 hours of data (180 data points max, but render ~36 points = one per 5 min)
- **Y axis**: Adaptive range — min(readings)-20 to max(readings)+20, clamped to [40, 400] mg/dL
- **Threshold lines**: Dashed horizontal lines at 70 mg/dL (low) and 180 mg/dL (high)
- **Color**: Each segment colored by its zone (green in range, red for low, yellow for high)
- **Current value dot**: Filled circle at rightmost data point, colored by current zone

---

## 4. Data Architecture

### 4.1 Data Source Protocol

The watchface communicates via the Pebble JavaScript (`pebblekit-js`) **AppMessage** bridge.

**Phone-side JS fetches** from a Nightscout-compatible URL:
```
GET {NS_URL}/api/v1/entries.json?count=37&token={API_SECRET}
```

Response is an array of:
```json
{
  "sgv": 127,
  "direction": "FortyFiveUp",
  "date": 1744123456789,
  "delta": 8
}
```

### 4.2 AppMessage Keys (C ↔ JS)

```c
#define KEY_GLUCOSE_VALUE    0   // int32 — current glucose in mg/dL
#define KEY_GLUCOSE_TREND    1   // int32 — trend enum (0=DoubleUp...6=DoubleDown)
#define KEY_GLUCOSE_DELTA    2   // int32 — delta mg/dL (signed)
#define KEY_LAST_READ_SEC    3   // int32 — unix seconds since last reading
#define KEY_GRAPH_DATA       4   // byte[] — up to 37 uint8 values (glucose/2 to fit byte)
#define KEY_USE_MMOL         5   // int32 — 0=mg/dL, 1=mmol/L
#define KEY_HIGH_THRESHOLD   6   // int32 — default 180
#define KEY_LOW_THRESHOLD    7   // int32 — default 70
#define KEY_URGENT_HIGH      8   // int32 — default 250
#define KEY_URGENT_LOW       9   // int32 — default 55
```

### 4.3 Refresh Interval

Phone JS polls every **5 minutes** (matching CGM transmission interval). On first load, fetches immediately. Uses `setInterval(fetch, 5*60*1000)`.

### 4.4 Data Persistence

Last known glucose value stored in `persist_write_int(KEY_GLUCOSE_VALUE, value)` so watchface shows last known data immediately on wake — never blank.

---

## 5. Alert & Haptic Specification

| Condition | Visual | Haptic Pattern | Repeat |
|---|---|---|---|
| Urgent Low (< 55) | Red flash, "⚠ URGENT LOW" | 3× long pulse (300ms on, 200ms off) | Every 5 min until dismissed |
| Low (55–69) | Orange border pulse | 2× short pulse | Once per event |
| High (181–250) | Yellow border | 1× short pulse | Once per event |
| Urgent High (> 250) | Red flash, "⚠ URGENT HIGH" | 3× long pulse | Every 5 min until dismissed |
| Stale Data > 15min | Gray glucose, "⚠ NO DATA" | 2× medium pulse | Once |
| Bluetooth disconnect | BT icon flashes | 1× short pulse | Once |

---

## 6. Settings (Clay)

Via Clay settings page (JavaScript config page):

| Setting | Type | Default | Description |
|---|---|---|---|
| Nightscout URL | Text | "" | e.g. https://mysite.ns.io |
| API Secret | Text | "" | Nightscout token (stored hashed) |
| Glucose Unit | Toggle | mg/dL | mg/dL or mmol/L |
| Low Threshold | Number | 70 | Alert below this value |
| High Threshold | Number | 180 | Alert above this value |
| Urgent Low | Number | 55 | Flash + urgent haptic below |
| Urgent High | Number | 250 | Flash + urgent haptic above |
| 12/24hr Time | Toggle | 12hr | Time format |
| Show Graph | Toggle | ON | Show/hide sparkline |
| Graph Window | Select | 3hr | 1hr / 2hr / 3hr |
| Vibrate on Range Return | Toggle | ON | Haptic when re-entering range |

---

## 7. Technical Architecture

### 7.1 File Structure

```
glucoseguard/
├── package.json
├── src/
│   ├── c/
│   │   └── main.c          # All watchface C code
│   └── pkjs/
│       ├── index.js        # Phone-side JS (fetch + AppMessage)
│       └── config.html     # Clay settings page
├── resources/
│   └── images/
│       ├── icon.png
│       └── banner.png
└── wscript
```

### 7.2 Memory Budget (Pebble Time 2 / Emery)

| Resource | Budget | Notes |
|---|---|---|
| App RAM | ~25KB | Pebble app heap |
| Graph buffer | 37 × 1 byte = 37 bytes | Uint8 array |
| String buffers | ~200 bytes | Various static char[] |
| Layers | ~15 | TextLayers + custom DrawLayer |

### 7.3 Battery Considerations

- `tick_timer_service_subscribe(MINUTE_UNIT, ...)` — not SECOND_UNIT
- No continuous animation — only on alert state
- Graph redraws only when new data arrives via AppMessage
- Alert flash uses `app_timer` at 1000ms interval, cancelled on dismiss

---

## 8. Publication Checklist

### Required for Contest Entry (April 2–19, 2026)

- [ ] App created date ≥ April 2, 2026
- [ ] Supports **Emery** (Pebble Time 2) platform — primary
- [ ] Supports **Gabbro** (Round 2) platform — secondary
- [ ] Quick View / Timeline Peek implemented (`UnobstructedArea` API)
- [ ] Published via `pebble publish` CLI or CloudPebble
- [ ] App store banner/icon uploaded
- [ ] Screenshots/GIF auto-generated by CLI
- [ ] App description references CGM/diabetes use case
- [ ] Source code public on GitHub (for community hearts + trust)

### Recommended to Maximize Hearts

- [ ] Post in rePebble Discord / community forums
- [ ] Tweet/Bsky with #PebbleContest #T1D #CGM
- [ ] Cross-post to r/diabetes, r/pebble, TuDiabetes forums
- [ ] Post in Nightscout Facebook groups
- [ ] Demo GIF showing color zones and alert flash

---

## 9. Risk & Mitigations

| Risk | Impact | Mitigation |
|---|---|---|
| Nightscout URL varies per user | Users can't set up | Clay settings page with clear instructions |
| Dexcom Share doesn't use Nightscout | Large user segment missed | Phone JS supports both Nightscout and Dexcom Share API |
| Data staleness causes false comfort | Medical risk | Prominent staleness display, alert after 15min |
| e-paper refresh on alert flash | Visual artifacts | Limit to 1Hz, use `layer_mark_dirty()` carefully |
| mmol/L conversion precision | Wrong values displayed | Multiply mg/dL × 0.0555, round to 1 decimal |

---

## 10. Success Metrics

| Metric | Target | Why |
|---|---|---|
| Hearts on leaderboard | Top 5 in phase 1 (Apr 2–12) | Wins a watch via Most Hearts |
| Judge's score | Top 3 | Wins transparent Time 2 |
| GitHub stars | >20 | Signal of quality + community trust |
| Community posts sharing it | >3 forums | Drives organic hearts |
