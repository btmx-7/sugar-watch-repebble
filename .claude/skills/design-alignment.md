# Design Alignment Skill

Align Pebble watchface builds with Figma designs. Use when design changes happen for any layout (Simple, Dashboard) or device format (T2 emery, R2 gabbro).

## When to use

- After Figma design updates that change icons, colors, layout, or spacing
- When adding new widget types or watchface variants
- When porting a layout to a new platform (e.g., Simple from emery to gabbro)
- When verifying visual fidelity between build and design

## Architecture

### Dual icon fonts

The project uses two Material Symbols Rounded font resources for state-dependent icon rendering:

| Resource ID | Font File | Usage |
|---|---|---|
| `MATERIAL_SYMBOLS_16` | `MaterialSymbolsRounded_Filled_28pt-Medium.ttf` | Active/data-present states (filled icons) |
| `MATERIAL_SYMBOLS_REGULAR_16` | `MaterialSymbolsRounded_28pt-Medium.ttf` | Inactive/error/empty states (outline icons) |

The `SlotRenderData.icon_filled` bool controls which font the renderer uses.

### Demo data

Build with `DEMO_DATA=1 pebble build` to inject fake data into all widget modules. This enables visual testing without a phone connection. The demo values are defined in `init()` inside `#ifdef DEMO_DATA`.

### Platforms

| Platform | Model | Resolution | Shape |
|---|---|---|---|
| emery | Pebble Time 2 | 200x228 | Rect |
| gabbro | Pebble Round 2 | 260x260 | Round |

Runtime detection: `bool is_r2 = (w == 260);`

## Icon codepoint workflow

When adding or changing icons, follow this process:

### 1. Identify icon names from Figma

In the Figma design context, icons use `font-["Material_Symbols_Rounded:Regular"]` with the icon name as text content (e.g., `favorite`, `battery_android_3`, `sunny`).

### 2. Extract codepoints from TTF

```bash
pip3 install fonttools
python3 << 'EOF'
from fontTools.ttLib import TTFont
font = TTFont("resources/fonts/MaterialSymbolsRounded_28pt-Medium.ttf")
cmap = font.getBestCmap()
name_to_cp = {}
for cp, name in cmap.items():
    if name not in name_to_cp:
        name_to_cp[name] = cp

target = ["icon_name_here"]  # Replace with icon names from Figma
for name in sorted(target):
    if name in name_to_cp:
        cp = name_to_cp[name]
        utf8 = ''.join(f'\\x{b:02x}' for b in chr(cp).encode('utf-8'))
        print(f'{name}: U+{cp:04X}  utf8={utf8}  regex=\\u{cp:04x}')
    else:
        print(f'{name}: NOT IN FONT')
font.close()
EOF
```

### 3. Update C code

Add `#define` in `src/c/main.c` (icon constants section):
```c
#define ICON_NAME  "\xXX\xXX\xXX"  // U+XXXX icon_name
```

### 4. Update characterRegex

Add the `\uXXXX` escape to the `characterRegex` in **both** font entries in `package.json`.

### 5. Verify

```bash
DEMO_DATA=1 pebble build
pebble install --emulator emery
pebble screenshot --emulator emery screenshot.png
```

## Verified codepoint reference

| Icon Name | Codepoint | UTF-8 | Category |
|---|---|---|---|
| error | U+E000 | `\xee\x80\x80` | Alert |
| cardiology | U+E09C | `\xee\x82\x9c` | Heart Rate |
| battery_full | U+E1A4 | `\xee\x86\xa4` | Battery |
| bluetooth | U+E1A7 | `\xee\x86\xa7` | Status |
| bluetooth_connected | U+E1A8 | `\xee\x86\xa8` | Status |
| bluetooth_disabled | U+E1A9 | `\xee\x86\xa9` | Status |
| cloud | U+E2BD | `\xee\x8a\xbd` | Weather |
| music_note | U+E3A1 | `\xee\x8e\xa1` | Status |
| directions_walk | U+E536 | `\xee\x94\xb6` | Steps |
| arrow_forward | U+E5C8 | `\xee\x97\x88` | Trend |
| arrow_upward | U+E5D8 | `\xee\x97\x98` | Trend |
| foggy | U+E818 | `\xee\xa0\x98` | Weather |
| sunny | U+E81A | `\xee\xa0\x9a` | Weather |
| favorite | U+E87D | `\xee\xa1\xbd` | Heart Rate |
| trending_up | U+E8E5 | `\xee\xa3\xa5` | Trend |
| keyboard_double_arrow_down | U+EAD0 | `\xee\xab\x90` | Trend |
| ac_unit | U+EB3B | `\xee\xac\xbb` | Weather |
| thunderstorm | U+EBDB | `\xee\xaf\x9b` | Weather |
| electric_bolt | U+EC1C | `\xee\xb0\x9c` | Battery |
| clear_day | U+F157 | `\xef\x85\x97` | Weather |
| mode_cool | U+F166 | `\xef\x85\xa6` | Weather |
| partly_cloudy_day | U+F172 | `\xef\x85\xb2` | Weather |
| rainy | U+F176 | `\xef\x85\xb6` | Weather |
| north | U+F1E0 | `\xef\x87\xa0` | Trend |
| north_east | U+F1E1 | `\xef\x87\xa1` | Trend |
| south | U+F1E3 | `\xef\x87\xa3` | Trend |
| south_east | U+F1E4 | `\xef\x87\xa4` | Trend |
| battery_android_bolt | U+F305 | `\xef\x8c\x85` | Battery |
| battery_android_alert | U+F306 | `\xef\x8c\x86` | Battery |
| battery_android_3 | U+F30A | `\xef\x8c\x8a` | Battery |
| battery_android_2 | U+F30B | `\xef\x8c\x8b` | Battery |
| battery_android_1 | U+F30C | `\xef\x8c\x8c` | Battery |
| cloud_alert | U+F3CC | `\xef\x8f\x8c` | Weather |
| pulse_alert | U+F501 | `\xef\x94\x81` | Heart Rate |
| rainy_light | U+F61E | `\xef\x98\x9e` | Weather |

## Critical files

| File | Purpose |
|---|---|
| `src/c/main.c` | Icon defines (line ~102), slot populate (~401), slot render (~479), layout (~876) |
| `package.json` | Font resources + characterRegex (line ~42), messageKeys |
| `wscript` | Build flags (DEMO_DATA) |
| `resources/fonts/` | MaterialSymbolsRounded TTF files (Filled + Regular) |
| `resources/screenshots/Steady-T2_emery/` | T2 reference screenshots |
| `resources/screenshots/Steady-R2_emery/` | R2 reference screenshots |
| `resources/tokens/` | Figma design tokens (colors, fonts, spacing) |

## Comparison checklist

When comparing build screenshot to Figma/reference:

1. **Icons visible:** All 4 widget slots show their icon glyph (not blank)
2. **Icon variant:** Filled icons for active states, Regular for inactive
3. **Arc rings:** Background track in border/subtle (#005555), fill in icon_color
4. **Colors:** Match design tokens (cyan #00FFFF, teal #55FFFF, white, etc.)
5. **Digit rendering:** H1=subtle, H2=inverted, M1=inverted, M2=default (cyan)
6. **Digit stroke:** Black outline behind fill digits (8-direction 4px offset)
7. **Slot positions:** T2: TL(4,4), TR(140,4), BL(4,168), BR(140,168). R2: inscribed in circle
8. **Day/month:** Flanking center vertically, text/subtle color
9. **BT icon:** Top center, filled when connected, regular when disconnected
10. **Music icon:** Bottom center when music playing
