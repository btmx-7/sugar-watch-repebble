// State 4: Higher — BT active, hyper 230 rapid fall (warning yellow), charging,
//          max temp, high HR
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
  // Rising graph peaking high
  var graph = [];
  for (var i = 0; i < 36; i++) graph.push(Math.round((160 + i * 2 + Math.sin(i * 0.4) * 6) / 2));

  var msg = {};
  // CGM: 230 mg/dL rapid fall (high, warning yellow)
  msg[KEY_GLUCOSE_VALUE]  = 230;
  msg[KEY_GLUCOSE_TREND]  = 6;   // DOUBLE_DOWN
  msg[KEY_GLUCOSE_DELTA]  = -18;
  msg[KEY_LAST_READ_SEC]  = now;
  msg[KEY_GRAPH_DATA]     = graph;
  msg[KEY_USE_MMOL]       = 0;
  msg[KEY_HIGH_THRESHOLD] = 180;
  msg[KEY_LOW_THRESHOLD]  = 70;
  msg[KEY_URGENT_HIGH]    = 250;
  msg[KEY_URGENT_LOW]     = 55;
  // Weather: 38°C clear/sunny, arc at max
  msg[KEY_WEATHER_TEMP]   = 38;
  msg[KEY_WEATHER_ICON]   = 0;   // clear/sunny
  msg[KEY_WEATHER_TMIN]   = 28;
  msg[KEY_WEATHER_TMAX]   = 42;
  // Slot config
  msg[KEY_LAYOUT]         = 0;
  msg[KEY_SLOT_0]         = 2;
  msg[KEY_SLOT_1]         = 1;
  msg[KEY_SLOT_2]         = 5;
  msg[KEY_SLOT_3]         = 3;
  // Health: high HR (exercise)
  msg[KEY_MOCK_HR]        = 165;
  msg[KEY_MOCK_STEPS]     = 18500;

  Pebble.sendAppMessage(msg,
    function()  { console.log('mock state4 sent OK'); },
    function(e) { console.error('mock state4 failed: ' + JSON.stringify(e)); }
  );
});
