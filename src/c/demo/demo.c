/**
 * Steady — Demo scenario fixtures (QA-only)
 *
 * Data table only. The apply/cycle logic lives in main.c where it can
 * touch the watchface's internal state.
 *
 * Trend values match GlucoseTrend in main.c:
 *   0=DOUBLE_UP, 1=SINGLE_UP, 2=45_UP, 3=FLAT,
 *   4=45_DOWN,   5=SINGLE_DOWN, 6=DOUBLE_DOWN, 7=NONE
 * Slot values match SlotType:
 *   0=NONE, 1=BATTERY, 2=WEATHER, 3=HEART_RATE, 4=STEPS, 5=CGM
 * Layout: 0=SIMPLE, 1=DASHBOARD
 * Graph pattern: 0=wave, 1=rising, 2=falling, 3=flat-low, 4=spike
 */
#ifdef DEMO_DATA
#include "demo.h"

const DemoScenario demo_scenarios[DEMO_SCENARIO_COUNT] = {
  /* name           gluc trend delta  age   wT  wMin wMax wI  HR   steps  layout slots         graph */
  { "urgent_low",   45,  6,    -15,   60,   10, 4,   15,  0,  88,  1234,  0,     {2,1,5,3},    2 },
  { "low",          65,  5,    -8,    120,  10, 4,   15,  0,  72,  3201,  0,     {2,1,5,3},    2 },
  { "in_range",     120, 3,     2,    180,  10, 4,   15,  0,  128, 6842,  0,     {2,1,5,3},    0 },
  { "high",         195, 1,     10,   120,  22, 16,  28,  0,  95,  8500,  0,     {2,1,5,3},    1 },
  { "urgent_high",  270, 0,     18,   60,   22, 16,  28,  0,  110, 12034, 0,     {2,1,5,3},    1 },
  { "stale",        120, 7,     0,    1800, 10, 4,   15,  0,  0,   0,     0,     {2,1,5,3},    3 },
  { "dashboard",    142, 3,     3,    180,  12, 6,   18,  0,  78,  6842,  1,     {2,1,5,0},    0 },
  { "zero_state",   0,   7,     0,    0,    -128,-128,-128,7, 0,   0,     0,     {0,0,0,0},    3 },
};

#endif /* DEMO_DATA */
