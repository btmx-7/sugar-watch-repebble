# Spec: Settings

## ADDED Requirements

### Requirement: Maintainable Config URL

The Pebble config page URL MUST be defined as a named constant, not inline, so it is easy to update before publishing.

#### Scenario: Config URL is a constant

- **WHEN** `showConfiguration` fires
- **THEN** the URL is built from a `CONFIG_URL` constant defined at the top of index.js
- **AND** the constant has a comment explaining it must be updated to the hosted URL before publishing

### Requirement: Dexcom Username Persistence

The Dexcom username (not password) MUST be pre-populated when the user reopens settings.

#### Scenario: Reopen settings with Dexcom source configured

- **WHEN** `dexcomUser` is stored in settings
- **AND** the user taps "Settings" on the phone
- **THEN** the Dexcom Username field shows the stored value
- **AND** the Dexcom Password field is blank (intentional: passwords don't repopulate)

## ADDED Requirements

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
