## Benchmark References

<!-- Pull the 2-3 most relevant entries from openspec/benchmark.md. -->
<!-- Format: Principle N from benchmark.md - one sentence on how it applies here. -->
<!-- Do not copy the full benchmark; summarize the constraint. -->

| Principle | Source (benchmark.md §) | Application to this change |
|---|---|---|
| <!-- e.g. One hero element --> | <!-- e.g. Principle 1 --> | <!-- e.g. Glucose value owns 40% screen height --> |

---

## Layout Variants

<!-- Define 2-3 named variants. Each explores a different visual hierarchy. -->

### Variant A: <!-- Name -->

**Who:** <!-- one sentence describing the target user mindset -->
**Hero:** <!-- the element that dominates -->

```
<!-- ASCII zone diagram. Use named zones: status-bar / hero / meta / separator / graph / footer -->
┌──────────────────────────────────┐
│  STATUS BAR                      │
├──────────────────────────────────┤
│                                  │
│  HERO                            │
│                                  │
├──────────────────────────────────┤
│  META                            │
├──────────────────────────────────┤
│  FOOTER                          │
└──────────────────────────────────┘
```

| Zone | Purpose | Key element |
|---|---|---|
| status-bar | <!-- role --> | <!-- element --> |
| hero | <!-- role --> | <!-- element --> |
| meta | <!-- role --> | <!-- element --> |
| footer | <!-- role --> | <!-- element --> |

---

### Variant B: <!-- Name -->

**Who:** <!-- one sentence -->
**Hero:** <!-- element -->

```
<!-- ASCII diagram -->
```

| Zone | Purpose | Key element |
|---|---|---|
<!-- rows -->

---

<!-- Add Variant C if needed. Delete this block otherwise. -->

---

## Design Tool Link

<!-- Use the Figma MCP (use_figma tool) to generate frames for each variant. -->
<!-- PenPot MCP is read-only - it cannot create designs. Use Figma. -->
<!-- Invoke the figma-use skill before calling use_figma. -->
<!-- One frame per variant per platform (Time 2 200x228 / Round 2 260x260). -->

**Figma Link:** <!-- paste URL here -->

---

## Decision

<!-- State which variant is selected and why. -->
<!-- This becomes the direct input for layout.md. -->

**Selected variant:** <!-- name -->
**Reason:** <!-- one sentence -->
**Deferred variants:** <!-- names - why not chosen -->

---

## Open Questions

<!-- Visual decisions left to the PenPot phase. -->
<!-- Note: who decides and when (before layout.md is written). -->

- <!-- question: e.g. battery indicator style (dots vs bars vs percentage) -->