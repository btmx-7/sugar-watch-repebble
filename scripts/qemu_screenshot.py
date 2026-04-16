#!/usr/bin/env python3
"""
qemu_screenshot.py — Take a screenshot via QEMU monitor screendump.
Used as fallback when `pebble screenshot` fails (e.g., gabbro after state 5 causes
the watch app to crash-restart, breaking the pypkjs protocol connection).

Usage:
  python3 scripts/qemu_screenshot.py --emulator gabbro --output /path/to/out.png
"""

import sys, json, socket, time, argparse, os, tempfile
from PIL import Image

EMULATOR_JSON = '/var/folders/rd/dzt_0bfs3y7bl1wfmtcbqm700000gn/T/pb-emulator.json'


def qemu_screendump(monitor_port: int, ppm_path: str, timeout: float = 5.0):
    s = socket.socket()
    s.settimeout(timeout)
    s.connect(('localhost', monitor_port))
    s.recv(4096)  # discard banner
    s.sendall(f'screendump {ppm_path}\n'.encode())
    time.sleep(0.5)
    s.recv(4096)  # discard response
    s.close()


def main():
    parser = argparse.ArgumentParser(description='QEMU screendump fallback for Pebble emulator')
    parser.add_argument('--emulator', required=True, choices=['emery', 'gabbro'])
    parser.add_argument('--output',   required=True, help='Output PNG path')
    args = parser.parse_args()

    with open(EMULATOR_JSON) as f:
        info = json.load(f)

    emu_info = info.get(args.emulator)
    if not emu_info:
        raise RuntimeError(f'Emulator "{args.emulator}" not found in {EMULATOR_JSON}')

    version = list(emu_info.keys())[0]
    monitor_port = emu_info[version]['qemu']['monitor']

    ppm_tmp = args.output.replace('.png', '_qemu_tmp.ppm')
    try:
        qemu_screendump(monitor_port, ppm_tmp)

        if not os.path.exists(ppm_tmp):
            raise RuntimeError(f'PPM file not created: {ppm_tmp}')

        img = Image.open(ppm_tmp)
        img.save(args.output)
        print(f'  saved (qemu fallback): {args.output}')
    finally:
        if os.path.exists(ppm_tmp):
            os.unlink(ppm_tmp)


if __name__ == '__main__':
    main()
