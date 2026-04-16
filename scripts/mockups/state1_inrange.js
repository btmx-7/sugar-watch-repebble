// State 1: All in range — BT active, all slots populated with healthy values
// Slots: Weather(TL) | Battery(TR) | CGM(BL) | HR(BR)
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
  var now = Math.floor(Date.now() / 1000);
  var graph = [];
  for (var i = 0; i < 36; i++) graph.push(Math.round((100 + Math.sin(i * 0.3) * 8) / 2));

  var msg = {};
  // CGM: 110 mg/dL flat, in-range (cyan)
  msg[KEY_GLUCOSE_VALUE]  = 110;
  msg[KEY_GLUCOSE_TREND]  = 3;   // FLAT
  msg[KEY_GLUCOSE_DELTA]  = 1;
  msg[KEY_LAST_READ_SEC]  = now;
  msg[KEY_GRAPH_DATA]     = graph;
  msg[KEY_USE_MMOL]       = 0;
  msg[KEY_HIGH_THRESHOLD] = 180;
  msg[KEY_LOW_THRESHOLD]  = 70;
  msg[KEY_URGENT_HIGH]    = 250;
  msg[KEY_URGENT_LOW]     = 55;
  // Weather: 22°C clear, arc at 50% of [15, 28]
  msg[KEY_WEATHER_TEMP]   = 22;
  msg[KEY_WEATHER_ICON]   = 0;   // clear/sunny
  msg[KEY_WEATHER_TMIN]   = 15;
  msg[KEY_WEATHER_TMAX]   = 28;
  // Slot config
  msg[KEY_LAYOUT]         = 0;   // Simple
  msg[KEY_SLOT_0]         = 2;   // Weather
  msg[KEY_SLOT_1]         = 1;   // Battery
  msg[KEY_SLOT_2]         = 5;   // CGM
  msg[KEY_SLOT_3]         = 3;   // Heart Rate
  // Health mock
  msg[KEY_MOCK_HR]        = 72;
  msg[KEY_MOCK_STEPS]     = 6543;

  Pebble.sendAppMessage(msg,
    function()  { console.log('mock state1 sent OK'); },
    function(e) { console.error('mock state1 failed: ' + JSON.stringify(e)); }
  );
});
