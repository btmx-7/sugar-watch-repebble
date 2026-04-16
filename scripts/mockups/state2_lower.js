// State 2: Lower — BT active, hypo 49 rising (danger), 20% battery warning,
//          min temp, low HR
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
  // Descending graph ending at hypo level
  var graph = [];
  for (var i = 0; i < 36; i++) graph.push(Math.round((80 - i * 0.8 + Math.sin(i * 0.5) * 4) / 2));

  var msg = {};
  // CGM: 49 mg/dL rising (urgent low, danger red)
  msg[KEY_GLUCOSE_VALUE]  = 49;
  msg[KEY_GLUCOSE_TREND]  = 1;   // SINGLE_UP
  msg[KEY_GLUCOSE_DELTA]  = 12;
  msg[KEY_LAST_READ_SEC]  = now;
  msg[KEY_GRAPH_DATA]     = graph;
  msg[KEY_USE_MMOL]       = 0;
  msg[KEY_HIGH_THRESHOLD] = 180;
  msg[KEY_LOW_THRESHOLD]  = 70;
  msg[KEY_URGENT_HIGH]    = 250;
  msg[KEY_URGENT_LOW]     = 55;
  // Weather: -4°C snow, arc at min (temp = tmin)
  msg[KEY_WEATHER_TEMP]   = -4;
  msg[KEY_WEATHER_ICON]   = 5;   // snow
  msg[KEY_WEATHER_TMIN]   = -8;
  msg[KEY_WEATHER_TMAX]   = 0;
  // Slot config
  msg[KEY_LAYOUT]         = 0;
  msg[KEY_SLOT_0]         = 2;
  msg[KEY_SLOT_1]         = 1;
  msg[KEY_SLOT_2]         = 5;
  msg[KEY_SLOT_3]         = 3;
  // Health mock: low HR
  msg[KEY_MOCK_HR]        = 48;
  msg[KEY_MOCK_STEPS]     = 1200;

  Pebble.sendAppMessage(msg,
    function()  { console.log('mock state2 sent OK'); },
    function(e) { console.error('mock state2 failed: ' + JSON.stringify(e)); }
  );
});
