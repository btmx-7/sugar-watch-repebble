/**
 * Steady — Phone-side JavaScript
 * Fetches CGM data from Nightscout (or Dexcom Share) and sends to watch via AppMessage.
 * Also fetches weather via OpenMeteo (no API key required).
 */

// ─── Config page URL ─────────────────────────────────────────────────────────
var CONFIG_URL = 'https://example.github.io/steady/config.html';

// ─── AppMessage Key constants (must match main.c) ────────────────────────────
var KEY_GLUCOSE_VALUE   = 0;
var KEY_GLUCOSE_TREND   = 1;
var KEY_GLUCOSE_DELTA   = 2;
var KEY_LAST_READ_SEC   = 3;
var KEY_GRAPH_DATA      = 4;
var KEY_USE_MMOL        = 5;
var KEY_HIGH_THRESHOLD  = 6;
var KEY_LOW_THRESHOLD   = 7;
var KEY_URGENT_HIGH     = 8;
var KEY_URGENT_LOW      = 9;
var KEY_WEATHER_TEMP    = 10;
var KEY_WEATHER_ICON    = 11;
var KEY_LAYOUT          = 12;
var KEY_SLOT_0          = 13;
var KEY_SLOT_1          = 14;
var KEY_SLOT_2          = 15;
var KEY_SLOT_3          = 16;

// ─── Trend direction mapping ─────────────────────────────────────────────────
var TREND_MAP = {
  'DoubleUp':          0,
  'SingleUp':          1,
  'FortyFiveUp':       2,
  'Flat':              3,
  'FortyFiveDown':     4,
  'SingleDown':        5,
  'DoubleDown':        6,
  'NOT COMPUTABLE':    7,
  'RATE OUT OF RANGE': 7,
  'None':              7
};

// ─── Settings ────────────────────────────────────────────────────────────────
var settings = {};

function loadSettings() {
  var keys = [
    'nsUrl', 'nsToken', 'dexcomUser', 'dexcomPass', 'useMmol',
    'highThresh', 'lowThresh', 'urgentHigh', 'urgentLow',
    'dataSource', 'graphWindow',
    'layout', 'slot0', 'slot1', 'slot2', 'slot3'
  ];
  keys.forEach(function(k) {
    var v = localStorage.getItem('steady_' + k);
    if (v !== null) settings[k] = v;
  });

  if (!settings.highThresh)  settings.highThresh  = '180';
  if (!settings.lowThresh)   settings.lowThresh   = '70';
  if (!settings.urgentHigh)  settings.urgentHigh  = '250';
  if (!settings.urgentLow)   settings.urgentLow   = '55';
  if (!settings.useMmol)     settings.useMmol     = '0';
  if (!settings.dataSource)  settings.dataSource  = 'nightscout';
  if (!settings.graphWindow) settings.graphWindow = '37';
  if (!settings.layout)      settings.layout      = '0';  // LAYOUT_SIMPLE
  if (!settings.slot0)       settings.slot0       = '2';  // SLOT_WEATHER
  if (!settings.slot1)       settings.slot1       = '1';  // SLOT_BATTERY
  if (!settings.slot2)       settings.slot2       = '5';  // SLOT_CGM
  if (!settings.slot3)       settings.slot3       = '3';  // SLOT_HEART_RATE
}

function saveSettings(data) {
  Object.keys(data).forEach(function(k) {
    localStorage.setItem('steady_' + k, data[k]);
  });
  loadSettings();
}

// ─── Send CGM data to watch ──────────────────────────────────────────────────

function sendToWatch(glucose, trend, delta, lastReadSec, graphArray) {
  var graphBytes = new Array(graphArray.length);
  for (var i = 0; i < graphArray.length; i++) {
    graphBytes[i] = Math.round(Math.min(graphArray[i], 510) / 2);
  }

  var msg = {};
  msg[KEY_GLUCOSE_VALUE]   = glucose;
  msg[KEY_GLUCOSE_TREND]   = trend;
  msg[KEY_GLUCOSE_DELTA]   = delta;
  msg[KEY_LAST_READ_SEC]   = lastReadSec;
  msg[KEY_GRAPH_DATA]      = graphBytes;
  msg[KEY_USE_MMOL]        = parseInt(settings.useMmol)    || 0;
  msg[KEY_HIGH_THRESHOLD]  = parseInt(settings.highThresh) || 180;
  msg[KEY_LOW_THRESHOLD]   = parseInt(settings.lowThresh)  || 70;
  msg[KEY_URGENT_HIGH]     = parseInt(settings.urgentHigh) || 250;
  msg[KEY_URGENT_LOW]      = parseInt(settings.urgentLow)  || 55;
  msg[KEY_LAYOUT]          = parseInt(settings.layout)     || 0;
  msg[KEY_SLOT_0]          = parseInt(settings.slot0)      || 0;
  msg[KEY_SLOT_1]          = parseInt(settings.slot1)      || 0;
  msg[KEY_SLOT_2]          = parseInt(settings.slot2)      || 0;
  msg[KEY_SLOT_3]          = parseInt(settings.slot3)      || 0;

  Pebble.sendAppMessage(msg,
    function()  { console.log('Steady: CGM data sent OK'); },
    function(e) { console.error('Steady: CGM send failed: ' + JSON.stringify(e)); }
  );
}

// ─── Weather via OpenMeteo ────────────────────────────────────────────────────

function weatherCodeToIconIndex(code) {
  // WMO weather code → 0-7 icon index
  if (code === 0)                                    return 0;  // clear
  if (code >= 1  && code <= 2)                       return 1;  // partly cloudy
  if (code === 3)                                    return 2;  // overcast
  if (code >= 45 && code <= 48)                      return 6;  // fog
  if ((code >= 51 && code <= 67) ||
      (code >= 80 && code <= 82))                    return 3;  // rain/drizzle
  if ((code >= 71 && code <= 77) ||
      (code >= 85 && code <= 86))                    return 5;  // snow
  if (code >= 95 && code <= 99)                      return 4;  // thunderstorm
  return 7;  // default cloud
}

function fetchWeather() {
  navigator.geolocation.getCurrentPosition(
    function(pos) {
      var lat = pos.coords.latitude;
      var lon = pos.coords.longitude;
      var url = 'https://api.open-meteo.com/v1/forecast' +
        '?latitude='  + lat +
        '&longitude=' + lon +
        '&current=temperature_2m,weather_code' +
        '&temperature_unit=celsius';

      var xhr = new XMLHttpRequest();
      xhr.onload = function() {
        if (xhr.status === 200) {
          try {
            var data = JSON.parse(xhr.responseText);
            var temp = Math.round(data.current.temperature_2m);
            var icon = weatherCodeToIconIndex(data.current.weather_code);
            var msg  = {};
            msg[KEY_WEATHER_TEMP] = temp;
            msg[KEY_WEATHER_ICON] = icon;
            Pebble.sendAppMessage(msg,
              function()  { console.log('Steady: weather sent OK (' + temp + 'C icon=' + icon + ')'); },
              function(e) { console.error('Steady: weather send failed: ' + JSON.stringify(e)); }
            );
          } catch(e) {
            console.error('Steady: weather parse error: ' + e.message);
          }
        } else {
          console.error('Steady: weather HTTP error ' + xhr.status);
        }
      };
      xhr.onerror = function() { console.error('Steady: weather network error'); };
      xhr.open('GET', url);
      xhr.send();
    },
    function(err) {
      // Geolocation denied or unavailable: send sentinel value (-128)
      console.log('Steady: geolocation unavailable, code=' + err.code);
      var msg = {};
      msg[KEY_WEATHER_TEMP] = -128;
      msg[KEY_WEATHER_ICON] = 7;
      Pebble.sendAppMessage(msg, function(){}, function(){});
    }
  );
}

// ─── Nightscout Fetch ────────────────────────────────────────────────────────

function fetchNightscout() {
  if (!settings.nsUrl) {
    console.log('Steady: No Nightscout URL configured');
    return;
  }

  var count = parseInt(settings.graphWindow) || 37;
  var url = settings.nsUrl.replace(/\/$/, '') +
    '/api/v1/entries.json?count=' + count;
  if (settings.nsToken) url += '&token=' + encodeURIComponent(settings.nsToken);

  var xhr = new XMLHttpRequest();
  xhr.onload = function() {
    if (xhr.status === 200) {
      try {
        var entries = JSON.parse(xhr.responseText);
        if (!entries || entries.length === 0) return;

        var latest  = entries[0];
        var glucose  = parseInt(latest.sgv) || 0;
        var trendStr = latest.direction || 'None';
        var trend    = TREND_MAP[trendStr] !== undefined ? TREND_MAP[trendStr] : 7;
        var delta    = parseInt(latest.delta) || 0;
        var lastRead = Math.round((latest.date || new Date(latest.dateString).getTime() || 0) / 1000);

        if (!latest.delta && entries.length >= 2) {
          delta = glucose - (parseInt(entries[1].sgv) || glucose);
        }

        var graphData = [];
        for (var i = entries.length - 1; i >= 0; i--) {
          graphData.push(parseInt(entries[i].sgv) || 0);
        }

        sendToWatch(glucose, trend, delta, lastRead, graphData);
      } catch(e) {
        console.error('Steady: NS parse error: ' + e.message);
      }
    } else {
      console.error('Steady: NS HTTP error ' + xhr.status);
    }
  };
  xhr.onerror = function() { console.error('Steady: NS network error'); };
  xhr.open('GET', url);
  xhr.send();
}

// ─── Dexcom Share Fetch ──────────────────────────────────────────────────────

var DEXCOM_READINGS_URL = '/ShareWebServices/Services/Publisher/ReadPublisherLatestGlucoseValues';
var DEXCOM_APP_ID       = 'd89443d2-327c-4a6f-89e5-496bbb0317db';
var s_dexcom_session_id = null;
var s_dexcom_base_url   = 'https://share2.dexcom.com';

function dexcomLogin(callback) {
  var url  = s_dexcom_base_url + '/ShareWebServices/Services/General/LoginPublisherAccountById';
  var body = JSON.stringify({
    accountName:   settings.dexcomUser,
    password:      settings.dexcomPass,
    applicationId: DEXCOM_APP_ID
  });
  var xhr = new XMLHttpRequest();
  xhr.onload = function() {
    if (xhr.status === 200) {
      s_dexcom_session_id = xhr.responseText.replace(/"/g, '');
      callback(true);
    } else if (xhr.status === 500 && s_dexcom_base_url.indexOf('shareous') === -1) {
      s_dexcom_base_url = 'https://shareous1.dexcom.com';
      dexcomLogin(callback);
    } else {
      console.error('Steady: Dexcom login failed ' + xhr.status);
      callback(false);
    }
  };
  xhr.onerror = function() { callback(false); };
  xhr.open('POST', url);
  xhr.setRequestHeader('Content-Type', 'application/json');
  xhr.setRequestHeader('Accept', 'application/json');
  xhr.send(body);
}

function dexcomFetchReadings() {
  var count = parseInt(settings.graphWindow) || 37;
  var url = s_dexcom_base_url + DEXCOM_READINGS_URL +
    '?sessionId=' + s_dexcom_session_id +
    '&minutes=180&maxCount=' + count;
  var xhr = new XMLHttpRequest();
  xhr.onload = function() {
    if (xhr.status === 200) {
      try {
        var readings = JSON.parse(xhr.responseText);
        if (!readings || readings.length === 0) return;
        var latest  = readings[0];
        var glucose = parseInt(latest.Value) || 0;
        var trend   = parseInt(latest.Trend) - 1;
        if (trend < 0 || trend > 6) trend = 7;
        var delta   = readings.length >= 2 ?
          glucose - (parseInt(readings[1].Value) || glucose) : 0;
        var lastRead = 0;
        var tsMatch  = latest.ST.match(/\d+/);
        if (tsMatch) lastRead = Math.round(parseInt(tsMatch[0]) / 1000);
        var graphData = [];
        for (var i = readings.length - 1; i >= 0; i--) {
          graphData.push(parseInt(readings[i].Value) || 0);
        }
        sendToWatch(glucose, trend, delta, lastRead, graphData);
      } catch(e) {
        console.error('Steady: Dexcom parse error: ' + e.message);
      }
    } else if (xhr.status === 500) {
      s_dexcom_session_id = null;
      fetchDexcom();
    }
  };
  xhr.onerror = function() { console.error('Steady: Dexcom network error'); };
  xhr.open('GET', url);
  xhr.setRequestHeader('Accept', 'application/json');
  xhr.send();
}

function fetchDexcom() {
  if (!settings.dexcomUser || !settings.dexcomPass) return;
  if (s_dexcom_session_id) dexcomFetchReadings();
  else dexcomLogin(function(ok) { if (ok) dexcomFetchReadings(); });
}

// ─── Main Fetch ──────────────────────────────────────────────────────────────

function fetchData() {
  loadSettings();
  if (settings.dataSource === 'dexcom') fetchDexcom();
  else fetchNightscout();
}

// ─── Pebble Event Handlers ───────────────────────────────────────────────────

Pebble.addEventListener('ready', function() {
  console.log('Steady JS ready');
  loadSettings();
  fetchData();
  fetchWeather();
  setInterval(fetchData,    5  * 60 * 1000);  // CGM every 5 min
  setInterval(fetchWeather, 30 * 60 * 1000);  // weather every 30 min
});

Pebble.addEventListener('appmessage', function(e) {
  console.log('Steady: message from watch');
  fetchData();
});

Pebble.addEventListener('showConfiguration', function() {
  loadSettings();
  var url = CONFIG_URL +
    '?nsUrl='       + encodeURIComponent(settings.nsUrl       || '') +
    '&dexcomUser='  + encodeURIComponent(settings.dexcomUser  || '') +
    '&dataSource='  + encodeURIComponent(settings.dataSource  || 'nightscout') +
    '&useMmol='     + encodeURIComponent(settings.useMmol     || '0') +
    '&highThresh='  + encodeURIComponent(settings.highThresh  || '180') +
    '&lowThresh='   + encodeURIComponent(settings.lowThresh   || '70') +
    '&urgentHigh='  + encodeURIComponent(settings.urgentHigh  || '250') +
    '&urgentLow='   + encodeURIComponent(settings.urgentLow   || '55') +
    '&graphWindow=' + encodeURIComponent(settings.graphWindow || '37') +
    '&layout='      + encodeURIComponent(settings.layout      || '0') +
    '&slot0='       + encodeURIComponent(settings.slot0       || '2') +
    '&slot1='       + encodeURIComponent(settings.slot1       || '1') +
    '&slot2='       + encodeURIComponent(settings.slot2       || '5') +
    '&slot3='       + encodeURIComponent(settings.slot3       || '3');
  Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
  if (e.response) {
    try {
      var data = JSON.parse(decodeURIComponent(e.response));
      saveSettings(data);
      // Send layout + slot settings to watch immediately
      var msg = {};
      msg[KEY_LAYOUT] = parseInt(data.layout) || 0;
      msg[KEY_SLOT_0] = parseInt(data.slot0)  || 0;
      msg[KEY_SLOT_1] = parseInt(data.slot1)  || 0;
      msg[KEY_SLOT_2] = parseInt(data.slot2)  || 0;
      msg[KEY_SLOT_3] = parseInt(data.slot3)  || 0;
      Pebble.sendAppMessage(msg, function(){}, function(){});
      fetchData();
    } catch(err) {
      console.error('Steady: config parse error: ' + err);
    }
  }
});
