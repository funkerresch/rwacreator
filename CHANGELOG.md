# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [v1.3.0] - 2026-07-29

**Engine parity updates**: align game engine behaviour between RWA Creator and RWA Player

### Added

- Added `rwatrace`, a headless, deterministic runtime trace harness for
  differential testing against RWA Player, including a smoke scenario and
  runtime investigation documentation.
- Add context (filename, function, line) to log output in release builds.
- Sent asset gain to Pure Data when an asset is initialized (`sendInitValue2pd`)

### Changed

- Improved scene transition handling in `setScene`: **Important**: this diverges from the RWA Player's historical behavior (which always cut active assets on scene change) and must be mirrored there for engine parity.
  - always end background assets when switching scenes
  - release the current state's block latch
  - switch scene
  - explicitly handle fallback activation: fallback enabled: the scene's fallback state;
    fallback disabled:  active assets keep playing until a new state is triggered.
  - activate background state
- Updated the simulator's RWA Player OSC target port to 8001.
- Aligned runtime logging with RWA Player, including timestamps and scoped Qt
  logging.
- Streamline Pd print logging: Pd delivers a print one fragment at a time
  ("print", ": ", "1", " ", "2", "\n"), leading to lines difficult to read
  (every symbol became its own log entry plus a trailing empty one). Now routing
  the print hook through libpd's own concatenator so printpd receives one
  complete line. Also drop QDebug's quoting/escaping of the line (noquote) and
  prefix Pd output with `[pd]`. Long lines are elided at 500 chars because the
  log view discards messages over 512.

### Fixed

- sendData2activeAssets: TODO
- Background state now initialises correctly and assets receive updates,
  allowing for panning and distance attenuation without activating another state before.
- `RwaRuntime::setScene` no longer invokes undefined behavior when a scene
  has fallback enabled but no states: it logs a warning and leaves the
  entity without a current state (same as fallback-disabled), instead of
  calling front() on an empty state list.
- Prevent scene-selection signals from clearing the current state after a
  scene transition: Updated `receiveLastTouchedScene()` to prevent the signal 
  `sendSelectedScene` from echoing into RwaRuntime::setScene(), where a previously set state would be again unset.

### Documentation

- Documented runtime asset fields and the engine's scene/state transition and
  audio-continuity behavior.

## [v1.2.6] - 2026-07-23

### Changed

- `RwaRuntime::setScene`: active assets of the previous state are now only
  ended when the new scene activates a fallback state that contains assets.
  A scene with fallback disabled — or with a silent (asset-less) fallback —
  lets running assets play out until a new state is triggered, instead of
  cutting to silence on the scene transition. Note: this diverges from the
  RWA Player's historical behavior (which always cut active assets on scene
  change) and must be mirrored there for engine parity.

**Update UI strings for save/export/open actions**

Rename menu items and dialog titles to clarify RWA and Sharing Server
terminology (e.g. Save Version as..., Copy Project to..., Send Project to
Sharing Server..., Export Project for transfer to RWA Player..., Save RWA
File, Open RWA File, File Path Preferences). Update preference labels
(Sharing Server Path, Project Export Path) and toolbar tooltip.

Also renamed client associated download/export functions/properties
to reflect update UI strings. The cutoff is keys in settings, as those
should be retained for users.

**Add dialog title and reduce widths**

Enhance RwaInputDialog to accept a title parameter
for better context in file path preferences.

**Engine parity**

- send gain when in sendInitValue2pd

**Scene Switch**

Improved scene transition handling in setScene

- always end background assets when switching scenes
- release the current state's block latch
- switch scene
- explicitly handle fallback activation:
  - fallback enabled: the scene's fallback state
  - fallback disabled:  active assets keep playing until
    a new state is triggered
- activate background state

### Fixed

- `RwaRuntime::setScene` no longer invokes undefined behavior when a scene
  has fallback enabled but no states: it logs a warning and leaves the
  entity without a current state (same as fallback-disabled), instead of
  calling front() on an empty state list.
- Updated `receiveLastTouchedScene()` to prevent the signal `sendSelectedScene`
  from echoing into RwaRuntime::setScene(), where a previously set state would
  be again unset.

### Documentation

- Documented `setScene`'s contract (post-call state per fallback setting,
  audio-continuity rule, preconditions) and clarified that the latch
  release on the old state exists because the per-tick geographic unblock
  only scans the current scene.
