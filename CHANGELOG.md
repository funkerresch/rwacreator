# Changelog

## Update UI strings for save/export/open actions

Rename menu items and dialog titles to clarify RWA and Sharing Server
terminology (e.g. Save Version as..., Copy Project to..., Send Project to
Sharing Server..., Export Project for transfer to RWA Player..., Save RWA
File, Open RWA File, File Path Preferences). Update preference labels
(Sharing Server Path, Project Export Path) and toolbar tooltip.

Also renamed client associated download/export functions/properties
to reflect update UI strings. The cutoff is keys in settings, as those
should be retained for users.

## Add dialog title and reduce widths

Enhance RwaInputDialog to accept a title parameter
for better context in file path preferences.

## Engine parity

- send gain when in sendInitValue2pd

## Scene Switch

Improved scene transition handling in setScene

- always end background assets when switching scenes
- release the current state's block latch
- switch scene
- explicitly handle fallback activation:
  - fallback enabled: the scene's fallback state
  - fallback disabled:  active assets keep playing until
    a new state is triggered
- activate background state

Updated `receiveLastTouchedScene()` to prevent the signal 
`sendSelectedScene` from echoing into RwaRuntime::setScene(),
where a previously set state would be again unset.

### Changed
- `RwaRuntime::setScene`: active assets of the previous state are now only
  ended when the new scene activates a fallback state that contains assets.
  A scene with fallback disabled — or with a silent (asset-less) fallback —
  lets running assets play out until a new state is triggered, instead of
  cutting to silence on the scene transition. Note: this diverges from the
  RWA Player's historical behavior (which always cut active assets on scene
  change) and must be mirrored there for engine parity.

### Fixed
- `RwaRuntime::setScene` no longer invokes undefined behavior when a scene
  has fallback enabled but no states: it logs a warning and leaves the
  entity without a current state (same as fallback-disabled), instead of
  calling front() on an empty state list.

### Documentation
- Documented `setScene`'s contract (post-call state per fallback setting,
  audio-continuity rule, preconditions) and clarified that the latch
  release on the old state exists because the per-tick geographic unblock
  only scans the current scene.
