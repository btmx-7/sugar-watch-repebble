# Idea: Google Home Device Controller for Pebble

Status: Archived
Date: 2026-04-05

## Concept

A Pebble watch app that controls Google Home devices, using the phone as a bridge:
- Watch sends commands via Bluetooth to the companion phone app
- PebbleKit JS on the phone calls the Google Smart Device Management API
- Results displayed back on the watch

## Why it was considered

- No existing Google Home controller for Pebble (new or old platform)
- The Home Assistant WS app covers HA users, but not mainstream Google Home users
- "Works out of box" without self-hosting anything = broader appeal

## Why it was archived

- Google Device Access API requires enrollment and approval (~$5 fee, approval process) - risky within the April 2-19 contest window
- Home Assistant WS app already covers the smart home angle well
- Limited "wow factor" for the judged prize (transparent watch)
- CGM app has a stronger story for the judged category

## Relevant links

- [Google Smart Device Management API](https://developers.google.com/nest/device-access)
- [Pebble Home Assistant WS](https://github.com/skylord123/pebble-home-assistant-ws) - existing competitor
- [Spring 2026 Contest](../spring-2026-pebble-contest.md)
