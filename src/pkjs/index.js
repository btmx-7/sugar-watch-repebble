/**
 * Sugar Watch — Phone-side JavaScript
 * Fetches CGM data from Nightscout (or Dexcom Share) and sends to watch via AppMessage
 *
 * Supports:
 *   - Nightscout API v1 (primary)
 *   - Dexcom Share API (secondary, US & International)
 */

// ─── Config page URL ─────────────────────────────────────────────────────────
// UPDATE THIS before publishing: host config.html (e.g. GitHub Pages) and set the URL here.
// Example: 'https://<your-github-username>.github.io/glucoseguard/config.html'
var CONFIG_URL = 'https://example.github.io/glucoseguard/config.html';

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

// ─── Trend direction mapping ─────────────────────────────────────────────────
var TREND_MAP = {
  'DoubleUp':       0,
  'SingleUp':       1,
  'FortyFiveUp':    2,
  'Flat':           3,
  'FortyFiveDown':  4,
  'SingleDown':     5,
  'DoubleDown':     6,
  'NOT COMPUTABLE': 7,
  'RATE OUT OF RANGE': 7,
  'None':           7
};

// ─── Settings ────────────────────────────────────────────────────────────────
var settings = {};

function loadSettings() {
  var keys = ['nsUrl', 'nsToken', 'dexcomUser', 'dexcomPass', 'useMmol',
              'highThresh', 'lowThresh', 'urgentHigh', 'urgentLow',
              'dataSource', 'graphWindow'];
  keys.forEach(function(k) {
    var v = localStorage.getItem('glucoseguard_' + k);
    if (v !== null) settings[k] = v;
  });

  // Defaults
  if (!settings.highThresh)  settings.highThresh  = '180';
  if (!settings.lowThresh)   settings.lowThresh   = '70';
  if (!settings.urgentHigh)  settings.urgentHigh  = '250';
  if (!settings.urgentLow)   settings.urgentLow   = '55';
  if (!settings.useMmol)     settings.useMmol     = '0';
  if (!settings.dataSource)  settings.dataSource  = 'nightscout';
  if (!settings.graphWindow) settings.graphWindow = '37';  // 37 × 5min = ~3hr
}

function saveSettings(data) {
  Object.keys(data).forEach(function(k) {
    localStorage.setItem('glucoseguard_' + k, data[k]);
  });
  loadSettings();
}

// ─── Send data to watch ──────────────────────────────────────────────────────

function sendToWatch(glucose, trend, delta, lastReadSec, graphArray) {
  // Graph data: stored as uint8 = mgdl / 2 (max 840 mg/dL representable)
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
  msg[KEY_USE_MMOL]        = parseInt(settings.useMmol) || 0;
  msg[KEY_HIGH_THRESHOLD]  = parseInt(settings.highThresh) || 180;
  msg[KEY_LOW_THRESHOLD]   = parseInt(settings.lowThresh)  || 70;
  msg[KEY_URGENT_HIGH]     = parseInt(settings.urgentHigh) || 250;
  msg[KEY_URGENT_LOW]      = parseInt(settings.urgentLow)  || 55;

  Pebble.sendAppMessage(msg,
    function() { console.log('Sugar Watch: data sent to watch OK'); },
    function(e) { console.error('Sugar Watch: send failed: ' + JSON.stringify(e)); }
  );
}

// ─── Nightscout Fetch ────────────────────────────────────────────────────────

function fetchNightscout() {
  if (!settings.nsUrl) {
    console.log('Sugar Watch: No Nightscout URL configured');
    return;
  }

  var count = parseInt(settings.graphWindow) || 37;
  var url = settings.nsUrl.replace(/\/$/, '') +
    '/api/v1/entries.json?count=' + count;

  if (settings.nsToken) {
    url += '&token=' + encodeURIComponent(settings.nsToken);
  }

  console.log('Sugar Watch: fetching ' + url);

  var xhr = new XMLHttpRequest();
  xhr.onload = function() {
    if (xhr.status === 200) {
      try {
        var entries = JSON.parse(xhr.responseText);
        if (!entries || entries.length === 0) {
          console.log('Sugar Watch: No entries returned');
          return;
        }

        // Entries are newest-first from Nightscout
        var latest = entries[0];
        var glucose    = parseInt(latest.sgv) || 0;
        var trendStr   = latest.direction || 'None';
        var trend      = TREND_MAP[trendStr] !== undefined ? TREND_MAP[trendStr] : 7;
        var delta      = parseInt(latest.delta) || 0;
        var lastRead   = Math.round((latest.date || new Date(latest.dateString).getTime() || 0) / 1000);

        // If delta not in entry, compute from last 2 entries
        if (!latest.delta && entries.length >= 2) {
          var prev = parseInt(entries[1].sgv) || glucose;
          delta = glucose - prev;
        }

        // Build graph array (oldest to newest for rendering left→right)
        var graphData = [];
        for (var i = entries.length - 1; i >= 0; i--) {
          var sgv = parseInt(entries[i].sgv) || 0;
          graphData.push(sgv);
        }

        sendToWatch(glucose, trend, delta, lastRead, graphData);

      } catch (e) {
        console.error('Sugar Watch: parse error: ' + e.message);
      }
    } else {
      console.error('Sugar Watch: HTTP error ' + xhr.status);
    }
  };
  xhr.onerror = function() {
    console.error('Sugar Watch: network error');
  };
  xhr.open('GET', url);
  xhr.send();
}

// ─── Dexcom Share Fetch ──────────────────────────────────────────────────────

var DEXCOM_LOGIN_URL_US  = 'https://share2.dexcom.com/ShareWebServices/Services/General/LoginPublisherAccountById';
var DEXCOM_LOGIN_URL_OUS = 'https://shareous1.dexcom.com/ShareWebServices/Services/General/LoginPublisherAccountById';
var DEXCOM_READINGS_URL  = '/ShareWebServices/Services/Publisher/ReadPublisherLatestGlucoseValues';
var DEXCOM_APP_ID        = 'd89443d2-327c-4a6f-89e5-496bbb0317db';

var s_dexcom_session_id  = null;
var s_dexcom_base_url    = 'https://share2.dexcom.com';

function dexcomLogin(callback) {
  var url = s_dexcom_base_url + '/ShareWebServices/Services/General/LoginPublisherAccountById';
  var body = JSON.stringify({
    accountName:  settings.dexcomUser,
    password:     settings.dexcomPass,
    applicationId: DEXCOM_APP_ID
  });

  var xhr = new XMLHttpRequest();
  xhr.onload = function() {
    if (xhr.status === 200) {
      s_dexcom_session_id = xhr.responseText.replace(/"/g, '');
      console.log('Sugar Watch: Dexcom session OK');
      callback(true);
    } else if (xhr.status === 500 && s_dexcom_base_url.indexOf('shareous') === -1) {
      // Try international endpoint
      s_dexcom_base_url = 'https://shareous1.dexcom.com';
      console.log('Sugar Watch: trying Dexcom international endpoint');
      dexcomLogin(callback);
    } else {
      console.error('Sugar Watch: Dexcom login failed ' + xhr.status);
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

        // Dexcom: readings are newest-first
        var latest  = readings[0];
        var glucose = parseInt(latest.Value) || 0;
        var trend   = parseInt(latest.Trend) - 1;  // Dexcom uses 1-7, we use 0-6
        if (trend < 0) trend = 7;
        if (trend > 6) trend = 7;

        var delta = 0;
        if (readings.length >= 2) {
          delta = glucose - (parseInt(readings[1].Value) || glucose);
        }

        // Dexcom timestamp: "/Date(milliseconds)/"
        var lastRead = 0;
        var tsMatch  = latest.ST.match(/\d+/);
        if (tsMatch) lastRead = Math.round(parseInt(tsMatch[0]) / 1000);

        var graphData = [];
        for (var i = readings.length - 1; i >= 0; i--) {
          graphData.push(parseInt(readings[i].Value) || 0);
        }

        sendToWatch(glucose, trend, delta, lastRead, graphData);
      } catch(e) {
        console.error('Sugar Watch: Dexcom parse error: ' + e.message);
      }
    } else if (xhr.status === 500) {
      // Session expired, re-login
      s_dexcom_session_id = null;
      fetchDexcom();
    }
  };
  xhr.onerror = function() { console.error('Sugar Watch: Dexcom readings network error'); };
  xhr.open('GET', url);
  xhr.setRequestHeader('Accept', 'application/json');
  xhr.send();
}

function fetchDexcom() {
  if (!settings.dexcomUser || !settings.dexcomPass) {
    console.log('Sugar Watch: No Dexcom credentials configured');
    return;
  }

  if (s_dexcom_session_id) {
    dexcomFetchReadings();
  } else {
    dexcomLogin(function(ok) {
      if (ok) dexcomFetchReadings();
    });
  }
}

// ─── Main Fetch ──────────────────────────────────────────────────────────────

function fetchData() {
  loadSettings();
  if (settings.dataSource === 'dexcom') {
    fetchDexcom();
  } else {
    fetchNightscout();
  }
}

// ─── Pebble Event Handlers ───────────────────────────────────────────────────

Pebble.addEventListener('ready', function() {
  console.log('Sugar Watch JS ready');
  loadSettings();
  fetchData();
  // Refresh every 5 minutes
  setInterval(fetchData, 5 * 60 * 1000);
});

Pebble.addEventListener('appmessage', function(e) {
  console.log('Sugar Watch: message from watch: ' + JSON.stringify(e.payload));
  // Watch can request a refresh by sending any message
  fetchData();
});

Pebble.addEventListener('showConfiguration', function() {
  loadSettings();
  var url = CONFIG_URL +
    '?nsUrl='        + encodeURIComponent(settings.nsUrl       || '') +
    '&dexcomUser='   + encodeURIComponent(settings.dexcomUser  || '') +
    '&dataSource='   + encodeURIComponent(settings.dataSource  || 'nightscout') +
    '&useMmol='      + encodeURIComponent(settings.useMmol     || '0') +
    '&highThresh='   + encodeURIComponent(settings.highThresh  || '180') +
    '&lowThresh='    + encodeURIComponent(settings.lowThresh   || '70') +
    '&urgentHigh='   + encodeURIComponent(settings.urgentHigh  || '250') +
    '&urgentLow='    + encodeURIComponent(settings.urgentLow   || '55') +
    '&graphWindow='  + encodeURIComponent(settings.graphWindow || '37');
  Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
  if (e.response) {
    try {
      var data = JSON.parse(decodeURIComponent(e.response));
      saveSettings(data);
      fetchData();
    } catch(err) {
      console.error('Sugar Watch: config parse error: ' + err);
    }
  }
});
