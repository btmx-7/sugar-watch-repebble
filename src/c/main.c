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

// ─── Material Symbols UTF-8 Glyph Constants (task 0.10) ──────────────────────
// Each constant is the UTF-8 byte sequence for the Unicode codepoint.

// Trend arrows
#define ICON_TREND_DOUBLE_UP    "\xee\x97\x98\xee\x97\x98"  // U+E5D8 north x2
#define ICON_TREND_SINGLE_UP    "\xee\x97\x98"               // U+E5D8 north
#define ICON_TREND_45_UP        "\xee\xa0\x9a"               // U+E81A trending_up
#define ICON_TREND_FLAT         "\xee\x97\x88"               // U+E5C8 arrow_forward
#define ICON_TREND_45_DOWN      "\xee\x97\x85"               // U+E5C5
#define ICON_TREND_SINGLE_DOWN  "\xee\x97\x9b"               // U+E5DB south
#define ICON_TREND_DOUBLE_DOWN  "\xee\x97\x9b\xee\x97\x9b"  // U+E5DB south x2
#define ICON_TREND_NONE         "-"

// Status icons
#define ICON_BT_CONNECTED       "\xee\x96\x9a"   // U+E59A bluetooth_connected
#define ICON_BT_DISCONNECTED    "\xee\xaf\x9c"   // U+EBDC bluetooth_disabled
#define ICON_MUSIC              "\xee\x90\x85"   // U+E405 music_note

// Slot type icons
#define ICON_HEART_RATE         "\xee\xa1\xbd"   // U+E87D favorite (heart)
#define ICON_STEPS              "\xee\x94\xb6"   // U+E536 directions_walk
#define ICON_BATTERY            "\xee\xa7\xa4"   // U+E9E4 electric_bolt
#define ICON_BATTERY_FULL       "\xef\xa2\x98"   // U+F898 battery_full

// Weather icons (indices 0-7 match weatherCodeToIconIndex in pkjs)
#define ICON_WEATHER_SUNNY      "\xef\x85\xb2"   // U+F172 clear_day
#define ICON_WEATHER_PARTLY     "\xef\x9b\x84"   // U+F6C4 partly_cloudy_day
#define ICON_WEATHER_CLOUD      "\xee\x8b\xbd"   // U+E2BD cloud
#define ICON_WEATHER_RAIN       "\xef\x98\x9f"   // U+F61F rainy
#define ICON_WEATHER_STORM      "\xee\xa0\x98"   // U+E818 thunderstorm
#define ICON_WEATHER_SNOW       "\xee\x80\xa8"   // U+E028 ac_unit
#define ICON_WEATHER_FOG        "\xee\x9b\xa1"   // U+E6E1 foggy

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
static GFont s_time_font;    // Inter Black 64px (RESOURCE_ID_TIME_DIGITS_64)
static GFont s_symbol_font;  // Material Symbols 16px (RESOURCE_ID_MATERIAL_SYMBOLS_16)

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
    case ZONE_URGENT_LOW:  return GColorRed;
    case ZONE_LOW:         return GColorOrange;
    case ZONE_IN_RANGE:    return GColorMintGreen;
    case ZONE_HIGH:        return GColorChromeYellow;
    case ZONE_URGENT_HIGH: return GColorRed;
    default:               return GColorLightGray;
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
    default:                    return ICON_TREND_NONE;
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
      d->icon_glyph  = ICON_BATTERY;
      d->icon_color  = pct <= 20 ? GColorRed : (pct <= 50 ? GColorChromeYellow : GColorCyan);
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
      d->icon_color = GColorCyan;
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
      d->icon_glyph = ICON_HEART_RATE;
      d->icon_color = GColorCyan;
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
      d->icon_color = GColorCyan;
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
      d->icon_glyph = trend_icon((GlucoseTrend)s_trend);
      d->icon_color = stale ? GColorLightGray : zone_color(zone);
      break;
    }
    default:  // SLOT_NONE
      d->value_normalized = 0;
      d->value_str[0] = '\0';
      d->unit_str[0]  = '\0';
      d->icon_glyph   = NULL;
      d->icon_color   = GColorClear;
      break;
  }
}

static void slot_update_proc(Layer *layer, GContext *ctx) {
  SlotRenderData *d = (SlotRenderData *)layer_get_data(layer);
  if (!d || d->type == SLOT_NONE) return;

  GRect bounds = layer_get_bounds(layer);
  int w = bounds.size.w;
  int h = bounds.size.h;
  GRect arc_bounds = GRect(2, 2, w - 4, h - 4);

  // Battery ring: gap at top (30°→330°). All others: gap at bottom (210°→510°).
  bool is_battery = (d->type == SLOT_BATTERY);
  int arc_start = is_battery ? 30  : 210;
  int arc_end   = is_battery ? 330 : 510;

  // Background track arc (300° sweep)
  graphics_context_set_stroke_color(ctx, GColorDarkGray);
  graphics_context_set_stroke_width(ctx, 3);
  graphics_draw_arc(ctx, arc_bounds, GOvalScaleModeFitCircle,
    DEG_TO_TRIGANGLE(arc_start), DEG_TO_TRIGANGLE(arc_end));

  // Active fill arc
  if (d->value_normalized > 0) {
    int fill_angle = arc_start + (d->value_normalized * 300 / 100);
    if (fill_angle > arc_end) fill_angle = arc_end;
    graphics_context_set_stroke_color(ctx, GColorCyan);
    graphics_draw_arc(ctx, arc_bounds, GOvalScaleModeFitCircle,
      DEG_TO_TRIGANGLE(arc_start), DEG_TO_TRIGANGLE(fill_angle));
  }

  // Icon (top center, 16px)
  if (d->icon_glyph && s_symbol_font) {
    graphics_context_set_text_color(ctx, d->icon_color);
    GRect icon_rect = GRect(0, 4, w, 18);
    graphics_draw_text(ctx, d->icon_glyph, s_symbol_font, icon_rect,
      GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  }

  // Value text (center)
  if (d->value_str[0]) {
    graphics_context_set_text_color(ctx, GColorWhite);
    GRect val_rect = GRect(0, 20, w, 22);
    graphics_draw_text(ctx, d->value_str,
      fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
      val_rect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  }

  // Unit text (below value)
  if (d->unit_str[0]) {
    graphics_context_set_text_color(ctx, GColorMediumAquamarine);
    GRect unit_rect = GRect(0, 40, w, 14);
    graphics_draw_text(ctx, d->unit_str,
      fonts_get_system_font(FONT_KEY_GOTHIC_14),
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

  // Day of month (left side)
  static char s_day_buf[4];
  snprintf(s_day_buf, sizeof(s_day_buf), "%d", t->tm_mday);
  if (s_simple_day_layer)   text_layer_set_text(s_simple_day_layer,   s_day_buf);

  // Month number (right side)
  static char s_month_buf[4];
  snprintf(s_month_buf, sizeof(s_month_buf), "%d", t->tm_mon + 1);
  if (s_simple_month_layer) text_layer_set_text(s_simple_month_layer, s_month_buf);

  // BT icon
  bool connected = connection_service_peek_pebble_app_connection();
  if (s_simple_bt_layer) {
    text_layer_set_text(s_simple_bt_layer,
      connected ? ICON_BT_CONNECTED : ICON_BT_DISCONNECTED);
    text_layer_set_text_color(s_simple_bt_layer,
      connected ? GColorCyan : GColorRed);
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

  // BT
  bool connected = connection_service_peek_pebble_app_connection();
  if (s_dash_bt_layer) {
    text_layer_set_text(s_dash_bt_layer,
      connected ? ICON_BT_CONNECTED : ICON_BT_DISCONNECTED);
    text_layer_set_text_color(s_dash_bt_layer,
      connected ? GColorCyan : GColorRed);
  }

  // CGM panel
  GlucoseZone zone = get_zone(s_glucose);
  bool stale = data_is_stale();
  GColor cgm_color = stale ? GColorLightGray : zone_color(zone);

  if (s_dash_trend_layer) {
    text_layer_set_text(s_dash_trend_layer, trend_icon((GlucoseTrend)s_trend));
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

      // Digit layers: 2-row time centered on screen (228px tall).
      // 64px font needs 70px box to avoid glyph clipping.
      // Row1 top at y=46, row2 at y=112 → pair centered at y=114.
      if (!compact) {
        GRect digit_frames[4] = {
          GRect(28,  46, 68, 70),   // H1
          GRect(100, 46, 68, 70),   // H2
          GRect(28,  112, 68, 70),  // M1
          GRect(100, 112, 68, 70)   // M2
        };
        for (int i = 0; i < 4; i++) {
          if (s_simple_digit[i]) {
            layer_set_frame(text_layer_get_layer(s_simple_digit[i]), digit_frames[i]);
          }
        }
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
        GRect digit_frames[4] = {
          GRect(62,  80, 68, 70),   // H1
          GRect(130, 80, 68, 70),   // H2
          GRect(62,  146, 68, 70),  // M1
          GRect(130, 146, 68, 70)   // M2
        };
        for (int i = 0; i < 4; i++) {
          if (s_simple_digit[i])
            layer_set_frame(text_layer_get_layer(s_simple_digit[i]), digit_frames[i]);
        }
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
  s_time_font   = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_TIME_DIGITS_64));
  s_symbol_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_MATERIAL_SYMBOLS_16));

  // ── Simple: 4 digit TextLayers (added first so slots render on top) ──
  GColor digit_colors[4] = {
    GColorMediumAquamarine,  // H1
    GColorWhite,             // H2
    GColorWhite,             // M1
    GColorCyan               // M2
  };
  for (int i = 0; i < 4; i++) {
    s_simple_digit[i] = text_layer_create(GRect(0, 0, 68, 70));
    text_layer_set_background_color(s_simple_digit[i], GColorClear);
    text_layer_set_text_color(s_simple_digit[i], digit_colors[i]);
    if (s_time_font) text_layer_set_font(s_simple_digit[i], s_time_font);
    text_layer_set_text_alignment(s_simple_digit[i], GTextAlignmentCenter);
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
  text_layer_set_text_color(s_simple_bt_layer, GColorCyan);
  if (s_symbol_font) text_layer_set_font(s_simple_bt_layer, s_symbol_font);
  text_layer_set_text_alignment(s_simple_bt_layer, GTextAlignmentCenter);
  layer_add_child(s_window_layer, text_layer_get_layer(s_simple_bt_layer));

  s_simple_music_layer = text_layer_create(GRect(92, 208, 16, 16));
  text_layer_set_background_color(s_simple_music_layer, GColorClear);
  text_layer_set_text_color(s_simple_music_layer, GColorCyan);
  if (s_symbol_font) text_layer_set_font(s_simple_music_layer, s_symbol_font);
  text_layer_set_text_alignment(s_simple_music_layer, GTextAlignmentCenter);
  text_layer_set_text(s_simple_music_layer, ICON_MUSIC);
  layer_add_child(s_window_layer, text_layer_get_layer(s_simple_music_layer));

  s_simple_day_layer = text_layer_create(GRect(4, 106, 18, 16));
  text_layer_set_background_color(s_simple_day_layer, GColorClear);
  text_layer_set_text_color(s_simple_day_layer, GColorMediumAquamarine);
  text_layer_set_font(s_simple_day_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_simple_day_layer, GTextAlignmentRight);
  layer_add_child(s_window_layer, text_layer_get_layer(s_simple_day_layer));

  s_simple_month_layer = text_layer_create(GRect(178, 106, 18, 16));
  text_layer_set_background_color(s_simple_month_layer, GColorClear);
  text_layer_set_text_color(s_simple_month_layer, GColorMediumAquamarine);
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
  text_layer_set_text_color(s_dash_bt_layer, GColorCyan);
  if (s_symbol_font) text_layer_set_font(s_dash_bt_layer, s_symbol_font);
  text_layer_set_text_alignment(s_dash_bt_layer, GTextAlignmentCenter);
  layer_add_child(s_window_layer, text_layer_get_layer(s_dash_bt_layer));

  s_dash_day_layer = text_layer_create(GRect(176, 74, 20, 14));
  text_layer_set_background_color(s_dash_day_layer, GColorClear);
  text_layer_set_text_color(s_dash_day_layer, GColorMediumAquamarine);
  text_layer_set_font(s_dash_day_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_dash_day_layer, GTextAlignmentRight);
  layer_add_child(s_window_layer, text_layer_get_layer(s_dash_day_layer));

  s_dash_month_layer = text_layer_create(GRect(176, 94, 20, 14));
  text_layer_set_background_color(s_dash_month_layer, GColorClear);
  text_layer_set_text_color(s_dash_month_layer, GColorMediumAquamarine);
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
  if (s_symbol_font) { fonts_unload_custom_font(s_symbol_font); s_symbol_font = NULL; }

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
