// State 5: Full error — all data unavailable, BT disabled, battery 5% danger
// BT disconnected + battery set via emu commands in the shoot script
var KEY_GLUCOSE_VALUE = 0, KEY_GLUCOSE_TREND = 1, KEY_GLUCOSE_DELTA = 2,
    KEY_LAST_READ_SEC = 3, KEY_GRAPH_DATA = 4,
    KEY_USE_MMOL = 5, KEY_HIGH_THRESHOLD = 6, KEY_LOW_THRESHOLD = 7,
    KEY_URGENT_HIGH = 8, KEY_URGENT_LOW = 9,
    KEY_WEATHER_TEMP = 10, KEY_WEATHER_ICON = 11,
    KEY_LAYOUT = 12, KEY_SLOT_0 = 13, KEY_SLOT_1 = 14,
    KEY_SLOT_2 = 15, KEY_SLOT_3 = 16,
    KEY_WEATHER_TMIN = 17, KEY_WEATHER_TMAX = 18,
    KEY_MOCK_HR = 19, KEY_MOCK_STEPS = 20;

Pebble.addEventListener('ready', function() {
  var graph = [];
  for (var i = 0; i < 36; i++) graph.push(0);

  var msg = {};
  // CGM: stale (last_read_sec=0) + glucose=0 → "--" gray
  msg[KEY_GLUCOSE_VALUE]  = 0;
  msg[KEY_GLUCOSE_TREND]  = 7;   // TREND_NONE
  msg[KEY_GLUCOSE_DELTA]  = 0;
  msg[KEY_LAST_READ_SEC]  = 0;   // stale sentinel
  msg[KEY_GRAPH_DATA]     = graph;
  msg[KEY_USE_MMOL]       = 0;
  msg[KEY_HIGH_THRESHOLD] = 180;
  msg[KEY_LOW_THRESHOLD]  = 70;
  msg[KEY_URGENT_HIGH]    = 250;
  msg[KEY_URGENT_LOW]     = 55;
  // Weather: -128 sentinel → "--°C", no arc
  msg[KEY_WEATHER_TEMP]   = -128;
  msg[KEY_WEATHER_ICON]   = 7;   // default cloud (unavailable)
  msg[KEY_WEATHER_TMIN]   = -128;
  msg[KEY_WEATHER_TMAX]   = -128;
  // Slot config
  msg[KEY_LAYOUT]         = 0;
  msg[KEY_SLOT_0]         = 2;
  msg[KEY_SLOT_1]         = 1;
  msg[KEY_SLOT_2]         = 5;
  msg[KEY_SLOT_3]         = 3;
  // Health: 0 → "--", alert icon
  msg[KEY_MOCK_HR]        = 0;
  msg[KEY_MOCK_STEPS]     = 0;

  Pebble.sendAppMessage(msg,
    function()  { console.log('mock state5 sent OK'); },
    function(e) { console.error('mock state5 failed: ' + JSON.stringify(e)); }
  );
});
