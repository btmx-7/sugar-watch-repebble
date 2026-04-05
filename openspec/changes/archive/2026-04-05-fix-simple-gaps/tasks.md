# Tasks: Fix Simple Gaps

## 1. main.c

- [x] 1.1 Fix `format_delta()`: use `abs(delta_mgdl)` for whole/frac, then prefix sign

## 2. index.js

- [x] 2.1 Fix `lastRead` dateString fallback to use `new Date(dateString).getTime()`
- [x] 2.2 Extract config page URL to `CONFIG_URL` constant with publish comment
- [x] 2.3 Add `dexcomUser` to `showConfiguration` URL params
- [x] 2.4 Add `graphWindow` to `showConfiguration` URL params

## 3. config.html

- [x] 3.1 Load `dexcomUser` from params on open
- [x] 3.2 Add Graph Window `<select>` (1hr/2hr/3hr) to the Display section
- [x] 3.3 Include `graphWindow` value in `save()` data object
