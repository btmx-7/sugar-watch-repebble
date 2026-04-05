<!-- All pixel values and font choices come from layout.md. Reference it explicitly. -->

## 1. Constants and fonts

<!-- Update FONT_KEY_* constants, zone y-coordinate defines, GRect frame values. -->
- [ ] 1.1 <!-- e.g., "Update glucose font constant to FONT_KEY_LECO_42_BOLD_NUMBERS" -->
- [ ] 1.2 <!-- e.g., "Update hero zone height define to 58px" -->

## 2. Layer changes

<!-- Create new layers, remove deleted layers, resize or move existing layers. -->
<!-- Pattern: declare static → create in window_load → add to parent → destroy in window_unload -->
- [ ] 2.1 <!-- e.g., "Add s_separator_layer declaration and creation in window_load" -->
- [ ] 2.2 <!-- e.g., "Remove s_zone_layer declaration and all references" -->

## 3. Display logic

<!-- Update update_display() and any layer_update_proc callbacks. -->
<!-- Include snprintf buffer size calculations as comments. -->
- [ ] 3.1 <!-- e.g., "Update freshness text format to compact 'Nm' in update_display" -->
- [ ] 3.2 <!-- e.g., "Update zone color logic to use GColorOrange for 180-250 range" -->

## 4. Layout function

<!-- Update prv_layout_for_bounds() for normal and compact (Quick View) branches. -->
<!-- Verify all y values match layout.md zone breakdown table. -->
- [ ] 4.1 <!-- e.g., "Update normal mode: glucose_y=24, meta_y=82, separator_y=108" -->
- [ ] 4.2 <!-- e.g., "Update compact mode: hide graph and footer layers" -->

## 5. Layer cleanup

<!-- Update window_unload() to destroy all new layers. Reverse creation order. -->
- [ ] 5.1 <!-- e.g., "Add text_layer_destroy(s_separator_layer) to window_unload" -->

## 6. Build, install, and visual verification

<!-- Follows the pebble-watchface skill delivery workflow: build → install → screenshot → verify -->

- [ ] 6.1 `pebble build` passes with zero warnings, zero errors
- [ ] 6.2 `pebble install --emulator emery` installs successfully on Time 2 emulator
- [ ] 6.3 `pebble screenshot --emulator emery` - capture screenshot for visual check
- [ ] 6.4 Visual check - Cropping: all elements fully visible, nothing clipped at edges
- [ ] 6.5 Visual check - Positioning: elements match ASCII diagram in layout.md
- [ ] 6.6 Visual check - Colors: zone colors match color palette in layout.md
- [ ] 6.7 Visual check - Intent: glucose value is the dominant element, readable at a glance
- [ ] 6.8 Quick View mode: compact layout correct, graph/footer hidden
- [ ] 6.9 Stale data state: freshness indicator updates correctly after 15+ min
- [ ] 6.10 Out-of-range state: correct color and alert behavior
- [ ] 6.11 Round 2: `pebble install --emulator gabbro` + screenshot (delete if not targeted)

## 7. Publish (optional - for contest submission)

<!-- Use pebble publish to push to the Pebble App Store. -->
<!-- Required for contest eligibility and community hearts. -->

- [ ] 7.1 `pebble publish` - publishes PBW to Pebble App Store
- [ ] 7.2 Verify app appears on apps.repebble.com with correct screenshots and description
