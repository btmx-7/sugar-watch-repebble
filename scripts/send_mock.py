#!/usr/bin/env python3
"""
send_mock.py — Inject AppMessage mock data into a running Pebble emulator.

Usage:
  python3 scripts/send_mock.py --emulator emery --state 1
  python3 scripts/send_mock.py --emulator gabbro --state 3

States:
  1 = in-range: glucose 110 flat, 22°C clear, HR 72, steps 6543
  2 = lower:    glucose 49 rising (hypo), -4°C snow, HR 48, steps 1200
  3 = bt-off:   stale CGM, -2°C snow cached, HR 52, steps 800
  4 = higher:   glucose 230 rapid fall, 38°C sunny, HR 165, steps 18500
  5 = error:    all stale/unavailable
"""

import sys, struct, uuid, json, time, argparse

LIBPEBBLE_PATH = '/Users/mixbook/.local/pipx/venvs/pebble-tool/lib/python3.13/site-packages'
sys.path.insert(0, LIBPEBBLE_PATH)

from libpebble2.communication import PebbleConnection
from libpebble2.communication.transports.websocket import WebsocketTransport
from libpebble2.protocol.appmessage import AppMessage, AppMessagePush, AppMessageTuple

EMULATOR_JSON = '/var/folders/rd/dzt_0bfs3y7bl1wfmtcbqm700000gn/T/pb-emulator.json'
APP_UUID = uuid.UUID('552fd91e-ad93-4d0f-ae44-74bc9d3108d6')

# AppMessage key constants (must match package.json / main.c)
KEY_GLUCOSE_VALUE  = 0
KEY_GLUCOSE_TREND  = 1
KEY_GLUCOSE_DELTA  = 2
KEY_LAST_READ_SEC  = 3
KEY_GRAPH_DATA     = 4
KEY_USE_MMOL       = 5
KEY_HIGH_THRESHOLD = 6
KEY_LOW_THRESHOLD  = 7
KEY_URGENT_HIGH    = 8
KEY_URGENT_LOW     = 9
KEY_WEATHER_TEMP   = 10
KEY_WEATHER_ICON   = 11
KEY_LAYOUT         = 12
KEY_SLOT_0         = 13
KEY_SLOT_1         = 14
KEY_SLOT_2         = 15
KEY_SLOT_3         = 16
KEY_WEATHER_TMIN   = 17
KEY_WEATHER_TMAX   = 18
KEY_MOCK_HR        = 19
KEY_MOCK_STEPS     = 20

# Trend constants
TREND_FLAT            = 0
TREND_SINGLE_UP       = 2
TREND_FORTY_FIVE_DOWN = 4
TREND_DOUBLE_DOWN     = 6
TREND_NONE            = 7

# Weather icon constants
ICON_CLEAR   = 0
ICON_SNOW    = 5
ICON_DEFAULT = 7


def make_int32(key, value):
    return AppMessageTuple(key=key, type=3, length=4, data=struct.pack('<i', int(value)))


def make_byte_array(key, values):
    data = bytes(v & 0xFF for v in values)
    return AppMessageTuple(key=key, type=0, length=len(data), data=data)


STATES = {
    1: {
        'name': 'in-range',
        # Glucose: 110 mg/dL flat, fresh
        KEY_GLUCOSE_VALUE:  110,
        KEY_GLUCOSE_TREND:  TREND_FLAT,
        KEY_GLUCOSE_DELTA:  2,
        KEY_LAST_READ_SEC:  None,  # will be set to now
        KEY_USE_MMOL:       0,
        KEY_HIGH_THRESHOLD: 180,
        KEY_LOW_THRESHOLD:  70,
        KEY_URGENT_HIGH:    250,
        KEY_URGENT_LOW:     55,
        # Weather: 22°C clear, tmin=15 tmax=28
        KEY_WEATHER_TEMP:   22,
        KEY_WEATHER_ICON:   ICON_CLEAR,
        KEY_WEATHER_TMIN:   15,
        KEY_WEATHER_TMAX:   28,
        # Slots: default layout
        KEY_LAYOUT:  0,
        KEY_SLOT_0:  2,
        KEY_SLOT_1:  1,
        KEY_SLOT_2:  5,
        KEY_SLOT_3:  3,
        # Health
        KEY_MOCK_HR:    72,
        KEY_MOCK_STEPS: 6543,
        # Graph: flat ~110 (encoded as half-values per existing pkjs convention)
        'graph': [55] * 36,
    },
    2: {
        'name': 'lower',
        # Glucose: 49 mg/dL rising (hypo)
        KEY_GLUCOSE_VALUE:  49,
        KEY_GLUCOSE_TREND:  TREND_SINGLE_UP,
        KEY_GLUCOSE_DELTA:  4,
        KEY_LAST_READ_SEC:  None,
        KEY_USE_MMOL:       0,
        KEY_HIGH_THRESHOLD: 180,
        KEY_LOW_THRESHOLD:  70,
        KEY_URGENT_HIGH:    250,
        KEY_URGENT_LOW:     55,
        # Weather: -4°C snow, tmin=-8 tmax=0
        KEY_WEATHER_TEMP:   -4,
        KEY_WEATHER_ICON:   ICON_SNOW,
        KEY_WEATHER_TMIN:   -8,
        KEY_WEATHER_TMAX:   0,
        KEY_LAYOUT:  0,
        KEY_SLOT_0:  2,
        KEY_SLOT_1:  1,
        KEY_SLOT_2:  5,
        KEY_SLOT_3:  3,
        # Health: low HR (rest), few steps
        KEY_MOCK_HR:    48,
        KEY_MOCK_STEPS: 1200,
        # Graph: descending then rising from hypo dip
        'graph': [int((80 - abs(i - 18) * 2)) // 2 for i in range(36)],
    },
    3: {
        'name': 'bt-off',
        # CGM: stale (last_read_sec=0)
        KEY_GLUCOSE_VALUE:  72,
        KEY_GLUCOSE_TREND:  TREND_FORTY_FIVE_DOWN,
        KEY_GLUCOSE_DELTA:  -5,
        KEY_LAST_READ_SEC:  0,   # stale sentinel
        KEY_USE_MMOL:       0,
        KEY_HIGH_THRESHOLD: 180,
        KEY_LOW_THRESHOLD:  70,
        KEY_URGENT_HIGH:    250,
        KEY_URGENT_LOW:     55,
        # Weather: cached -2°C snow before BT cut
        KEY_WEATHER_TEMP:   -2,
        KEY_WEATHER_ICON:   ICON_SNOW,
        KEY_WEATHER_TMIN:   -5,
        KEY_WEATHER_TMAX:   3,
        KEY_LAYOUT:  0,
        KEY_SLOT_0:  2,
        KEY_SLOT_1:  1,
        KEY_SLOT_2:  5,
        KEY_SLOT_3:  3,
        # Health: still works on-watch
        KEY_MOCK_HR:    52,
        KEY_MOCK_STEPS: 800,
        'graph': [0] * 36,
    },
    4: {
        'name': 'higher',
        # Glucose: 230 mg/dL rapid fall (above high, warning)
        KEY_GLUCOSE_VALUE:  230,
        KEY_GLUCOSE_TREND:  TREND_DOUBLE_DOWN,
        KEY_GLUCOSE_DELTA:  -18,
        KEY_LAST_READ_SEC:  None,
        KEY_USE_MMOL:       0,
        KEY_HIGH_THRESHOLD: 180,
        KEY_LOW_THRESHOLD:  70,
        KEY_URGENT_HIGH:    250,
        KEY_URGENT_LOW:     55,
        # Weather: 38°C clear sunny, tmin=28 tmax=42
        KEY_WEATHER_TEMP:   38,
        KEY_WEATHER_ICON:   ICON_CLEAR,
        KEY_WEATHER_TMIN:   28,
        KEY_WEATHER_TMAX:   42,
        KEY_LAYOUT:  0,
        KEY_SLOT_0:  2,
        KEY_SLOT_1:  1,
        KEY_SLOT_2:  5,
        KEY_SLOT_3:  3,
        # Health: high HR (exercise), many steps
        KEY_MOCK_HR:    165,
        KEY_MOCK_STEPS: 18500,
        # Graph: rising arc peaking high
        'graph': [int((160 + i * 2) / 2) for i in range(36)],
    },
    5: {
        'name': 'error',
        # CGM: stale + glucose=0
        KEY_GLUCOSE_VALUE:  0,
        KEY_GLUCOSE_TREND:  TREND_NONE,
        KEY_GLUCOSE_DELTA:  0,
        KEY_LAST_READ_SEC:  0,   # stale sentinel
        KEY_USE_MMOL:       0,
        KEY_HIGH_THRESHOLD: 180,
        KEY_LOW_THRESHOLD:  70,
        KEY_URGENT_HIGH:    250,
        KEY_URGENT_LOW:     55,
        # Weather: unavailable (-128 sentinel)
        KEY_WEATHER_TEMP:   -128,
        KEY_WEATHER_ICON:   ICON_DEFAULT,
        KEY_WEATHER_TMIN:   -128,
        KEY_WEATHER_TMAX:   -128,
        KEY_LAYOUT:  0,
        KEY_SLOT_0:  2,
        KEY_SLOT_1:  1,
        KEY_SLOT_2:  5,
        KEY_SLOT_3:  3,
        # Health: unavailable
        KEY_MOCK_HR:    0,
        KEY_MOCK_STEPS: 0,
        'graph': [0] * 36,
    },
}


def send_state(emulator: str, state_num: int):
    with open(EMULATOR_JSON) as f:
        info = json.load(f)

    emulator_versions = info.get(emulator)
    if not emulator_versions:
        raise RuntimeError(f'Emulator "{emulator}" not found in {EMULATOR_JSON}')
    version = list(emulator_versions.keys())[0]
    port = emulator_versions[version]['pypkjs']['port']

    state = STATES[state_num]
    print(f'  Connecting to {emulator} pypkjs on port {port} (state {state_num}: {state["name"]})...')

    # connect() establishes the WebSocket and starts the reader thread.
    # Skip run_async() (which calls fetch_watch_info()) — the watch sometimes
    # doesn't respond to WatchVersionRequest (especially on gabbro/R2).
    # send_packet() only writes to the transport so no version info is needed.
    pebble = PebbleConnection(WebsocketTransport(f'ws://localhost:{port}'))
    pebble.connect()
    time.sleep(0.3)

    now = int(time.time())

    tuples = []
    for key, val in state.items():
        if key == 'name' or key == 'graph':
            continue
        if key == KEY_LAST_READ_SEC and val is None:
            val = now
        tuples.append(make_int32(key, val))

    graph = state.get('graph', [0] * 36)
    # Clamp graph values to [0, 127] (byte array, unsigned)
    graph_clamped = [max(0, min(127, v)) for v in graph]
    tuples.append(make_byte_array(KEY_GRAPH_DATA, graph_clamped))

    msg = AppMessage(
        command=0x01,
        transaction_id=1,
        data=AppMessagePush(
            uuid=APP_UUID,
            count=len(tuples),
            dictionary=tuples,
        )
    )

    pebble.send_packet(msg)
    time.sleep(0.5)
    print(f'  Sent state {state_num} ({state["name"]}) to {emulator}')


def main():
    parser = argparse.ArgumentParser(description='Inject mock AppMessage into Pebble emulator')
    parser.add_argument('--emulator', required=True, choices=['emery', 'gabbro'],
                        help='Emulator name (emery=T2, gabbro=R2)')
    parser.add_argument('--state', required=True, type=int, choices=[1, 2, 3, 4, 5],
                        help='State number 1-5')
    args = parser.parse_args()
    send_state(args.emulator, args.state)


if __name__ == '__main__':
    main()
