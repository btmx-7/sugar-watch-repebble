/**
 * Steady — Demo scenario fixtures (QA-only, compiled under -DDEMO_DATA)
 *
 * Scenario table for visual QA against Figma. Not part of release builds.
 * See src/c/demo/README.md for usage.
 */
#pragma once

#ifdef DEMO_DATA
#include <stdint.h>

typedef struct {
  const char *name;
  int32_t   glucose;               // mg/dL (0 = no data)
  int32_t   trend;                 // GlucoseTrend enum value
  int32_t   delta;                 // mg/dL delta
  int32_t   last_read_offset_sec;  // seconds before now (0 = no reading)
  int8_t    weather_temp;          // -128 = unavailable
  uint8_t   weather_icon;          // 0..7
  int       heart_rate;            // BPM (0 = none)
  uint32_t  step_count;
  uint8_t   layout;                // WatchLayout
  uint8_t   slots[4];              // SlotType per slot A-D
  uint8_t   graph_pattern;         // 0=wave, 1=rising, 2=falling, 3=flat-low, 4=spike
} DemoScenario;

#define DEMO_SCENARIO_COUNT 8

/* Default state if DEMO_STATE is not overridden at compile time. */
#ifndef DEMO_STATE
#define DEMO_STATE 2
#endif

extern const DemoScenario demo_scenarios[DEMO_SCENARIO_COUNT];

#endif /* DEMO_DATA */
