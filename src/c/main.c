/**
 * Steady — CGM Watchface for Pebble Time 2 (Emery) & Round 2 (Gabbro)
 * Spring 2026 rePebble Contest Entry
 *
 * Two layouts: Simple (2-row time hero, 4 corner widget slots)
 *              Dashboard (time center, 3 top slots, sparkline graph + CGM panel)
 * 4 configurable slots: Battery, Weather, Heart Rate, Steps, CGM
 * Material Symbols Rounded icons, Inter Black 64px time font
 * Pebble Health API (HR + Steps), OpenMeteo weather via pkjs
 */

#include <pebble.h>

// ─── Design System Colors ────────────────────────────────────────────────────
// Source: Figma variables (bKKqEkSN0q1rOdsEX8OpaE)
// Pebble GColor: argb = 11_RR_GG_BB, 2 bits/channel (0=0x00,1=0x55,2=0xAA,3=0xFF)
#define CLR_TEXT_SUBTLE    ((GColor){.argb = 0xEF})  // #AAFFFF text/subtle
#define CLR_TEXT_INVERTED  ((GColor){.argb = 0xFF})  // #FFFFFF text/inverted
#define CLR_TEXT_DEFAULT   ((GColor){.argb = 0xCF})  // #00FFFF text/default
#define CLR_ICON_DEFAULT   ((GColor){.argb = 0xDF})  // #55FFFF icon/default
#define CLR_ICON_SUBTLE    ((GColor){.argb = 0xCA})  // #00AAAA icon/subtle
#define CLR_BORDER_SUBTLE  ((GColor){.argb = 0xC5})  // #005555 surface/border/subtle
#define CLR_STATE_DANGER   ((GColor){.argb = 0xF0})  // #FF0000 state/danger
#define CLR_STATE_WARNING  ((GColor){.argb = 0xF8})  // #FFAA00 state/warning
#define CLR_STATE_POSITIVE ((GColor){.argb = 0xCE})  // #00FFAA state/positive
#define CLR_STATE_INACTIVE ((GColor){.argb = 0xEA})  // #AAAAAA state/inactive
#define CLR_STATE_DISABLED ((GColor){.argb = 0xD5})  // #555555 state/disabled

// ─── AppMessage Keys ─────────────────────────────────────────────────────────
#define KEY_GLUCOSE_VALUE   0
#define KEY_GLUCOSE_TREND   1
#define KEY_GLUCOSE_DELTA   2
#define KEY_LAST_READ_SEC   3
#define KEY_GRAPH_DATA      4
#define KEY_USE_MMOL        5
#define KEY_HIGH_THRESHOLD  6
#define KEY_LOW_THRESHOLD   7
#define KEY_URGENT_HIGH     8
#define KEY_URGENT_LOW      9
#define KEY_WEATHER_TEMP    10
#define KEY_WEATHER_ICON    11
#define KEY_LAYOUT          12
#define KEY_SLOT_0          13
#define KEY_SLOT_1          14
#define KEY_SLOT_2          15
#define KEY_SLOT_3          16

// ─── Persistence Keys ────────────────────────────────────────────────────────
#define PERSIST_GLUCOSE     100
#define PERSIST_TREND       101
#define PERSIST_DELTA       102
#define PERSIST_LAST_READ   103
#define PERSIST_USE_MMOL    104
#define PERSIST_HIGH_THRESH 105
#define PERSIST_LOW_THRESH  106
#define PERSIST_URG_HIGH    107
#define PERSIST_URG_LOW     108
#define PERSIST_LAYOUT      109
#define PERSIST_SLOT_0      110
#define PERSIST_SLOT_1      111
#define PERSIST_SLOT_2      112
#define PERSIST_SLOT_3      113
#define PERSIST_WEATHER_TMP 114
#define PERSIST_WEATHER_ICN 115

// ─── Enums ───────────────────────────────────────────────────────────────────

typedef enum {
  TREND_DOUBLE_UP = 0,
  TREND_SINGLE_UP,
  TREND_FORTY_FIVE_UP,
  TREND_FLAT,
  TREND_FORTY_FIVE_DOWN,
  TREND_SINGLE_DOWN,
  TREND_DOUBLE_DOWN,
  TREND_NONE
} GlucoseTrend;

typedef enum {
  ZONE_URGENT_LOW,
  ZONE_LOW,
  ZONE_IN_RANGE,
  ZONE_HIGH,
  ZONE_URGENT_HIGH,
  ZONE_UNKNOWN
} GlucoseZone;

typedef enum {
  LAYOUT_SIMPLE    = 0,
  LAYOUT_DASHBOARD = 1
} WatchLayout;

typedef enum {
  SLOT_NONE      = 0,
  SLOT_BATTERY   = 1,
  SLOT_WEATHER   = 2,
  SLOT_HEART_RATE = 3,
  SLOT_STEPS     = 4,
  SLOT_CGM       = 5
} SlotType;

// ─── Material Symbols UTF-8 Glyph Constants ─────────────────────────────────
// Codepoints verified via fonttools extraction from MaterialSymbolsRounded TTF.
// Each constant is the UTF-8 byte sequence for the Unicode codepoint.

// Trend arrows
#define ICON_TREND_DOUBLE_UP    "\xef\x87\xa0\xef\x87\xa0"  // U+F1E0 north x2
#define ICON_TREND_SINGLE_UP    "\xee\x97\x98"               // U+E5D8 arrow_upward
#define ICON_TREND_45_UP        "\xef\x87\xa1"               // U+F1E1 north_east
#define ICON_TREND_FLAT         "\xee\x97\x88"               // U+E5C8 arrow_forward
#define ICON_TREND_45_DOWN      "\xef\x87\xa4"               // U+F1E4 south_east
#define ICON_TREND_SINGLE_DOWN  "\xef\x87\xa3"               // U+F1E3 south
#define ICON_TREND_DOUBLE_DOWN  "\xee\xab\x90"               // U+EAD0 keyboard_double_arrow_down
#define ICON_TREND_NONE         "-"

// Status icons
#define ICON_BT_CONNECTED       "\xee\x86\xa8"   // U+E1A8 bluetooth_connected
#define ICON_BT_DISCONNECTED    "\xee\x86\xa9"   // U+E1A9 bluetooth_disabled
#define ICON_MUSIC              "\xee\x8e\xa1"   // U+E3A1 music_note

// Slot type icons: heart rate
#define ICON_HEART_RATE         "\xee\xa1\xbd"   // U+E87D favorite (heart)
#define ICON_CARDIOLOGY         "\xee\x82\x9c"   // U+E09C cardiology
#define ICON_PULSE_ALERT        "\xef\x94\x81"   // U+F501 pulse_alert

// Slot type icons: steps
#define ICON_STEPS              "\xee\x94\xb6"   // U+E536 directions_walk

// Slot type icons: battery (level-dependent per Figma)
#define ICON_BATTERY            "\xee\xb0\x9c"   // U+EC1C electric_bolt
#define ICON_BATTERY_FULL       "\xee\x86\xa4"   // U+E1A4 battery_full
#define ICON_BATTERY_ANDROID_1  "\xef\x8c\x8c"   // U+F30C battery_android_1 (low)
#define ICON_BATTERY_ANDROID_2  "\xef\x8c\x8b"   // U+F30B battery_android_2
#define ICON_BATTERY_ANDROID_3  "\xef\x8c\x8a"   // U+F30A battery_android_3
#define ICON_BATTERY_ANDROID_BOLT  "\xef\x8c\x85" // U+F305 battery_android_bolt
#define ICON_BATTERY_ANDROID_ALERT "\xef\x8c\x86" // U+F306 battery_android_alert

// Weather icons (indices 0-7 match weatherCodeToIconIndex in pkjs)
#define ICON_WEATHER_SUNNY      "\xee\xa0\x9a"   // U+E81A sunny
#define ICON_WEATHER_PARTLY     "\xef\x85\xb2"   // U+F172 partly_cloudy_day
#define ICON_WEATHER_CLOUD      "\xee\x8a\xbd"   // U+E2BD cloud
#define ICON_WEATHER_RAIN       "\xef\x85\xb6"   // U+F176 rainy
#define ICON_WEATHER_STORM      "\xee\xaf\x9b"   // U+EBDB thunderstorm
#define ICON_WEATHER_SNOW       "\xee\xac\xbb"   // U+EB3B ac_unit
#define ICON_WEATHER_FOG        "\xee\xa0\x98"   // U+E818 foggy
#define ICON_WEATHER_RAIN_LIGHT "\xef\x98\x9e"   // U+F61E rainy_light
#define ICON_WEATHER_COOL       "\xef\x85\xa6"   // U+F166 mode_cool
#define ICON_WEATHER_ALERT      "\xef\x8f\x8c"   // U+F3CC cloud_alert

// Error/Alert
#define ICON_ERROR              "\xee\x80\x80"   // U+E000 error

static const char* const s_weather_icons[8] = {
  ICON_WEATHER_SUNNY,   // 0 clear
  ICON_WEATHER_PARTLY,  // 1 partly cloudy
  ICON_WEATHER_CLOUD,   // 2 overcast
  ICON_WEATHER_RAIN,    // 3 rain
  ICON_WEATHER_STORM,   // 4 thunderstorm
  ICON_WEATHER_SNOW,    // 5 snow
  ICON_WEATHER_FOG,     // 6 fog
  ICON_WEATHER_CLOUD    // 7 default
};

// ─── Slot Render Data Struct ─────────────────────────────────────────────────

typedef struct {
  SlotType    type;
  int         value_normalized;  // 0-100 for arc fill
  char        value_str[8];
  char        unit_str[8];
  const char *icon_glyph;
  GColor      icon_color;
  bool        icon_filled;       // true=Filled font, false=Regular font
} SlotRenderData;

// ─── Settings Struct ─────────────────────────────────────────────────────────

typedef struct {
  bool        use_mmol;
  int32_t     high_thresh;
  int32_t     low_thresh;
  int32_t     urgent_high;
  int32_t     urgent_low;
  WatchLayout layout;
  SlotType    slots[4];  // 0=TL/Left, 1=TR/Center, 2=BL/Right, 3=BR (Simple only)
} WatchSettings;

static WatchSettings s_settings = {
  .use_mmol   = false,
  .high_thresh = 180,
  .low_thresh  = 70,
  .urgent_high = 250,
  .urgent_low  = 55,
  .layout      = LAYOUT_SIMPLE,
  .slots       = { SLOT_WEATHER, SLOT_BATTERY, SLOT_CGM, SLOT_HEART_RATE }
};

// ─── Graph Buffer ────────────────────────────────────────────────────────────
#define GRAPH_POINTS 37
static uint8_t s_graph_data[GRAPH_POINTS];
static int     s_graph_count = 0;

// ─── Data State ──────────────────────────────────────────────────────────────
static int32_t s_glucose       = 0;
static int32_t s_trend         = TREND_NONE;
static int32_t s_delta         = 0;
static int32_t s_last_read_sec = 0;

// New sensor data
static int8_t  s_weather_temp  = -128;  // INT8_MIN = unavailable sentinel
static uint8_t s_weather_icon  = 7;     // default cloud
static int     s_heart_rate    = 0;
static uint32_t s_step_count   = 0;

// Alert state
static AppTimer *s_alert_timer    = NULL;
static bool      s_alert_dismissed = false;
static time_t    s_dismiss_until   = 0;
static AppTimer *s_flash_timer    = NULL;
static bool      s_flash_on       = false;

// ─── UI Fonts ────────────────────────────────────────────────────────────────
static GFont s_time_font;           // Inter Black 64px (RESOURCE_ID_TIME_DIGITS_64)
static GFont s_symbol_font;         // Material Symbols Filled 16px (RESOURCE_ID_MATERIAL_SYMBOLS_16)
static GFont s_symbol_font_regular; // Material Symbols Regular 16px (RESOURCE_ID_MATERIAL_SYMBOLS_REGULAR_16)
static GFont s_value_font;          // Inter Black 20px (RESOURCE_ID_DATA_VALUE_20)
static GFont s_unit_font;           // Inter Black 10px (RESOURCE_ID_DATA_UNIT_10)

// ─── UI Layers ───────────────────────────────────────────────────────────────
static Window    *s_main_window;
static Layer     *s_window_layer;

// Shared / always present
static Layer     *s_graph_layer;
static TextLayer *s_stale_layer;

// Widget slots (4 max; slot[3] hidden in Dashboard)
static Layer     *s_slot_layer[4];

// Simple layout specific
static TextLayer *s_simple_digit[4];   // H1, H2, M1, M2
static Layer     *s_simple_digit_stroke[4]; // black stroke layers behind H1, H2, M1, M2
static TextLayer *s_simple_day_layer;
static TextLayer *s_simple_month_layer;
static TextLayer *s_simple_bt_layer;
static TextLayer *s_simple_music_layer;

// Dashboard layout specific
static TextLayer *s_dash_time_layer;
static TextLayer *s_dash_day_layer;
static TextLayer *s_dash_month_layer;
static TextLayer *s_dash_bt_layer;
static TextLayer *s_dash_trend_layer;   // trend icon (Material Symbol)
static TextLayer *s_dash_glucose_layer; // glucose value
static TextLayer *s_dash_unit_layer;    // unit label

// ─── Helpers: Zone ───────────────────────────────────────────────────────────

static GlucoseZone get_zone(int32_t glucose_mgdl) {
  if (glucose_mgdl == 0)                         return ZONE_UNKNOWN;
  if (glucose_mgdl < s_settings.urgent_low)      return ZONE_URGENT_LOW;
  if (glucose_mgdl < s_settings.low_thresh)      return ZONE_LOW;
  if (glucose_mgdl <= s_settings.high_thresh)    return ZONE_IN_RANGE;
  if (glucose_mgdl <= s_settings.urgent_high)    return ZONE_HIGH;
  return ZONE_URGENT_HIGH;
}

static GColor zone_color(GlucoseZone zone) {
  switch (zone) {
    case ZONE_URGENT_LOW:  return CLR_STATE_DANGER;
    case ZONE_LOW:         return CLR_STATE_WARNING;
    case ZONE_IN_RANGE:    return CLR_STATE_POSITIVE;
    case ZONE_HIGH:        return CLR_STATE_WARNING;
    case ZONE_URGENT_HIGH: return CLR_STATE_DANGER;
    default:               return CLR_STATE_INACTIVE;
  }
}

// ─── Helpers: Staleness ──────────────────────────────────────────────────────

static int32_t minutes_since_last_read(void) {
  if (s_last_read_sec == 0) return 9999;
  return (int32_t)((time(NULL) - s_last_read_sec) / 60);
}

static bool data_is_stale(void) {
  return minutes_since_last_read() > 15;
}

// ─── Helpers: Value Formatting ───────────────────────────────────────────────

static void format_glucose(char *buf, size_t len, int32_t mgdl) {
  if (mgdl == 0) { snprintf(buf, len, "--"); return; }
  if (s_settings.use_mmol) {
    int whole = (int)(mgdl * 555 / 10000);
    int frac  = (int)((mgdl * 555 % 10000) / 1000);
    snprintf(buf, len, "%d.%d", whole, frac);
  } else {
    snprintf(buf, len, "%ld", (long)mgdl);
  }
}

static const char* trend_icon(GlucoseTrend t) {
  switch (t) {
    case TREND_DOUBLE_UP:       return ICON_TREND_DOUBLE_UP;
    case TREND_SINGLE_UP:       return ICON_TREND_SINGLE_UP;
    case TREND_FORTY_FIVE_UP:   return ICON_TREND_45_UP;
    case TREND_FLAT:            return ICON_TREND_FLAT;
    case TREND_FORTY_FIVE_DOWN: return ICON_TREND_45_DOWN;
    case TREND_SINGLE_DOWN:     return ICON_TREND_SINGLE_DOWN;
    case TREND_DOUBLE_DOWN:     return ICON_TREND_DOUBLE_DOWN;
    default:                    return NULL;
  }
}

// ─── Graph Layer Draw Proc ───────────────────────────────────────────────────

static void graph_layer_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  int w = bounds.size.w;
  int h = bounds.size.h;

  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  if (s_graph_count < 2) return;

  int min_val = 400, max_val = 40;
  for (int i = 0; i < s_graph_count; i++) {
    int v = (int)s_graph_data[i] * 2;
    if (v > 0) {
      if (v < min_val) min_val = v;
      if (v > max_val) max_val = v;
    }
  }
  if (min_val > 300) min_val = 300;
  if (max_val < 60)  max_val = 60;
  min_val -= 20; max_val += 20;
  if (min_val < 30)  min_val = 30;
  if (max_val > 420) max_val = 420;
  int range = max_val - min_val;
  if (range < 1) range = 1;

  #define VAL_TO_Y(v) (h - 1 - ((v - min_val) * (h - 1) / range))

  if (s_settings.low_thresh >= min_val && s_settings.low_thresh <= max_val) {
    int ly = VAL_TO_Y(s_settings.low_thresh);
    graphics_context_set_stroke_color(ctx, GColorOrange);
    graphics_context_set_stroke_width(ctx, 1);
    for (int x = 0; x < w; x += 4) { graphics_draw_pixel(ctx, GPoint(x, ly)); graphics_draw_pixel(ctx, GPoint(x+1, ly)); }
  }

  if (s_settings.high_thresh >= min_val && s_settings.high_thresh <= max_val) {
    int hy = VAL_TO_Y(s_settings.high_thresh);
    graphics_context_set_stroke_color(ctx, GColorChromeYellow);
    for (int x = 0; x < w; x += 4) { graphics_draw_pixel(ctx, GPoint(x, hy)); graphics_draw_pixel(ctx, GPoint(x+1, hy)); }
  }

  int x_step = (w - 4) / (GRAPH_POINTS - 1);
  if (x_step < 1) x_step = 1;
  GPoint prev_pt = GPoint(-1, -1);

  for (int i = 0; i < s_graph_count; i++) {
    int v = (int)s_graph_data[i] * 2;
    if (v < 30) { prev_pt = GPoint(-1, -1); continue; }

    int x = 2 + i * x_step;
    int y = VAL_TO_Y(v);
    GPoint pt = GPoint(x, y);
    GColor seg_color = zone_color(get_zone((int32_t)v));

    if (prev_pt.x >= 0) {
      graphics_context_set_stroke_color(ctx, seg_color);
      graphics_context_set_stroke_width(ctx, 2);
      graphics_draw_line(ctx, prev_pt, pt);
    }
    prev_pt = pt;

    if (i == s_graph_count - 1) {
      graphics_context_set_fill_color(ctx, seg_color);
      graphics_fill_circle(ctx, pt, 4);
      graphics_context_set_stroke_color(ctx, GColorWhite);
      graphics_context_set_stroke_width(ctx, 1);
      graphics_draw_circle(ctx, pt, 4);
    }
  }
  #undef VAL_TO_Y
}

// ─── Hour Digit Stroke ───────────────────────────────────────────────────────
// Each hour stroke layer covers the same area as its TextLayer, expanded 4px
// on all sides so the stroke doesn't get clipped at the layer boundary.
// The text is drawn at inset (4,4) within the layer; stroke passes add ±4px.

typedef struct {
  char digit;
  GTextAlignment align;
} DigitStrokeData;

static void digit_stroke_update_proc(Layer *layer, GContext *ctx) {
  DigitStrokeData *d = (DigitStrokeData *)layer_get_data(layer);
  if (!d || !d->digit || !s_time_font) return;
  char buf[2] = { d->digit, '\0' };
  GRect b = layer_get_bounds(layer);
  int tw = b.size.w - 8;  // text rect width inside the 4px expansion
  int th = b.size.h - 8;  // text rect height
  static const GPoint offs[] = {
    {-4, 0}, {4, 0}, {0, -4}, {0, 4},
    {-4,-4}, {4,-4}, {-4, 4}, {4, 4}
  };
  graphics_context_set_text_color(ctx, GColorBlack);
  for (int j = 0; j < 8; j++) {
    GRect r = GRect(4 + offs[j].x, 4 + offs[j].y, tw, th);
    graphics_draw_text(ctx, buf, s_time_font, r,
                       GTextOverflowModeWordWrap, d->align, NULL);
  }
}

// ─── Widget Slot System ──────────────────────────────────────────────────────

static void prv_populate_slot_data(SlotRenderData *d, SlotType type) {
  d->type = type;

  switch (type) {
    case SLOT_BATTERY: {
      BatteryChargeState bat = battery_state_service_peek();
      int pct = bat.charge_percent;
      d->value_normalized = pct;
      snprintf(d->value_str, sizeof(d->value_str), "%d", pct);
      snprintf(d->unit_str,  sizeof(d->unit_str),  "%%");
      d->icon_filled = true;
      if (bat.is_charging) {
        d->icon_glyph = ICON_BATTERY_ANDROID_BOLT;
        d->icon_color = CLR_STATE_POSITIVE;
      } else if (pct <= 10) {
        d->icon_glyph = ICON_BATTERY_ANDROID_ALERT;
        d->icon_color = CLR_STATE_DANGER;
      } else if (pct <= 25) {
        d->icon_glyph = ICON_BATTERY_ANDROID_1;
        d->icon_color = CLR_STATE_WARNING;
      } else if (pct <= 50) {
        d->icon_glyph = ICON_BATTERY_ANDROID_2;
        d->icon_color = CLR_ICON_DEFAULT;
      } else if (pct <= 75) {
        d->icon_glyph = ICON_BATTERY_ANDROID_3;
        d->icon_color = CLR_ICON_DEFAULT;
      } else {
        d->icon_glyph = ICON_BATTERY_FULL;
        d->icon_color = CLR_STATE_POSITIVE;
      }
      break;
    }
    case SLOT_WEATHER: {
      bool avail = (s_weather_temp != -128);
      d->value_normalized = 0;  // no arc for weather
      if (avail) snprintf(d->value_str, sizeof(d->value_str), "%d", (int)s_weather_temp);
      else       snprintf(d->value_str, sizeof(d->value_str), "--");
      snprintf(d->unit_str, sizeof(d->unit_str), "\xc2\xb0" "C");  // °C in UTF-8
      uint8_t icon_idx = s_weather_icon < 8 ? s_weather_icon : 7;
      d->icon_glyph = s_weather_icons[icon_idx];
      d->icon_filled = avail;
      d->icon_color = avail ? CLR_ICON_DEFAULT : CLR_STATE_INACTIVE;
      break;
    }
    case SLOT_HEART_RATE: {
      int hr = s_heart_rate;
      int norm = (hr > 40) ? ((hr - 40) * 100 / 160) : 0;
      if (norm > 100) norm = 100;
      d->value_normalized = norm;
      if (hr > 0) snprintf(d->value_str, sizeof(d->value_str), "%d", hr);
      else        snprintf(d->value_str, sizeof(d->value_str), "--");
      snprintf(d->unit_str, sizeof(d->unit_str), "bpm");
      if (hr == 0) {
        d->icon_glyph = ICON_PULSE_ALERT;
        d->icon_filled = false;
        d->icon_color = CLR_STATE_INACTIVE;
      } else if (hr > 160) {
        d->icon_glyph = ICON_CARDIOLOGY;
        d->icon_filled = true;
        d->icon_color = CLR_ICON_DEFAULT;
      } else {
        d->icon_glyph = ICON_HEART_RATE;
        d->icon_filled = true;
        d->icon_color = CLR_ICON_SUBTLE;
      }
      break;
    }
    case SLOT_STEPS: {
      uint32_t steps = s_step_count;
      int norm = (int)(steps * 100 / 10000);
      if (norm > 100) norm = 100;
      d->value_normalized = norm;
      if (steps >= 1000)
        snprintf(d->value_str, sizeof(d->value_str), "%ldk", (unsigned long)(steps / 1000));
      else
        snprintf(d->value_str, sizeof(d->value_str), "%lu", (unsigned long)steps);
      snprintf(d->unit_str, sizeof(d->unit_str), "steps");
      d->icon_glyph = ICON_STEPS;
      d->icon_filled = (steps > 0);
      d->icon_color = (steps > 0) ? CLR_ICON_SUBTLE : CLR_STATE_INACTIVE;
      break;
    }
    case SLOT_CGM: {
      bool stale = data_is_stale();
      GlucoseZone zone = get_zone(s_glucose);
      int norm = (s_glucose > 0) ? (int)(s_glucose * 100 / 300) : 0;
      if (norm > 100) norm = 100;
      d->value_normalized = norm;
      if (stale || s_glucose == 0)
        snprintf(d->value_str, sizeof(d->value_str), "--");
      else
        format_glucose(d->value_str, sizeof(d->value_str), s_glucose);
      snprintf(d->unit_str, sizeof(d->unit_str), s_settings.use_mmol ? "mmol" : "mg/dL");
      if (stale || s_glucose == 0) {
        d->icon_glyph = ICON_ERROR;
        d->icon_filled = false;
        d->icon_color = CLR_STATE_INACTIVE;
      } else {
        d->icon_glyph = trend_icon((GlucoseTrend)s_trend);
        d->icon_filled = true;
        d->icon_color = zone_color(zone);
      }
      break;
    }
    default:  // SLOT_NONE
      d->value_normalized = 0;
      d->value_str[0] = '\0';
      d->unit_str[0]  = '\0';
      d->icon_glyph   = NULL;
      d->icon_color   = GColorClear;
      d->icon_filled  = false;
      break;
  }
}

static void slot_update_proc(Layer *layer, GContext *ctx) {
  SlotRenderData *d = (SlotRenderData *)layer_get_data(layer);
  if (!d || d->type == SLOT_NONE) return;

  GRect bounds = layer_get_bounds(layer);
  int w = bounds.size.w;
  int h = bounds.size.h;
  // Arc drawn in a 48×44 rect, bottom-aligned with 4px side margin and 10px top offset
  // Matches Figma: h-[44px] w-[48px] bottom-aligned in p-[2px] 56×56 container
  GRect arc_bounds = GRect(4, 10, w - 8, h - 14);

  // Battery ring: gap at top (30°→330°). All others: gap at bottom (210°→510°).
  bool is_battery = (d->type == SLOT_BATTERY);
  int arc_start = is_battery ? 30  : 210;
  int arc_end   = is_battery ? 330 : 510;

  // Background track arc — surface/border/subtle (#005555), 2px stroke
  graphics_context_set_stroke_color(ctx, CLR_BORDER_SUBTLE);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_arc(ctx, arc_bounds, GOvalScaleModeFitCircle,
    DEG_TO_TRIGANGLE(arc_start), DEG_TO_TRIGANGLE(arc_end));

  // Active fill arc — icon_color, 2px stroke
  if (d->value_normalized > 0) {
    int fill_angle = arc_start + (d->value_normalized * 300 / 100);
    if (fill_angle > arc_end) fill_angle = arc_end;
    graphics_context_set_stroke_color(ctx, d->icon_color);
    graphics_context_set_stroke_width(ctx, 2);
    graphics_draw_arc(ctx, arc_bounds, GOvalScaleModeFitCircle,
      DEG_TO_TRIGANGLE(arc_start), DEG_TO_TRIGANGLE(fill_angle));
  }

  // Icon — 16px Material Symbol, top-center (absolute at y=2, 20px tall to prevent clipping)
  // Select Filled or Regular variant based on slot state
  if (d->icon_glyph) {
    GFont icon_font = d->icon_filled ? s_symbol_font : s_symbol_font_regular;
    if (!icon_font) icon_font = s_symbol_font;  // fallback
    if (icon_font) {
      graphics_context_set_text_color(ctx, d->icon_color);
      GRect icon_rect = GRect(0, 2, w, 20);
      graphics_draw_text(ctx, d->icon_glyph, icon_font, icon_rect,
        GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
    }
  }

  // Value — Inter Black 20px, white, centered below icon
  if (d->value_str[0]) {
    GFont vfont = s_value_font ? s_value_font
                               : fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
    graphics_context_set_text_color(ctx, CLR_TEXT_INVERTED);
    GRect val_rect = GRect(0, 20, w, 22);
    graphics_draw_text(ctx, d->value_str, vfont,
      val_rect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  }

  // Unit — Inter Black 10px, text/subtle (#AAFFFF)
  if (d->unit_str[0]) {
    GFont ufont = s_unit_font ? s_unit_font
                              : fonts_get_system_font(FONT_KEY_GOTHIC_14);
    graphics_context_set_text_color(ctx, CLR_TEXT_SUBTLE);
    GRect unit_rect = GRect(0, 42, w, 12);
    graphics_draw_text(ctx, d->unit_str, ufont,
      unit_rect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  }
}

static void prv_update_all_slots(void) {
  int count = (s_settings.layout == LAYOUT_SIMPLE) ? 4 : 3;
  for (int i = 0; i < 4; i++) {
    if (!s_slot_layer[i]) continue;
    if (i >= count) {
      layer_set_hidden(s_slot_layer[i], true);
      continue;
    }
    layer_set_hidden(s_slot_layer[i], false);
    SlotRenderData *d = (SlotRenderData *)layer_get_data(s_slot_layer[i]);
    if (d) prv_populate_slot_data(d, s_settings.slots[i]);
    layer_mark_dirty(s_slot_layer[i]);
  }
}

// ─── Update Display ──────────────────────────────────────────────────────────

static void update_display_simple(void) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);

  // 4 separate digit layers
  static char dig[4][4];  // single char + null
  snprintf(dig[0], sizeof(dig[0]), "%c", '0' + (t->tm_hour / 10));
  snprintf(dig[1], sizeof(dig[1]), "%c", '0' + (t->tm_hour % 10));
  snprintf(dig[2], sizeof(dig[2]), "%c", '0' + (t->tm_min  / 10));
  snprintf(dig[3], sizeof(dig[3]), "%c", '0' + (t->tm_min  % 10));

  if (s_simple_digit[0]) text_layer_set_text(s_simple_digit[0], dig[0]);
  if (s_simple_digit[1]) text_layer_set_text(s_simple_digit[1], dig[1]);
  if (s_simple_digit[2]) text_layer_set_text(s_simple_digit[2], dig[2]);
  if (s_simple_digit[3]) text_layer_set_text(s_simple_digit[3], dig[3]);

  // Update stroke layers with new digit characters
  for (int i = 0; i < 4; i++) {
    if (s_simple_digit_stroke[i]) {
      DigitStrokeData *d = (DigitStrokeData *)layer_get_data(s_simple_digit_stroke[i]);
      if (d) d->digit = dig[i][0];
      layer_mark_dirty(s_simple_digit_stroke[i]);
    }
  }

  // Day of month (left side)
  static char s_day_buf[4];
  snprintf(s_day_buf, sizeof(s_day_buf), "%d", t->tm_mday);
  if (s_simple_day_layer)   text_layer_set_text(s_simple_day_layer,   s_day_buf);

  // Month number (right side)
  static char s_month_buf[4];
  snprintf(s_month_buf, sizeof(s_month_buf), "%d", t->tm_mon + 1);
  if (s_simple_month_layer) text_layer_set_text(s_simple_month_layer, s_month_buf);

  // BT icon: Filled when connected, Regular when disconnected
  bool connected = connection_service_peek_pebble_app_connection();
  if (s_simple_bt_layer) {
    text_layer_set_text(s_simple_bt_layer,
      connected ? ICON_BT_CONNECTED : ICON_BT_DISCONNECTED);
    text_layer_set_text_color(s_simple_bt_layer,
      connected ? CLR_ICON_DEFAULT : CLR_STATE_DISABLED);
    GFont bt_font = connected ? s_symbol_font : s_symbol_font_regular;
    if (bt_font) text_layer_set_font(s_simple_bt_layer, bt_font);
  }
}

static void update_display_dashboard(void) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);

  // Single-row time "HH:MM"
  static char s_time_buf[8];
  strftime(s_time_buf, sizeof(s_time_buf),
    clock_is_24h_style() ? "%H:%M" : "%I:%M", t);
  if (s_dash_time_layer) text_layer_set_text(s_dash_time_layer, s_time_buf);

  // Day / month
  static char s_dd_buf[4], s_mm_buf[4];
  snprintf(s_dd_buf, sizeof(s_dd_buf), "%d",       t->tm_mday);
  snprintf(s_mm_buf, sizeof(s_mm_buf), "%d",       t->tm_mon + 1);
  if (s_dash_day_layer)   text_layer_set_text(s_dash_day_layer,   s_dd_buf);
  if (s_dash_month_layer) text_layer_set_text(s_dash_month_layer, s_mm_buf);

  // BT: Filled when connected, Regular when disconnected
  bool connected = connection_service_peek_pebble_app_connection();
  if (s_dash_bt_layer) {
    text_layer_set_text(s_dash_bt_layer,
      connected ? ICON_BT_CONNECTED : ICON_BT_DISCONNECTED);
    text_layer_set_text_color(s_dash_bt_layer,
      connected ? CLR_ICON_DEFAULT : CLR_STATE_DISABLED);
    GFont bt_font = connected ? s_symbol_font : s_symbol_font_regular;
    if (bt_font) text_layer_set_font(s_dash_bt_layer, bt_font);
  }

  // CGM panel
  GlucoseZone zone = get_zone(s_glucose);
  bool stale = data_is_stale();
  GColor cgm_color = stale ? GColorLightGray : zone_color(zone);

  if (s_dash_trend_layer) {
    const char *t_icon = trend_icon((GlucoseTrend)s_trend);
    text_layer_set_text(s_dash_trend_layer, t_icon ? t_icon : "");
    text_layer_set_text_color(s_dash_trend_layer, cgm_color);
  }

  static char s_glu_buf[12];
  if (stale || s_glucose == 0) snprintf(s_glu_buf, sizeof(s_glu_buf), "--");
  else format_glucose(s_glu_buf, sizeof(s_glu_buf), s_glucose);
  if (s_dash_glucose_layer) {
    text_layer_set_text(s_dash_glucose_layer, s_glu_buf);
    text_layer_set_text_color(s_dash_glucose_layer, cgm_color);
  }

  if (s_dash_unit_layer) {
    text_layer_set_text(s_dash_unit_layer,
      s_settings.use_mmol ? "mmol/L" : "mg/dL");
  }
}

static void update_display(void) {
  if (!s_main_window) return;

  if (s_settings.layout == LAYOUT_SIMPLE) {
    update_display_simple();
  } else {
    update_display_dashboard();
  }

  prv_update_all_slots();

  // Update stale row (shared)
  static char s_stale_buf[32];
  int32_t mins = minutes_since_last_read();
  if (mins > 9000)       snprintf(s_stale_buf, sizeof(s_stale_buf), "Waiting for data...");
  else if (mins < 2)     snprintf(s_stale_buf, sizeof(s_stale_buf), "Just updated");
  else                   snprintf(s_stale_buf, sizeof(s_stale_buf), "%ld min ago", (long)mins);
  if (s_stale_layer) {
    text_layer_set_text(s_stale_layer, s_stale_buf);
    text_layer_set_text_color(s_stale_layer,
      data_is_stale() ? GColorRed : GColorDarkGray);
  }

  if (s_graph_layer && s_settings.layout == LAYOUT_DASHBOARD) {
    layer_mark_dirty(s_graph_layer);
  }
}

static void update_time(void) {
  update_display();
}

// ─── Alert Flash System ──────────────────────────────────────────────────────

static void flash_timer_callback(void *context) {
  GlucoseZone zone = get_zone(s_glucose);
  bool dismissed = s_alert_dismissed && time(NULL) < s_dismiss_until;
  bool urgent = !dismissed && (zone == ZONE_URGENT_LOW || zone == ZONE_URGENT_HIGH);

  if (!urgent) {
    s_flash_timer = NULL;
    window_set_background_color(s_main_window, GColorBlack);
    update_display();
    return;
  }

  s_flash_on = !s_flash_on;
  window_set_background_color(s_main_window,
    s_flash_on ? zone_color(zone) : GColorBlack);
  if (!s_flash_on) update_display();

  s_flash_timer = app_timer_register(700, flash_timer_callback, NULL);
}

static void start_alert_flash(void) {
  if (s_flash_timer) return;
  s_flash_on = false;
  s_flash_timer = app_timer_register(100, flash_timer_callback, NULL);
  static const uint32_t segs[] = { 300, 200, 300, 200, 300 };
  VibePattern pat = { .durations = segs, .num_segments = 5 };
  vibes_enqueue_custom_pattern(pat);
}

static void stop_alert_flash(void) {
  if (s_flash_timer) { app_timer_cancel(s_flash_timer); s_flash_timer = NULL; }
  s_flash_on = false;
  window_set_background_color(s_main_window, GColorBlack);
  update_display();
}

static void dismiss_alert(void) {
  s_alert_dismissed = true;
  s_dismiss_until   = time(NULL) + (15 * 60);
  stop_alert_flash();
}

// ─── Tick Handler ────────────────────────────────────────────────────────────

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();

  if (s_alert_dismissed && time(NULL) >= s_dismiss_until) {
    s_alert_dismissed = false;
  }

  GlucoseZone zone    = get_zone(s_glucose);
  bool dismissed      = s_alert_dismissed && time(NULL) < s_dismiss_until;
  bool urgent         = !dismissed && (zone == ZONE_URGENT_LOW || zone == ZONE_URGENT_HIGH);

  if (urgent && !s_flash_timer) start_alert_flash();
  else if (!urgent && s_flash_timer) stop_alert_flash();
}

// Forward declaration needed by inbox_received_handler
void prv_layout_for_bounds(GRect bounds);

// ─── AppMessage Handler ──────────────────────────────────────────────────────

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  Tuple *t;

  t = dict_find(iter, KEY_GLUCOSE_VALUE);  if (t) s_glucose        = t->value->int32;
  t = dict_find(iter, KEY_GLUCOSE_TREND);  if (t) s_trend          = t->value->int32;
  t = dict_find(iter, KEY_GLUCOSE_DELTA);  if (t) s_delta          = t->value->int32;
  t = dict_find(iter, KEY_LAST_READ_SEC);  if (t) s_last_read_sec  = t->value->int32;
  t = dict_find(iter, KEY_USE_MMOL);       if (t) s_settings.use_mmol    = (bool)t->value->int32;
  t = dict_find(iter, KEY_HIGH_THRESHOLD); if (t) s_settings.high_thresh = t->value->int32;
  t = dict_find(iter, KEY_LOW_THRESHOLD);  if (t) s_settings.low_thresh  = t->value->int32;
  t = dict_find(iter, KEY_URGENT_HIGH);    if (t) s_settings.urgent_high = t->value->int32;
  t = dict_find(iter, KEY_URGENT_LOW);     if (t) s_settings.urgent_low  = t->value->int32;

  t = dict_find(iter, KEY_GRAPH_DATA);
  if (t && t->length <= GRAPH_POINTS) {
    s_graph_count = (int)t->length;
    memcpy(s_graph_data, t->value->data, s_graph_count);
  }

  t = dict_find(iter, KEY_WEATHER_TEMP);
  if (t) s_weather_temp = (int8_t)t->value->int32;

  t = dict_find(iter, KEY_WEATHER_ICON);
  if (t) s_weather_icon = (uint8_t)t->value->int32;

  t = dict_find(iter, KEY_LAYOUT);
  if (t) s_settings.layout = (WatchLayout)t->value->int32;

  t = dict_find(iter, KEY_SLOT_0); if (t) s_settings.slots[0] = (SlotType)t->value->int32;
  t = dict_find(iter, KEY_SLOT_1); if (t) s_settings.slots[1] = (SlotType)t->value->int32;
  t = dict_find(iter, KEY_SLOT_2); if (t) s_settings.slots[2] = (SlotType)t->value->int32;
  t = dict_find(iter, KEY_SLOT_3); if (t) s_settings.slots[3] = (SlotType)t->value->int32;

  // Persist
  persist_write_int(PERSIST_GLUCOSE,     s_glucose);
  persist_write_int(PERSIST_TREND,       s_trend);
  persist_write_int(PERSIST_DELTA,       s_delta);
  persist_write_int(PERSIST_LAST_READ,   (int32_t)s_last_read_sec);
  persist_write_int(PERSIST_USE_MMOL,    s_settings.use_mmol ? 1 : 0);
  persist_write_int(PERSIST_HIGH_THRESH, s_settings.high_thresh);
  persist_write_int(PERSIST_LOW_THRESH,  s_settings.low_thresh);
  persist_write_int(PERSIST_URG_HIGH,    s_settings.urgent_high);
  persist_write_int(PERSIST_URG_LOW,     s_settings.urgent_low);
  persist_write_int(PERSIST_LAYOUT,      (int32_t)s_settings.layout);
  persist_write_int(PERSIST_SLOT_0,      (int32_t)s_settings.slots[0]);
  persist_write_int(PERSIST_SLOT_1,      (int32_t)s_settings.slots[1]);
  persist_write_int(PERSIST_SLOT_2,      (int32_t)s_settings.slots[2]);
  persist_write_int(PERSIST_SLOT_3,      (int32_t)s_settings.slots[3]);
  persist_write_int(PERSIST_WEATHER_TMP, (int32_t)s_weather_temp);
  persist_write_int(PERSIST_WEATHER_ICN, (int32_t)s_weather_icon);

  // Reset dismiss when data fresh and in-range
  if (!data_is_stale() && get_zone(s_glucose) == ZONE_IN_RANGE) {
    s_alert_dismissed = false;
  }

  // Reposition layers in case layout changed
  if (s_window_layer) {
    GRect avail = layer_get_unobstructed_bounds(s_window_layer);
    prv_layout_for_bounds(avail);
  }

  update_display();

  GlucoseZone zone = get_zone(s_glucose);
  bool dismissed   = s_alert_dismissed && time(NULL) < s_dismiss_until;
  bool urgent      = !dismissed && (zone == ZONE_URGENT_LOW || zone == ZONE_URGENT_HIGH);

  if (urgent) {
    start_alert_flash();
  } else {
    stop_alert_flash();
    if (zone == ZONE_LOW || zone == ZONE_HIGH) {
      static const uint32_t segs[] = { 200, 100, 200 };
      VibePattern pat = { .durations = segs, .num_segments = 3 };
      vibes_enqueue_custom_pattern(pat);
    }
  }
}

static void inbox_dropped_handler(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "AppMessage dropped: %d", (int)reason);
}

// ─── Button Handler ──────────────────────────────────────────────────────────

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  GlucoseZone zone = get_zone(s_glucose);
  if (zone == ZONE_URGENT_LOW || zone == ZONE_URGENT_HIGH) dismiss_alert();
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

// ─── Pebble Health Callback ──────────────────────────────────────────────────

static void prv_health_handler(HealthEventType event, void *context) {
  if (event == HealthEventHeartRateUpdate || event == HealthEventSignificantUpdate) {
    HealthValue hr = health_service_peek_current_value(HealthMetricHeartRateBPM);
    if (hr > 0) s_heart_rate = (int)hr;
  }
  if (event == HealthEventMovementUpdate || event == HealthEventSignificantUpdate) {
    HealthValue steps = health_service_peek_current_value(HealthMetricStepCount);
    if (steps >= 0) s_step_count = (uint32_t)steps;
  }
  update_display();
}

// ─── Bluetooth Callback ──────────────────────────────────────────────────────

static void bluetooth_callback(bool connected) {
  update_display();
  if (!connected) {
    static const uint32_t segs[] = { 200, 100, 200 };
    VibePattern pat = { .durations = segs, .num_segments = 3 };
    vibes_enqueue_custom_pattern(pat);
  }
}

// ─── Layout Positioning ──────────────────────────────────────────────────────
//
// T2 (emery): 200x228  R2 (gabbro): 260x260
// Called on window_load, unobstructed change, and layout setting change.

void prv_layout_for_bounds(GRect bounds) {
  int w = bounds.size.w;
  int h = bounds.size.h;

  bool is_r2     = (w == 260);
  bool compact   = (h < 185);  // Quick View active
  bool simple    = (s_settings.layout == LAYOUT_SIMPLE);
  bool dashboard = !simple;

  // ── Hide/show layout-specific layers ──────────────────────────────────────
  bool show_simple    = simple;
  bool show_dashboard = dashboard;

  for (int i = 0; i < 4; i++) {
    if (s_simple_digit[i])
      layer_set_hidden(text_layer_get_layer(s_simple_digit[i]), !show_simple || compact);
  }
  for (int i = 0; i < 4; i++) {
    if (s_simple_digit_stroke[i])
      layer_set_hidden(s_simple_digit_stroke[i], !show_simple || compact);
  }
  if (s_simple_day_layer)   layer_set_hidden(text_layer_get_layer(s_simple_day_layer),   !show_simple);
  if (s_simple_month_layer) layer_set_hidden(text_layer_get_layer(s_simple_month_layer), !show_simple);
  if (s_simple_bt_layer)    layer_set_hidden(text_layer_get_layer(s_simple_bt_layer),    !show_simple);
  if (s_simple_music_layer) layer_set_hidden(text_layer_get_layer(s_simple_music_layer), true);  // always hidden

  if (s_dash_time_layer)    layer_set_hidden(text_layer_get_layer(s_dash_time_layer),    !show_dashboard);
  if (s_dash_day_layer)     layer_set_hidden(text_layer_get_layer(s_dash_day_layer),     !show_dashboard);
  if (s_dash_month_layer)   layer_set_hidden(text_layer_get_layer(s_dash_month_layer),   !show_dashboard);
  if (s_dash_bt_layer)      layer_set_hidden(text_layer_get_layer(s_dash_bt_layer),      !show_dashboard);
  if (s_dash_trend_layer)   layer_set_hidden(text_layer_get_layer(s_dash_trend_layer),   !show_dashboard);
  if (s_dash_glucose_layer) layer_set_hidden(text_layer_get_layer(s_dash_glucose_layer), !show_dashboard);
  if (s_dash_unit_layer)    layer_set_hidden(text_layer_get_layer(s_dash_unit_layer),    !show_dashboard);

  // Graph: visible in Dashboard (non-compact), hidden in Simple and compact Dashboard
  bool show_graph = (dashboard && !compact);
  if (s_graph_layer) layer_set_hidden(s_graph_layer, !show_graph);

  // Stale row: always visible
  if (s_stale_layer) layer_set_hidden(text_layer_get_layer(s_stale_layer), false);

  // ── Simple layout positions ───────────────────────────────────────────────
  if (simple) {
    if (!is_r2) {
      // T2: 200x228
      // Slot positions (corners)
      GRect slot_frames[4] = {
        GRect(4,   4, 56, 56),    // TL
        GRect(140, 4, 56, 56),    // TR
        GRect(4,   168, 56, 56),  // BL
        GRect(140, 168, 56, 56)   // BR
      };
      for (int i = 0; i < 4; i++) {
        if (s_slot_layer[i]) {
          layer_set_frame(s_slot_layer[i], slot_frames[i]);
          layer_set_hidden(s_slot_layer[i], compact && i >= 2);
        }
      }

      // Digit layers: 2-row time centered on screen (200x228, center=100,114).
      // Box 48x70 (tight to glyph width, safe for widest digits). H overlap -2, V overlap -28.
      // Horizontally centered, vertically top-anchored → shared baseline at equal Y.
      // Group: 94x112px, top-left (53, 58).
      // Z-order: stroke+fill interleaved per digit (H1, H2, M1, M2). Minute row on top.
      if (!compact) {
        GRect digit_frames[4] = {
          GRect(53, 58,  48, 70),  // H1
          GRect(99, 58,  48, 70),  // H2
          GRect(53, 100, 48, 70),  // M1
          GRect(99, 100, 48, 70)   // M2
        };
        for (int i = 0; i < 4; i++) {
          if (s_simple_digit[i]) {
            layer_set_frame(text_layer_get_layer(s_simple_digit[i]), digit_frames[i]);
          }
        }
        // Stroke layers: fill frame expanded ±4px (56x78).
        if (s_simple_digit_stroke[0])
          layer_set_frame(s_simple_digit_stroke[0], GRect(49, 54, 56, 78));  // H1
        if (s_simple_digit_stroke[1])
          layer_set_frame(s_simple_digit_stroke[1], GRect(95, 54, 56, 78));  // H2
        if (s_simple_digit_stroke[2])
          layer_set_frame(s_simple_digit_stroke[2], GRect(49, 96, 56, 78));  // M1
        if (s_simple_digit_stroke[3])
          layer_set_frame(s_simple_digit_stroke[3], GRect(95, 96, 56, 78));  // M2
      }

      // BT icon: top center
      if (s_simple_bt_layer)
        layer_set_frame(text_layer_get_layer(s_simple_bt_layer), GRect(92, 4, 16, 16));

      // Day/Month: vertical center flanking time
      if (s_simple_day_layer)
        layer_set_frame(text_layer_get_layer(s_simple_day_layer), GRect(4, 106, 18, 16));
      if (s_simple_month_layer)
        layer_set_frame(text_layer_get_layer(s_simple_month_layer), GRect(178, 106, 18, 16));

      // Music: bottom center
      if (s_simple_music_layer)
        layer_set_frame(text_layer_get_layer(s_simple_music_layer), GRect(92, 208, 16, 16));

    } else {
      // R2: 260x260 inscribed circle
      GRect slot_frames[4] = {
        GRect(38,  38,  56, 56),  // TL
        GRect(166, 38,  56, 56),  // TR
        GRect(38,  166, 56, 56),  // BL
        GRect(166, 166, 56, 56)   // BR
      };
      for (int i = 0; i < 4; i++) {
        if (s_slot_layer[i]) {
          layer_set_frame(s_slot_layer[i], slot_frames[i]);
          layer_set_hidden(s_slot_layer[i], compact && i >= 2);
        }
      }

      if (!compact) {
        // Box 48x70. H overlap -2, V overlap -28. Horizontally centered,
        // vertically top-anchored (shared baseline across all 4 digits).
        // Group: 94x112px centered at (130,130), top-left (83, 74).
        GRect digit_frames[4] = {
          GRect(83,  74,  48, 70),  // H1
          GRect(129, 74,  48, 70),  // H2
          GRect(83,  116, 48, 70),  // M1
          GRect(129, 116, 48, 70)   // M2
        };
        for (int i = 0; i < 4; i++) {
          if (s_simple_digit[i])
            layer_set_frame(text_layer_get_layer(s_simple_digit[i]), digit_frames[i]);
        }
        if (s_simple_digit_stroke[0])
          layer_set_frame(s_simple_digit_stroke[0], GRect(79,  70,  56, 78));  // H1
        if (s_simple_digit_stroke[1])
          layer_set_frame(s_simple_digit_stroke[1], GRect(125, 70,  56, 78));  // H2
        if (s_simple_digit_stroke[2])
          layer_set_frame(s_simple_digit_stroke[2], GRect(79,  112, 56, 78));  // M1
        if (s_simple_digit_stroke[3])
          layer_set_frame(s_simple_digit_stroke[3], GRect(125, 112, 56, 78));  // M2
      }

      if (s_simple_bt_layer)
        layer_set_frame(text_layer_get_layer(s_simple_bt_layer), GRect(122, 14, 16, 16));
      if (s_simple_day_layer)
        layer_set_frame(text_layer_get_layer(s_simple_day_layer), GRect(8, 128, 24, 16));
      if (s_simple_month_layer)
        layer_set_frame(text_layer_get_layer(s_simple_month_layer), GRect(228, 128, 24, 16));
    }

    // Stale row (Simple)
    if (s_stale_layer)
      layer_set_frame(text_layer_get_layer(s_stale_layer),
        GRect(4, h - 18, w - 8, 16));
  }

  // ── Dashboard layout positions ────────────────────────────────────────────
  if (dashboard) {
    if (!is_r2) {
      // T2: 200x228

      // Top 3 slots
      GRect slot_frames[3] = {
        GRect(4,   4, 56, 56),   // Left
        GRect(72,  4, 56, 56),   // Center
        GRect(140, 4, 56, 56)    // Right
      };
      for (int i = 0; i < 3; i++) {
        if (s_slot_layer[i]) {
          layer_set_frame(s_slot_layer[i], slot_frames[i]);
          layer_set_hidden(s_slot_layer[i], compact);
        }
      }
      if (s_slot_layer[3]) layer_set_hidden(s_slot_layer[3], true);

      // Time row (between top slots and CGM panel)
      // Top slots bottom: y=60; CGM panel top: y=168
      // Center time in 60..168: midpoint=114, LECO_38 height ~44px → y=92
      if (s_dash_time_layer)
        layer_set_frame(text_layer_get_layer(s_dash_time_layer), GRect(24, 76, 144, 44));
      if (s_dash_bt_layer)
        layer_set_frame(text_layer_get_layer(s_dash_bt_layer), GRect(4, 80, 16, 16));
      if (s_dash_day_layer)
        layer_set_frame(text_layer_get_layer(s_dash_day_layer), GRect(176, 74, 20, 14));
      if (s_dash_month_layer)
        layer_set_frame(text_layer_get_layer(s_dash_month_layer), GRect(176, 94, 20, 14));

      // CGM panel: y=130..224
      // Graph: 120px wide
      if (s_graph_layer)
        layer_set_frame(s_graph_layer, GRect(4, 130, 120, 80));
      // Trend icon
      if (s_dash_trend_layer)
        layer_set_frame(text_layer_get_layer(s_dash_trend_layer), GRect(128, 130, 68, 20));
      // Glucose value
      if (s_dash_glucose_layer)
        layer_set_frame(text_layer_get_layer(s_dash_glucose_layer), GRect(128, 152, 68, 30));
      // Unit
      if (s_dash_unit_layer)
        layer_set_frame(text_layer_get_layer(s_dash_unit_layer), GRect(128, 182, 68, 14));
      // Stale
      if (s_stale_layer)
        layer_set_frame(text_layer_get_layer(s_stale_layer), GRect(4, 212, 192, 14));

    } else {
      // R2: 260x260

      GRect slot_frames[3] = {
        GRect(20,  20, 56, 56),
        GRect(102, 14, 56, 56),
        GRect(184, 20, 56, 56)
      };
      for (int i = 0; i < 3; i++) {
        if (s_slot_layer[i]) {
          layer_set_frame(s_slot_layer[i], slot_frames[i]);
          layer_set_hidden(s_slot_layer[i], compact);
        }
      }
      if (s_slot_layer[3]) layer_set_hidden(s_slot_layer[3], true);

      if (s_dash_time_layer)
        layer_set_frame(text_layer_get_layer(s_dash_time_layer), GRect(50, 90, 160, 44));
      if (s_dash_bt_layer)
        layer_set_frame(text_layer_get_layer(s_dash_bt_layer), GRect(10, 94, 16, 16));
      if (s_dash_day_layer)
        layer_set_frame(text_layer_get_layer(s_dash_day_layer), GRect(234, 88, 20, 14));
      if (s_dash_month_layer)
        layer_set_frame(text_layer_get_layer(s_dash_month_layer), GRect(234, 108, 20, 14));

      if (s_graph_layer)
        layer_set_frame(s_graph_layer, GRect(30, 148, 150, 76));
      if (s_dash_trend_layer)
        layer_set_frame(text_layer_get_layer(s_dash_trend_layer), GRect(188, 148, 42, 20));
      if (s_dash_glucose_layer)
        layer_set_frame(text_layer_get_layer(s_dash_glucose_layer), GRect(188, 170, 42, 30));
      if (s_dash_unit_layer)
        layer_set_frame(text_layer_get_layer(s_dash_unit_layer), GRect(188, 200, 42, 14));
      if (s_stale_layer)
        layer_set_frame(text_layer_get_layer(s_stale_layer), GRect(30, 228, 200, 14));
    }
  }

  layer_mark_dirty(s_window_layer);
}

// ─── Unobstructed Area Handlers ──────────────────────────────────────────────

static void prv_unobstructed_change(AnimationProgress progress, void *context) {
  GRect bounds = layer_get_unobstructed_bounds(s_window_layer);
  prv_layout_for_bounds(bounds);
}

static void prv_unobstructed_did_change(void *context) {
  GRect bounds = layer_get_unobstructed_bounds(s_window_layer);
  prv_layout_for_bounds(bounds);
}

// ─── Window Load / Unload ────────────────────────────────────────────────────

static void main_window_load(Window *window) {
  s_window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(s_window_layer);
  int w = bounds.size.w;
  int h = bounds.size.h;

  window_set_background_color(window, GColorBlack);

  // Load custom fonts
  s_time_font           = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_TIME_DIGITS_64));
  s_symbol_font         = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_MATERIAL_SYMBOLS_16));
  s_symbol_font_regular = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_MATERIAL_SYMBOLS_REGULAR_16));
  s_value_font          = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_DATA_VALUE_20));
  s_unit_font           = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_DATA_UNIT_10));

  // ── Simple: per-digit stroke + fill pairs ──
  // Z-order: each digit's stroke + fill is added as a unit, so H2 stacks on top of H1,
  // M1 on top of H2, etc. That way outlines are never covered by a neighbor's fill.
  // Colors per Figma: H1=text/subtle, H2=text/inverted, M1=text/inverted, M2=text/default.
  // H1/M1 right-aligned, H2/M2 left-aligned.
  GColor digit_colors[4] = {
    CLR_TEXT_SUBTLE,    // H1 — #AAFFFF text/subtle
    CLR_TEXT_INVERTED,  // H2 — #FFFFFF text/inverted
    CLR_TEXT_INVERTED,  // M1 — #FFFFFF text/inverted
    CLR_TEXT_DEFAULT    // M2 — #00FFFF text/default
  };
  GTextAlignment digit_aligns[4] = {
    GTextAlignmentCenter,  // H1
    GTextAlignmentCenter,  // H2
    GTextAlignmentCenter,  // M1
    GTextAlignmentCenter   // M2
  };
  for (int i = 0; i < 4; i++) {
    // Stroke layer (outline) — added first so fill renders on top of own outline.
    s_simple_digit_stroke[i] = layer_create_with_data(GRect(0, 0, 56, 78), sizeof(DigitStrokeData));
    DigitStrokeData *d = (DigitStrokeData *)layer_get_data(s_simple_digit_stroke[i]);
    if (d) { d->digit = '0'; d->align = digit_aligns[i]; }
    layer_set_update_proc(s_simple_digit_stroke[i], digit_stroke_update_proc);
    layer_add_child(s_window_layer, s_simple_digit_stroke[i]);

    // Fill text layer — added right after its stroke, before the next digit's stroke.
    s_simple_digit[i] = text_layer_create(GRect(0, 0, 48, 70));
    text_layer_set_background_color(s_simple_digit[i], GColorClear);
    text_layer_set_text_color(s_simple_digit[i], digit_colors[i]);
    if (s_time_font) text_layer_set_font(s_simple_digit[i], s_time_font);
    text_layer_set_text_alignment(s_simple_digit[i], digit_aligns[i]);
    text_layer_set_text(s_simple_digit[i], "0");
    layer_add_child(s_window_layer, text_layer_get_layer(s_simple_digit[i]));
  }

  // ── Widget slot layers (added after digits so they render on top) ──
  for (int i = 0; i < 4; i++) {
    s_slot_layer[i] = layer_create_with_data(GRect(0, 0, 56, 56), sizeof(SlotRenderData));
    layer_set_update_proc(s_slot_layer[i], slot_update_proc);
    layer_add_child(s_window_layer, s_slot_layer[i]);
  }

  // ── Simple: status + date ──
  s_simple_bt_layer = text_layer_create(GRect(92, 4, 16, 16));
  text_layer_set_background_color(s_simple_bt_layer, GColorClear);
  text_layer_set_text_color(s_simple_bt_layer, CLR_ICON_DEFAULT);
  if (s_symbol_font) text_layer_set_font(s_simple_bt_layer, s_symbol_font);
  text_layer_set_text_alignment(s_simple_bt_layer, GTextAlignmentCenter);
  layer_add_child(s_window_layer, text_layer_get_layer(s_simple_bt_layer));

  s_simple_music_layer = text_layer_create(GRect(92, 208, 16, 16));
  text_layer_set_background_color(s_simple_music_layer, GColorClear);
  text_layer_set_text_color(s_simple_music_layer, CLR_ICON_SUBTLE);
  if (s_symbol_font) text_layer_set_font(s_simple_music_layer, s_symbol_font);
  text_layer_set_text_alignment(s_simple_music_layer, GTextAlignmentCenter);
  text_layer_set_text(s_simple_music_layer, ICON_MUSIC);
  layer_add_child(s_window_layer, text_layer_get_layer(s_simple_music_layer));

  s_simple_day_layer = text_layer_create(GRect(4, 106, 18, 16));
  text_layer_set_background_color(s_simple_day_layer, GColorClear);
  text_layer_set_text_color(s_simple_day_layer, CLR_TEXT_SUBTLE);
  text_layer_set_font(s_simple_day_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_simple_day_layer, GTextAlignmentRight);
  layer_add_child(s_window_layer, text_layer_get_layer(s_simple_day_layer));

  s_simple_month_layer = text_layer_create(GRect(178, 106, 18, 16));
  text_layer_set_background_color(s_simple_month_layer, GColorClear);
  text_layer_set_text_color(s_simple_month_layer, CLR_TEXT_SUBTLE);
  text_layer_set_font(s_simple_month_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_simple_month_layer, GTextAlignmentLeft);
  layer_add_child(s_window_layer, text_layer_get_layer(s_simple_month_layer));

  // ── Dashboard: time row ──
  s_dash_time_layer = text_layer_create(GRect(24, 76, 144, 44));
  text_layer_set_background_color(s_dash_time_layer, GColorClear);
  text_layer_set_text_color(s_dash_time_layer, GColorWhite);
  text_layer_set_font(s_dash_time_layer, fonts_get_system_font(FONT_KEY_LECO_36_BOLD_NUMBERS));
  text_layer_set_text_alignment(s_dash_time_layer, GTextAlignmentCenter);
  text_layer_set_text(s_dash_time_layer, "00:00");
  layer_add_child(s_window_layer, text_layer_get_layer(s_dash_time_layer));

  s_dash_bt_layer = text_layer_create(GRect(4, 80, 16, 16));
  text_layer_set_background_color(s_dash_bt_layer, GColorClear);
  text_layer_set_text_color(s_dash_bt_layer, CLR_ICON_DEFAULT);
  if (s_symbol_font) text_layer_set_font(s_dash_bt_layer, s_symbol_font);
  text_layer_set_text_alignment(s_dash_bt_layer, GTextAlignmentCenter);
  layer_add_child(s_window_layer, text_layer_get_layer(s_dash_bt_layer));

  s_dash_day_layer = text_layer_create(GRect(176, 74, 20, 14));
  text_layer_set_background_color(s_dash_day_layer, GColorClear);
  text_layer_set_text_color(s_dash_day_layer, CLR_TEXT_SUBTLE);
  text_layer_set_font(s_dash_day_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_dash_day_layer, GTextAlignmentRight);
  layer_add_child(s_window_layer, text_layer_get_layer(s_dash_day_layer));

  s_dash_month_layer = text_layer_create(GRect(176, 94, 20, 14));
  text_layer_set_background_color(s_dash_month_layer, GColorClear);
  text_layer_set_text_color(s_dash_month_layer, CLR_TEXT_SUBTLE);
  text_layer_set_font(s_dash_month_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_dash_month_layer, GTextAlignmentRight);
  layer_add_child(s_window_layer, text_layer_get_layer(s_dash_month_layer));

  // ── Dashboard: CGM panel ──
  s_dash_trend_layer = text_layer_create(GRect(128, 130, 68, 20));
  text_layer_set_background_color(s_dash_trend_layer, GColorClear);
  text_layer_set_text_color(s_dash_trend_layer, GColorLightGray);
  if (s_symbol_font) text_layer_set_font(s_dash_trend_layer, s_symbol_font);
  text_layer_set_text_alignment(s_dash_trend_layer, GTextAlignmentRight);
  layer_add_child(s_window_layer, text_layer_get_layer(s_dash_trend_layer));

  s_dash_glucose_layer = text_layer_create(GRect(128, 152, 68, 30));
  text_layer_set_background_color(s_dash_glucose_layer, GColorClear);
  text_layer_set_text_color(s_dash_glucose_layer, GColorMintGreen);
  text_layer_set_font(s_dash_glucose_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_dash_glucose_layer, GTextAlignmentRight);
  text_layer_set_text(s_dash_glucose_layer, "--");
  layer_add_child(s_window_layer, text_layer_get_layer(s_dash_glucose_layer));

  s_dash_unit_layer = text_layer_create(GRect(128, 182, 68, 14));
  text_layer_set_background_color(s_dash_unit_layer, GColorClear);
  text_layer_set_text_color(s_dash_unit_layer, GColorMediumAquamarine);
  text_layer_set_font(s_dash_unit_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_dash_unit_layer, GTextAlignmentRight);
  layer_add_child(s_window_layer, text_layer_get_layer(s_dash_unit_layer));

  // ── Graph layer (dashboard only) ──
  s_graph_layer = layer_create(GRect(4, 130, 120, 80));
  layer_set_update_proc(s_graph_layer, graph_layer_update_proc);
  layer_add_child(s_window_layer, s_graph_layer);

  // ── Stale row (shared) ──
  s_stale_layer = text_layer_create(GRect(4, h - 18, w - 8, 16));
  text_layer_set_background_color(s_stale_layer, GColorClear);
  text_layer_set_text_color(s_stale_layer, GColorDarkGray);
  text_layer_set_font(s_stale_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_stale_layer, GTextAlignmentCenter);
  text_layer_set_text(s_stale_layer, "Waiting for data...");
  layer_add_child(s_window_layer, text_layer_get_layer(s_stale_layer));

  // ── Subscribe to UnobstructedArea ──
  UnobstructedAreaHandlers ua = {
    .change     = prv_unobstructed_change,
    .did_change = prv_unobstructed_did_change
  };
  unobstructed_area_service_subscribe(ua, NULL);

  // ── Subscribe to Health ──
  health_service_events_subscribe(prv_health_handler, NULL);

  // Apply initial layout
  GRect avail = layer_get_unobstructed_bounds(s_window_layer);
  prv_layout_for_bounds(avail);
}

static void main_window_unload(Window *window) {
  // Slot layers
  for (int i = 0; i < 4; i++) {
    if (s_slot_layer[i]) { layer_destroy(s_slot_layer[i]); s_slot_layer[i] = NULL; }
  }

  // Simple layers
  for (int i = 0; i < 4; i++) {
    if (s_simple_digit_stroke[i]) { layer_destroy(s_simple_digit_stroke[i]); s_simple_digit_stroke[i] = NULL; }
  }
  for (int i = 0; i < 4; i++) {
    if (s_simple_digit[i]) { text_layer_destroy(s_simple_digit[i]); s_simple_digit[i] = NULL; }
  }
  if (s_simple_bt_layer)    { text_layer_destroy(s_simple_bt_layer);    s_simple_bt_layer    = NULL; }
  if (s_simple_music_layer) { text_layer_destroy(s_simple_music_layer); s_simple_music_layer = NULL; }
  if (s_simple_day_layer)   { text_layer_destroy(s_simple_day_layer);   s_simple_day_layer   = NULL; }
  if (s_simple_month_layer) { text_layer_destroy(s_simple_month_layer); s_simple_month_layer = NULL; }

  // Dashboard layers
  if (s_dash_time_layer)    { text_layer_destroy(s_dash_time_layer);    s_dash_time_layer    = NULL; }
  if (s_dash_bt_layer)      { text_layer_destroy(s_dash_bt_layer);      s_dash_bt_layer      = NULL; }
  if (s_dash_day_layer)     { text_layer_destroy(s_dash_day_layer);     s_dash_day_layer     = NULL; }
  if (s_dash_month_layer)   { text_layer_destroy(s_dash_month_layer);   s_dash_month_layer   = NULL; }
  if (s_dash_trend_layer)   { text_layer_destroy(s_dash_trend_layer);   s_dash_trend_layer   = NULL; }
  if (s_dash_glucose_layer) { text_layer_destroy(s_dash_glucose_layer); s_dash_glucose_layer = NULL; }
  if (s_dash_unit_layer)    { text_layer_destroy(s_dash_unit_layer);    s_dash_unit_layer    = NULL; }

  // Shared
  if (s_graph_layer) { layer_destroy(s_graph_layer); s_graph_layer = NULL; }
  if (s_stale_layer) { text_layer_destroy(s_stale_layer); s_stale_layer = NULL; }

  // Custom fonts
  if (s_time_font)   { fonts_unload_custom_font(s_time_font);   s_time_font   = NULL; }
  if (s_symbol_font)         { fonts_unload_custom_font(s_symbol_font);         s_symbol_font         = NULL; }
  if (s_symbol_font_regular) { fonts_unload_custom_font(s_symbol_font_regular); s_symbol_font_regular = NULL; }
  if (s_value_font)  { fonts_unload_custom_font(s_value_font);  s_value_font  = NULL; }
  if (s_unit_font)   { fonts_unload_custom_font(s_unit_font);   s_unit_font   = NULL; }

  health_service_events_unsubscribe();
  unobstructed_area_service_unsubscribe();
}

// ─── Init / Deinit ───────────────────────────────────────────────────────────

static void init(void) {
  // Restore persisted state
  if (persist_exists(PERSIST_GLUCOSE))     s_glucose               = persist_read_int(PERSIST_GLUCOSE);
  if (persist_exists(PERSIST_TREND))       s_trend                 = persist_read_int(PERSIST_TREND);
  if (persist_exists(PERSIST_DELTA))       s_delta                 = persist_read_int(PERSIST_DELTA);
  if (persist_exists(PERSIST_LAST_READ))   s_last_read_sec         = (time_t)persist_read_int(PERSIST_LAST_READ);
  if (persist_exists(PERSIST_USE_MMOL))    s_settings.use_mmol     = persist_read_int(PERSIST_USE_MMOL) != 0;
  if (persist_exists(PERSIST_HIGH_THRESH)) s_settings.high_thresh  = persist_read_int(PERSIST_HIGH_THRESH);
  if (persist_exists(PERSIST_LOW_THRESH))  s_settings.low_thresh   = persist_read_int(PERSIST_LOW_THRESH);
  if (persist_exists(PERSIST_URG_HIGH))    s_settings.urgent_high  = persist_read_int(PERSIST_URG_HIGH);
  if (persist_exists(PERSIST_URG_LOW))     s_settings.urgent_low   = persist_read_int(PERSIST_URG_LOW);
  if (persist_exists(PERSIST_LAYOUT))      s_settings.layout       = (WatchLayout)persist_read_int(PERSIST_LAYOUT);
  if (persist_exists(PERSIST_SLOT_0))      s_settings.slots[0]     = (SlotType)persist_read_int(PERSIST_SLOT_0);
  if (persist_exists(PERSIST_SLOT_1))      s_settings.slots[1]     = (SlotType)persist_read_int(PERSIST_SLOT_1);
  if (persist_exists(PERSIST_SLOT_2))      s_settings.slots[2]     = (SlotType)persist_read_int(PERSIST_SLOT_2);
  if (persist_exists(PERSIST_SLOT_3))      s_settings.slots[3]     = (SlotType)persist_read_int(PERSIST_SLOT_3);
  if (persist_exists(PERSIST_WEATHER_TMP)) s_weather_temp          = (int8_t)persist_read_int(PERSIST_WEATHER_TMP);
  if (persist_exists(PERSIST_WEATHER_ICN)) s_weather_icon          = (uint8_t)persist_read_int(PERSIST_WEATHER_ICN);

#ifdef DEMO_DATA
  // Override with demo values for visual testing of all widget modules
  s_glucose       = 120;
  s_trend         = TREND_FLAT;
  s_delta         = 2;
  s_last_read_sec = time(NULL) - 180;  // 3 min ago (not stale)
  s_weather_temp  = 10;
  s_weather_icon  = 0;  // sunny
  s_heart_rate    = 128;
  s_step_count    = 6842;

  s_settings.use_mmol    = false;
  s_settings.high_thresh = 180;
  s_settings.low_thresh  = 70;
  s_settings.urgent_high = 250;
  s_settings.urgent_low  = 55;
  s_settings.layout      = LAYOUT_SIMPLE;
  s_settings.slots[0]    = SLOT_WEATHER;     // TL
  s_settings.slots[1]    = SLOT_BATTERY;     // TR
  s_settings.slots[2]    = SLOT_CGM;         // BL
  s_settings.slots[3]    = SLOT_HEART_RATE;  // BR

  // Demo graph data (gentle wave around 120-160 mg/dL, stored as value/2)
  s_graph_count = GRAPH_POINTS;
  for (int i = 0; i < GRAPH_POINTS; i++) {
    s_graph_data[i] = (uint8_t)(60 + (i % 7) * 5);  // 120-150 mg/dL range
  }
#endif

  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
  window_set_window_handlers(s_main_window, (WindowHandlers){
    .load   = main_window_load,
    .unload = main_window_unload
  });
  window_set_click_config_provider(s_main_window, click_config_provider);
  window_stack_push(s_main_window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  update_time();

  connection_service_subscribe((ConnectionHandlers){
    .pebble_app_connection_handler = bluetooth_callback
  });

  app_message_register_inbox_received(inbox_received_handler);
  app_message_register_inbox_dropped(inbox_dropped_handler);
  app_message_open(512, 128);
}

static void deinit(void) {
  if (s_flash_timer) app_timer_cancel(s_flash_timer);
  if (s_alert_timer) app_timer_cancel(s_alert_timer);
  tick_timer_service_unsubscribe();
  connection_service_unsubscribe();
  app_message_deregister_callbacks();
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
  return 0;
}
