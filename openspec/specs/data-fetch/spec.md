# data-fetch Specification

## Purpose
TBD - created by archiving change fix-simple-gaps. Update Purpose after archive.
## Requirements
### Requirement: lastRead Timestamp Fallback

When a Nightscout entry lacks a `date` field (or it is 0/falsy), the timestamp MUST be derived from `dateString` without producing NaN.

#### Scenario: Entry has `date` field

- **WHEN** `latest.date` is a non-zero Unix millisecond timestamp
- **THEN** `lastRead` equals `Math.round(latest.date / 1000)`

#### Scenario: Entry missing `date` field

- **WHEN** `latest.date` is absent or 0
- **AND** `latest.dateString` is an ISO 8601 string (e.g. "2026-04-05T10:30:00.000Z")
- **THEN** `lastRead` equals `Math.round(new Date(latest.dateString).getTime() / 1000)`
- **AND** `lastRead` is a valid integer (not NaN)

