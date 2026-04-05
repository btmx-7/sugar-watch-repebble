# settings Specification

## Purpose
TBD - created by archiving change fix-simple-gaps. Update Purpose after archive.
## Requirements
### Requirement: Graph Window Setting

The settings page MUST expose a Graph Window selector so the user can choose how many hours of glucose history the graph displays.

#### Scenario: User selects graph window

- **WHEN** the user opens settings
- **THEN** a "Graph Window" select shows options: 1 hour (13 points), 2 hours (25 points), 3 hours (37 points)
- **AND** the current value is pre-selected

#### Scenario: Graph window saved and applied

- **WHEN** the user saves settings with "2 hours" selected
- **THEN** `settings.graphWindow` is "25"
- **AND** the next Nightscout fetch requests `count=25`

