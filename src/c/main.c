/**
 * Sugar Watch — CGM Watchface for Pebble Time 2 (Emery) & Round 2 (Gabbro)
 * Spring 2026 rePebble Contest Entry
 *
 * Data source: Nightscout-compatible API (or Dexcom Share via phone JS)
 * Features:
 *   - Large color-coded glucose value (green/orange/red by zone)
 *   - Trend arrow + delta
 *   - 3-hour sparkline graph with threshold lines
 *   - Alert flash + haptic for urgent low/high
 *   - Stale data detection
 *   - Quick View / Timeline Peek support (UnobstructedArea API)
 *   - Persistent last-known value across restarts
 *   - Touch/SELECT to dismiss alerts
 *   - 12/24hr time, mg/dL or mmol/L
 */

#include <pebble.h>

// ─── App Message Keys ───────────────────────────────────────────────────────
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

// ─── Trend Enum ──────────────────────────────────────────────────────────────
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

// ─── Graph buffer ────────────────────────────────────────────────────────────
#define GRAPH_POINTS 37
static uint8_t s_graph_data[GRAPH_POINTS];
static int s_graph_count = 0;

// ─── State ───────────────────────────────────────────────────────────────────
static int32_t  s_glucose       = 0;
static int32_t  s_trend         = TREND_NONE;
static int32_t  s_delta         = 0;
static int32_t  s_last_read_sec = 0;  // unix timestamp of last reading
static bool     s_use_mmol      = false;
static int32_t  s_high_thresh   = 180;
static int32_t  s_low_thresh    = 70;
static int32_t  s_urgent_high   = 250;
static int32_t  s_urgent_low    = 55;

// Alert dismiss timer
static AppTimer *s_alert_timer  = NULL;
static bool      s_alert_dismissed = false;
static time_t    s_dismiss_until   = 0;

// Alert flash state
static AppTimer *s_flash_timer  = NULL;
static bool      s_flash_on     = false;

// ─── UI Elements ─────────────────────────────────────────────────────────────
static Window     *s_main_window;
static Layer      *s_window_layer;
static Layer      *s_graph_layer;
static TextLayer  *s_glucose_layer;
static TextLayer  *s_trend_layer;
static TextLayer  *s_delta_layer;
static TextLayer  *s_zone_layer;
static TextLayer  *s_time_layer;
static TextLayer  *s_date_layer;
static TextLayer  *s_stale_layer;
static BitmapLayer *s_bt_icon_layer;
static GBitmap    *s_bt_icon_bitmap;

// ─── Helpers: Zone ───────────────────────────────────────────────────────────

typedef enum {
  ZONE_URGENT_LOW,
  ZONE_LOW,
  ZONE_IN_RANGE,
  ZONE_HIGH,
  ZONE_URGENT_HIGH,
  ZONE_UNKNOWN
} GlucoseZone;

static GlucoseZone get_zone(int32_t glucose_mgdl) {
  if (glucose_mgdl == 0) return ZONE_UNKNOWN;
  if (glucose_mgdl < s_urgent_low)  return ZONE_URGENT_LOW;
  if (glucose_mgdl < s_low_thresh)  return ZONE_LOW;
  if (glucose_mgdl <= s_high_thresh) return ZONE_IN_RANGE;
  if (glucose_mgdl <= s_urgent_high) return ZONE_HIGH;
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

static const char* zone_label(GlucoseZone zone) {
  switch (zone) {
    case ZONE_URGENT_LOW:  return "URGENT LOW";
    case ZONE_LOW:         return "LOW";
    case ZONE_IN_RANGE:    return "IN RANGE";
    case ZONE_HIGH:        return "HIGH";
    case ZONE_URGENT_HIGH: return "URGENT HIGH";
    default:               return "NO DATA";
  }
}

static const char* trend_arrow(GlucoseTrend trend) {
  switch (trend) {
    case TREND_DOUBLE_UP:       return "↑↑";
    case TREND_SINGLE_UP:       return "↑";
    case TREND_FORTY_FIVE_UP:   return "↗";
    case TREND_FLAT:            return "→";
    case TREND_FORTY_FIVE_DOWN: return "↘";
    case TREND_SINGLE_DOWN:     return "↓";
    case TREND_DOUBLE_DOWN:     return "↓↓";
    default:                    return "-";
  }
}

// ─── Helpers: Staleness ──────────────────────────────────────────────────────

static int32_t minutes_since_last_read(void) {
  if (s_last_read_sec == 0) return 9999;
  time_t now = time(NULL);
  return (int32_t)((now - s_last_read_sec) / 60);
}

static bool data_is_stale(void) {
  return minutes_since_last_read() > 15;
}

// ─── Helpers: Value formatting ──────────────────────────────────────────────

static void format_glucose(char *buf, size_t len, int32_t mgdl) {
  if (mgdl == 0) {
    snprintf(buf, len, "--");
    return;
  }
  if (s_use_mmol) {
    // mmol/L = mg/dL * 0.0555, one decimal place
    int whole = (int)(mgdl * 555 / 10000);
    int frac  = (int)((mgdl * 555 % 10000) / 1000);
    snprintf(buf, len, "%d.%d", whole, frac);
  } else {
    snprintf(buf, len, "%ld", (long)mgdl);
  }
}

static void format_delta(char *buf, size_t len, int32_t delta_mgdl) {
  if (s_use_mmol) {
    int32_t abs_delta = delta_mgdl < 0 ? -delta_mgdl : delta_mgdl;
    int whole = (int)(abs_delta * 555 / 10000);
    int frac  = (int)((abs_delta * 555 % 10000) / 1000);
    if (delta_mgdl >= 0)
      snprintf(buf, len, "+%d.%d", whole, frac);
    else
      snprintf(buf, len, "-%d.%d", whole, frac);
  } else {
    if (delta_mgdl >= 0)
      snprintf(buf, len, "+%ld", (long)delta_mgdl);
    else
      snprintf(buf, len, "%ld", (long)delta_mgdl);
  }
}

// ─── Graph Layer Drawing ────────────────────────────────────────────────────

static void graph_layer_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  int w = bounds.size.w;
  int h = bounds.size.h;

  // Background
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  if (s_graph_count < 2) return;

  // Find min/max for adaptive scaling
  int min_val = 400, max_val = 40;
  for (int i = 0; i < s_graph_count; i++) {
    int v = (int)s_graph_data[i] * 2;  // stored as mgdl/2
    if (v > 0) {
      if (v < min_val) min_val = v;
      if (v > max_val) max_val = v;
    }
  }
  // Clamp and add padding
  if (min_val > 300) min_val = 300;
  if (max_val < 60)  max_val = 60;
  min_val = min_val - 20;
  max_val = max_val + 20;
  if (min_val < 30)  min_val = 30;
  if (max_val > 420) max_val = 420;
  int range = max_val - min_val;
  if (range < 1) range = 1;

  // Helper macro: value → y pixel (inverted — high value = low y)
  #define VAL_TO_Y(v) (h - 1 - ((v - min_val) * (h - 1) / range))

  // Draw threshold lines (dashed)
  GColor low_line_color  = GColorOrange;
  GColor high_line_color = GColorChromeYellow;

  if (s_low_thresh >= min_val && s_low_thresh <= max_val) {
    int ly = VAL_TO_Y(s_low_thresh);
    graphics_context_set_stroke_color(ctx, low_line_color);
    graphics_context_set_stroke_width(ctx, 1);
    for (int x = 0; x < w; x += 4) {
      graphics_draw_pixel(ctx, GPoint(x, ly));
      graphics_draw_pixel(ctx, GPoint(x+1, ly));
    }
  }

  if (s_high_thresh >= min_val && s_high_thresh <= max_val) {
    int hy = VAL_TO_Y(s_high_thresh);
    graphics_context_set_stroke_color(ctx, high_line_color);
    for (int x = 0; x < w; x += 4) {
      graphics_draw_pixel(ctx, GPoint(x, hy));
      graphics_draw_pixel(ctx, GPoint(x+1, hy));
    }
  }

  // Draw line segments colored by zone
  int x_step = (w - 4) / (GRAPH_POINTS - 1);
  if (x_step < 1) x_step = 1;

  GPoint prev_pt = GPoint(-1, -1);

  for (int i = 0; i < s_graph_count; i++) {
    int v = (int)s_graph_data[i] * 2;
    if (v < 30) { prev_pt = GPoint(-1, -1); continue; }

    int x = 2 + i * x_step;
    int y = VAL_TO_Y(v);
    GPoint pt = GPoint(x, y);

    GlucoseZone seg_zone = get_zone((int32_t)v);
    GColor seg_color = zone_color(seg_zone);

    if (prev_pt.x >= 0) {
      graphics_context_set_stroke_color(ctx, seg_color);
      graphics_context_set_stroke_width(ctx, 2);
      graphics_draw_line(ctx, prev_pt, pt);
    }

    prev_pt = pt;

    // Draw current value dot (last point)
    if (i == s_graph_count - 1) {
      graphics_context_set_fill_color(ctx, seg_color);
      graphics_fill_circle(ctx, pt, 4);
      // White outline on dot
      graphics_context_set_stroke_color(ctx, GColorWhite);
      graphics_context_set_stroke_width(ctx, 1);
      graphics_draw_circle(ctx, pt, 4);
    }
  }

  #undef VAL_TO_Y
}

// ─── Update UI ───────────────────────────────────────────────────────────────

static void update_display(void) {
  if (!s_main_window) return;

  GlucoseZone zone = get_zone(s_glucose);
  bool stale = data_is_stale();
  bool dismissed = (s_alert_dismissed && time(NULL) < s_dismiss_until);
  bool urgent = (!dismissed) && (zone == ZONE_URGENT_LOW || zone == ZONE_URGENT_HIGH);

  // ── Glucose value ──
  static char s_glucose_buf[16];
  if (stale || s_glucose == 0) {
    snprintf(s_glucose_buf, sizeof(s_glucose_buf), "--");
  } else {
    format_glucose(s_glucose_buf, sizeof(s_glucose_buf), s_glucose);
  }
  text_layer_set_text(s_glucose_layer, s_glucose_buf);

  // Color the glucose text by zone (or gray if stale)
  GColor glucose_color = stale ? GColorLightGray : zone_color(zone);
  text_layer_set_text_color(s_glucose_layer, glucose_color);

  // ── Trend arrow ──
  text_layer_set_text(s_trend_layer, trend_arrow((GlucoseTrend)s_trend));
  text_layer_set_text_color(s_trend_layer, glucose_color);

  // ── Delta ──
  static char s_delta_buf[10];
  if (stale || s_glucose == 0) {
    snprintf(s_delta_buf, sizeof(s_delta_buf), "");
  } else {
    format_delta(s_delta_buf, sizeof(s_delta_buf), s_delta);
  }
  text_layer_set_text(s_delta_layer, s_delta_buf);
  text_layer_set_text_color(s_delta_layer, glucose_color);

  // ── Zone label ──
  if (stale || s_glucose == 0) {
    text_layer_set_text(s_zone_layer, urgent ? "⚠ NO DATA" : "NO DATA");
    text_layer_set_text_color(s_zone_layer, GColorRed);
  } else {
    static char zone_buf[24];
    snprintf(zone_buf, sizeof(zone_buf), "%s%s",
      urgent ? "⚠ " : "", zone_label(zone));
    text_layer_set_text(s_zone_layer, zone_buf);
    text_layer_set_text_color(s_zone_layer, glucose_color);
  }

  // ── Stale / last-read row ──
  static char s_stale_buf[32];
  int32_t mins = minutes_since_last_read();
  if (mins > 9000) {
    snprintf(s_stale_buf, sizeof(s_stale_buf), "Waiting for data...");
  } else if (mins < 2) {
    snprintf(s_stale_buf, sizeof(s_stale_buf), "Just updated");
  } else {
    snprintf(s_stale_buf, sizeof(s_stale_buf), "Last read: %ld min ago", (long)mins);
  }
  text_layer_set_text(s_stale_layer, s_stale_buf);
  text_layer_set_text_color(s_stale_layer, stale ? GColorRed : GColorLightGray);

  // ── Window background (flash alert) ──
  // Handled in flash_timer callback

  // ── Redraw graph ──
  if (s_graph_layer) {
    layer_mark_dirty(s_graph_layer);
  }
}

// ─── Time Display ────────────────────────────────────────────────────────────

static void update_time(void) {
  time_t temp = time(NULL);
  struct tm *t = localtime(&temp);

  static char s_time_buf[8];
  strftime(s_time_buf, sizeof(s_time_buf),
    clock_is_24h_style() ? "%H:%M" : "%I:%M", t);
  text_layer_set_text(s_time_layer, s_time_buf);

  static char s_date_buf[20];
  strftime(s_date_buf, sizeof(s_date_buf), "%a %b %d", t);
  text_layer_set_text(s_date_layer, s_date_buf);

  // Update stale label once per minute too
  update_display();
}

// ─── Alert Flash ─────────────────────────────────────────────────────────────

static void flash_timer_callback(void *context) {
  GlucoseZone zone = get_zone(s_glucose);
  bool dismissed = (s_alert_dismissed && time(NULL) < s_dismiss_until);
  bool urgent = (!dismissed) && (zone == ZONE_URGENT_LOW || zone == ZONE_URGENT_HIGH);

  if (!urgent) {
    s_flash_timer = NULL;
    window_set_background_color(s_main_window, GColorBlack);
    return;
  }

  s_flash_on = !s_flash_on;
  if (s_flash_on) {
    window_set_background_color(s_main_window, zone_color(zone));
    // Invert text colors for readability on colored background
    text_layer_set_text_color(s_glucose_layer, GColorBlack);
    text_layer_set_text_color(s_trend_layer, GColorBlack);
    text_layer_set_text_color(s_zone_layer, GColorBlack);
  } else {
    window_set_background_color(s_main_window, GColorBlack);
    update_display();
  }

  s_flash_timer = app_timer_register(700, flash_timer_callback, NULL);
}

static void start_alert_flash(void) {
  if (s_flash_timer) return;  // already running
  s_flash_on = false;
  s_flash_timer = app_timer_register(100, flash_timer_callback, NULL);

  // Haptic: triple long pulse for urgent
  static const uint32_t segments[] = { 300, 200, 300, 200, 300 };
  VibePattern pat = { .durations = segments, .num_segments = 5 };
  vibes_enqueue_custom_pattern(pat);
}

static void stop_alert_flash(void) {
  if (s_flash_timer) {
    app_timer_cancel(s_flash_timer);
    s_flash_timer = NULL;
  }
  s_flash_on = false;
  window_set_background_color(s_main_window, GColorBlack);
  update_display();
}

// ─── Dismiss Alert ───────────────────────────────────────────────────────────

static void dismiss_alert(void) {
  s_alert_dismissed = true;
  s_dismiss_until = time(NULL) + (15 * 60);  // 15 minutes
  stop_alert_flash();
}

// ─── Tick Handler ────────────────────────────────────────────────────────────

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();

  // Check if dismiss has expired
  if (s_alert_dismissed && time(NULL) >= s_dismiss_until) {
    s_alert_dismissed = false;
  }

  // Manage alert flash based on zone
  GlucoseZone zone = get_zone(s_glucose);
  bool dismissed = (s_alert_dismissed && time(NULL) < s_dismiss_until);
  bool urgent = (!dismissed) && (zone == ZONE_URGENT_LOW || zone == ZONE_URGENT_HIGH);

  if (urgent && !s_flash_timer) {
    start_alert_flash();
  } else if (!urgent && s_flash_timer) {
    stop_alert_flash();
  }
}

// ─── AppMessage ──────────────────────────────────────────────────────────────

static void inbox_received_handler(DictionaryIterator *iterator, void *context) {
  Tuple *t;

  t = dict_find(iterator, KEY_GLUCOSE_VALUE);
  if (t) s_glucose = t->value->int32;

  t = dict_find(iterator, KEY_GLUCOSE_TREND);
  if (t) s_trend = t->value->int32;

  t = dict_find(iterator, KEY_GLUCOSE_DELTA);
  if (t) s_delta = t->value->int32;

  t = dict_find(iterator, KEY_LAST_READ_SEC);
  if (t) s_last_read_sec = t->value->int32;

  t = dict_find(iterator, KEY_USE_MMOL);
  if (t) s_use_mmol = (bool)t->value->int32;

  t = dict_find(iterator, KEY_HIGH_THRESHOLD);
  if (t) s_high_thresh = t->value->int32;

  t = dict_find(iterator, KEY_LOW_THRESHOLD);
  if (t) s_low_thresh = t->value->int32;

  t = dict_find(iterator, KEY_URGENT_HIGH);
  if (t) s_urgent_high = t->value->int32;

  t = dict_find(iterator, KEY_URGENT_LOW);
  if (t) s_urgent_low = t->value->int32;

  t = dict_find(iterator, KEY_GRAPH_DATA);
  if (t && t->length <= GRAPH_POINTS) {
    s_graph_count = (int)t->length;
    memcpy(s_graph_data, t->value->data, s_graph_count);
  }

  // Persist key values for cold start
  persist_write_int(PERSIST_GLUCOSE,     s_glucose);
  persist_write_int(PERSIST_TREND,       s_trend);
  persist_write_int(PERSIST_DELTA,       s_delta);
  persist_write_int(PERSIST_LAST_READ,   (int32_t)s_last_read_sec);
  persist_write_int(PERSIST_USE_MMOL,    s_use_mmol ? 1 : 0);
  persist_write_int(PERSIST_HIGH_THRESH, s_high_thresh);
  persist_write_int(PERSIST_LOW_THRESH,  s_low_thresh);
  persist_write_int(PERSIST_URG_HIGH,    s_urgent_high);
  persist_write_int(PERSIST_URG_LOW,     s_urgent_low);

  // Reset dismiss state when fresh data arrives
  if (!data_is_stale()) {
    // If we got new data and now in range, auto-reset dismiss
    GlucoseZone zone = get_zone(s_glucose);
    if (zone == ZONE_IN_RANGE) s_alert_dismissed = false;
  }

  update_display();

  // Re-evaluate alert state
  GlucoseZone zone = get_zone(s_glucose);
  bool dismissed = (s_alert_dismissed && time(NULL) < s_dismiss_until);
  bool urgent = (!dismissed) && (zone == ZONE_URGENT_LOW || zone == ZONE_URGENT_HIGH);

  if (urgent) {
    start_alert_flash();
  } else {
    stop_alert_flash();
    // Single haptic for non-urgent alerts
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

// ─── Button / Touch ──────────────────────────────────────────────────────────

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  GlucoseZone zone = get_zone(s_glucose);
  if (zone == ZONE_URGENT_LOW || zone == ZONE_URGENT_HIGH) {
    dismiss_alert();
  }
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

// ─── Quick View / UnobstructedArea ──────────────────────────────────────────

static void prv_layout_for_bounds(GRect bounds) {
  int w = bounds.size.w;
  int h = bounds.size.h;

  // ── Positions: adapt to available height ──
  // Status bar: always at top
  // For Emery (200×228): normal layout
  // For Quick View (removes bottom ~51px): compress

  bool compact = (h < 185);  // Quick View active

  // Time: top-right
  layer_set_frame(text_layer_get_layer(s_time_layer),
    GRect(w - 75, 2, 73, 28));

  // Glucose value block: upper area
  int glucose_y = compact ? 22 : 28;
  layer_set_frame(text_layer_get_layer(s_glucose_layer),
    GRect(4, glucose_y, w - 4, 50));

  // Trend + delta: same row as glucose
  layer_set_frame(text_layer_get_layer(s_trend_layer),
    GRect(4, glucose_y + 48, 50, 28));

  layer_set_frame(text_layer_get_layer(s_delta_layer),
    GRect(58, glucose_y + 48, w - 62, 28));

  // Zone label
  layer_set_frame(text_layer_get_layer(s_zone_layer),
    GRect(4, glucose_y + 74, w - 8, 24));

  // Graph
  if (!compact) {
    int graph_y = glucose_y + 100;
    int graph_h = h - graph_y - 40;
    if (graph_h < 30) graph_h = 30;
    layer_set_frame(s_graph_layer,
      GRect(4, graph_y, w - 8, graph_h));
    layer_set_hidden(s_graph_layer, false);

    // Stale row
    layer_set_frame(text_layer_get_layer(s_stale_layer),
      GRect(4, h - 38, w - 8, 18));

    // Date row
    layer_set_frame(text_layer_get_layer(s_date_layer),
      GRect(4, h - 20, w - 8, 18));
    layer_set_hidden(text_layer_get_layer(s_date_layer), false);
  } else {
    // Compact: smaller graph, no date
    int graph_y = glucose_y + 100;
    int graph_h = h - graph_y - 22;
    if (graph_h < 20) graph_h = 20;
    layer_set_frame(s_graph_layer,
      GRect(4, graph_y, w - 8, graph_h));
    layer_set_hidden(s_graph_layer, graph_h < 20);

    layer_set_frame(text_layer_get_layer(s_stale_layer),
      GRect(4, h - 20, w - 8, 18));

    layer_set_hidden(text_layer_get_layer(s_date_layer), true);
  }

  layer_mark_dirty(s_window_layer);
}

static void prv_unobstructed_change(AnimationProgress progress, void *context) {
  GRect bounds = layer_get_unobstructed_bounds(s_window_layer);
  prv_layout_for_bounds(bounds);
}

static void prv_unobstructed_did_change(void *context) {
  GRect full_bounds   = layer_get_bounds(s_window_layer);
  GRect avail_bounds  = layer_get_unobstructed_bounds(s_window_layer);
  bool obstructed = !grect_equal(&full_bounds, &avail_bounds);
  (void)obstructed;  // used via prv_layout_for_bounds
  GRect bounds = layer_get_unobstructed_bounds(s_window_layer);
  prv_layout_for_bounds(bounds);
}

// ─── BT Status ───────────────────────────────────────────────────────────────

static void bluetooth_callback(bool connected) {
  layer_set_hidden(bitmap_layer_get_layer(s_bt_icon_layer), connected);
  if (!connected) {
    static const uint32_t segs[] = { 200, 100, 200 };
    VibePattern pat = { .durations = segs, .num_segments = 3 };
    vibes_enqueue_custom_pattern(pat);
  }
}

// ─── Window Load / Unload ────────────────────────────────────────────────────

static void main_window_load(Window *window) {
  s_window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(s_window_layer);
  int w = bounds.size.w;
  int h = bounds.size.h;

  window_set_background_color(window, GColorBlack);

  // ── Time layer (top right) ──
  s_time_layer = text_layer_create(GRect(w - 75, 2, 73, 28));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentRight);
  layer_add_child(s_window_layer, text_layer_get_layer(s_time_layer));

  // ── BT icon (top left) ──
  s_bt_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BT_ICON);
  s_bt_icon_layer  = bitmap_layer_create(GRect(2, 4, 20, 20));
  bitmap_layer_set_bitmap(s_bt_icon_layer, s_bt_icon_bitmap);
  bitmap_layer_set_compositing_mode(s_bt_icon_layer, GCompOpSet);
  layer_add_child(s_window_layer, bitmap_layer_get_layer(s_bt_icon_layer));
  layer_set_hidden(bitmap_layer_get_layer(s_bt_icon_layer),
    connection_service_peek_pebble_app_connection());

  // ── Glucose value (large) ──
  s_glucose_layer = text_layer_create(GRect(4, 28, w - 4, 52));
  text_layer_set_background_color(s_glucose_layer, GColorClear);
  text_layer_set_text_color(s_glucose_layer, GColorMintGreen);
  text_layer_set_font(s_glucose_layer, fonts_get_system_font(FONT_KEY_LECO_38_BOLD_NUMBERS));
  text_layer_set_text_alignment(s_glucose_layer, GTextAlignmentLeft);
  text_layer_set_text(s_glucose_layer, "--");
  layer_add_child(s_window_layer, text_layer_get_layer(s_glucose_layer));

  // ── Trend arrow ──
  s_trend_layer = text_layer_create(GRect(4, 80, 50, 28));
  text_layer_set_background_color(s_trend_layer, GColorClear);
  text_layer_set_text_color(s_trend_layer, GColorMintGreen);
  text_layer_set_font(s_trend_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text(s_trend_layer, "-");
  layer_add_child(s_window_layer, text_layer_get_layer(s_trend_layer));

  // ── Delta ──
  s_delta_layer = text_layer_create(GRect(60, 80, w - 64, 28));
  text_layer_set_background_color(s_delta_layer, GColorClear);
  text_layer_set_text_color(s_delta_layer, GColorMintGreen);
  text_layer_set_font(s_delta_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text(s_delta_layer, "");
  layer_add_child(s_window_layer, text_layer_get_layer(s_delta_layer));

  // ── Zone label ──
  s_zone_layer = text_layer_create(GRect(4, 106, w - 8, 24));
  text_layer_set_background_color(s_zone_layer, GColorClear);
  text_layer_set_text_color(s_zone_layer, GColorMintGreen);
  text_layer_set_font(s_zone_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_zone_layer, GTextAlignmentLeft);
  text_layer_set_text(s_zone_layer, "NO DATA");
  layer_add_child(s_window_layer, text_layer_get_layer(s_zone_layer));

  // ── Graph layer ──
  s_graph_layer = layer_create(GRect(4, 133, w - 8, 60));
  layer_set_update_proc(s_graph_layer, graph_layer_update_proc);
  layer_add_child(s_window_layer, s_graph_layer);

  // ── Stale row ──
  s_stale_layer = text_layer_create(GRect(4, h - 38, w - 8, 18));
  text_layer_set_background_color(s_stale_layer, GColorClear);
  text_layer_set_text_color(s_stale_layer, GColorLightGray);
  text_layer_set_font(s_stale_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_stale_layer, GTextAlignmentCenter);
  text_layer_set_text(s_stale_layer, "Waiting for data...");
  layer_add_child(s_window_layer, text_layer_get_layer(s_stale_layer));

  // ── Date row ──
  s_date_layer = text_layer_create(GRect(4, h - 20, w - 8, 18));
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorLightGray);
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  layer_add_child(s_window_layer, text_layer_get_layer(s_date_layer));

  // ── Subscribe to UnobstructedArea ──
  UnobstructedAreaHandlers ua_handlers = {
    .change     = prv_unobstructed_change,
    .did_change = prv_unobstructed_did_change
  };
  unobstructed_area_service_subscribe(ua_handlers, NULL);

  // Apply initial layout (handles Quick View already active on startup)
  GRect avail = layer_get_unobstructed_bounds(s_window_layer);
  prv_layout_for_bounds(avail);
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_glucose_layer);
  text_layer_destroy(s_trend_layer);
  text_layer_destroy(s_delta_layer);
  text_layer_destroy(s_zone_layer);
  text_layer_destroy(s_stale_layer);
  layer_destroy(s_graph_layer);
  bitmap_layer_destroy(s_bt_icon_layer);
  gbitmap_destroy(s_bt_icon_bitmap);

  unobstructed_area_service_unsubscribe();
}

// ─── Init / Deinit ───────────────────────────────────────────────────────────

static void init(void) {
  // Load persisted values for cold-start display
  if (persist_exists(PERSIST_GLUCOSE))     s_glucose      = persist_read_int(PERSIST_GLUCOSE);
  if (persist_exists(PERSIST_TREND))       s_trend        = persist_read_int(PERSIST_TREND);
  if (persist_exists(PERSIST_DELTA))       s_delta        = persist_read_int(PERSIST_DELTA);
  if (persist_exists(PERSIST_LAST_READ))   s_last_read_sec = (time_t)persist_read_int(PERSIST_LAST_READ);
  if (persist_exists(PERSIST_USE_MMOL))    s_use_mmol     = persist_read_int(PERSIST_USE_MMOL) != 0;
  if (persist_exists(PERSIST_HIGH_THRESH)) s_high_thresh  = persist_read_int(PERSIST_HIGH_THRESH);
  if (persist_exists(PERSIST_LOW_THRESH))  s_low_thresh   = persist_read_int(PERSIST_LOW_THRESH);
  if (persist_exists(PERSIST_URG_HIGH))    s_urgent_high  = persist_read_int(PERSIST_URG_HIGH);
  if (persist_exists(PERSIST_URG_LOW))     s_urgent_low   = persist_read_int(PERSIST_URG_LOW);

  // Create and configure main window
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
  window_set_window_handlers(s_main_window, (WindowHandlers){
    .load   = main_window_load,
    .unload = main_window_unload
  });
  window_set_click_config_provider(s_main_window, click_config_provider);
  window_stack_push(s_main_window, true);

  // Tick timer
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  update_time();

  // Bluetooth
  connection_service_subscribe((ConnectionHandlers){
    .pebble_app_connection_handler = bluetooth_callback
  });

  // AppMessage
  app_message_register_inbox_received(inbox_received_handler);
  app_message_register_inbox_dropped(inbox_dropped_handler);
  app_message_open(256, 64);
}

static void deinit(void) {
  if (s_flash_timer)  app_timer_cancel(s_flash_timer);
  if (s_alert_timer)  app_timer_cancel(s_alert_timer);
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
