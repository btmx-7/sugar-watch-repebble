# display Specification

## Purpose
TBD - created by archiving change fix-simple-gaps. Update Purpose after archive.
## Requirements
### Requirement: Negative mmol/L Delta

The delta value in mmol/L mode MUST render with the correct sign and magnitude for all signed integer inputs.

#### Scenario: Negative delta in mmol/L mode

- **WHEN** `s_delta` is -8 mg/dL and `s_use_mmol` is true
- **THEN** the delta buffer contains "-0.4"
- **AND** does not contain "0.-4"

#### Scenario: Positive delta in mmol/L mode

- **WHEN** `s_delta` is +8 mg/dL and `s_use_mmol` is true
- **THEN** the delta buffer contains "+0.4"

#### Scenario: Zero delta in mmol/L mode

- **WHEN** `s_delta` is 0 and `s_use_mmol` is true
- **THEN** the delta buffer contains "+0.0"

