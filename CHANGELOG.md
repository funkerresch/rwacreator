# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- Key commands for the operations a creator repeats all day.

  | Key command | Action | Menu |
  | --- | --- | --- |
  | Cmd-N | New | File |
  | Cmd-O | Open | File |
  | Cmd-S | Save | File (was a hidden shortcut before, now shown in the menu) |
  | Cmd-Shift-S | Save Version as… | File |
  | Cmd-Opt-S | Copy Project to… | File |
  | Cmd-E | Export Project for transfer to RWA Player... | File |
  | Cmd-Shift-E | Send Project to Sharing Server... | File |
  | Cmd-R | Run Simulation, restarts a running one | Simulation |
  | Cmd-K | Stop Simulation | Simulation |
  | Cmd-Shift-L | Clear Log Window | View |

- **Simulation** menu with *Run Simulation* and *Stop Simulation*, doing the same
  as the start/stop buttons in the Map View toolbar - the only place from which
  the simulation could be run until now, so there was nothing a key command could
  hang on.

  Cmd-R means "run from the top": on a running simulation it restarts it, taking
  one keystroke for what the toolbar takes two clicks. The menu entry is called
  *Restart Simulation* while the simulation runs, and *Stop Simulation* is greyed
  out while it does not.

- **Clear Log Window** in the View menu, the menu counterpart of the log view's
  *clear* button.

- The sharing server now logs what connected players do, so a creator can see
  whether a phone reached RWA Creator at all and which game it pulled. Two lines
  per request — the attempt as it arrives and the outcome once answered:

  ```
  Player 192.168.1.42 requests GET /My Soundwalk.zip
  Player 192.168.1.42 got /My Soundwalk.zip (200, 24.3 MiB)
  Player 192.168.1.42 could not get /Typo.zip (404)
  ```

  The arrival line is separate on purpose: a game of that size takes a while to
  transfer and would otherwise only show up once it is through. Starting the
  server also logs its port and the directory it serves.

  Note that httplib calls its logger even when writing the response failed, and
  the status is by then already sent — a player who walks out of wifi mid-download
  is logged as a completed transfer.

- **Rescan Audio Devices** in the Audio Preferences menu. It re-enumerates the
  audio hardware and rebuilds the menu, so a headset connected after the app was
  started can be used without restarting RWA Creator. The scan also runs
  automatically whenever the simulation is started, so in practice pressing play
  is enough.

- **Follow System Default (…)** at the top of both device lists in the Audio
  Preferences menu, naming the device it currently resolves to. It is the state
  RWA Creator starts in: macOS switches its default output to a headset when one
  is connected, and following that is what a creator plugging in headphones
  expects. Picking a device from the list opts out of this for that direction
  until the entry is selected again — the pick is then remembered by name and
  wins over the default whenever it is present, including after it was unplugged
  and reconnected.

  Both selections survive a restart (`audiooutputdevice` / `audioinputdevice` in
  the settings). They are stored by name, not by index: PortAudio hands out
  indices in whatever order it enumerates the hardware, so they mean nothing in
  the next session. A stored device that is not connected at startup stays
  remembered — RWA Creator runs on the system default until it appears.

### Changed

- The File menu entry **Clear** is now called **New**: It clears out the current
  project (without saving) and opens a file dialogue (now correctly titled
  "New RWA Project" instead of "Copy entire RWA Project Folder") to save the new project.
  The first save of a game that was never written to disk opens the same dialogue as
  "Save RWA Project". "Copy Project to..." keeps its title. All three still write a
  complete project folder, only the title of the dialogue differs.

- The start button in the Map View toolbar now follows the state of the
  simulation instead of only its own clicks — otherwise Cmd-R and Cmd-K would
  start and stop the simulation with the button not moving at all. The state
  comes from `RwaSimulator` itself, which announces every start and stop
  (`sendSimulationRunningChanged`), so several open Map Views agree as well.

  This also fixes the button staying pressed after a simulation was stopped by
  something other than a click on it: rescanning the audio devices or picking an
  input/output device stops the simulation, and the toolbar kept claiming it was
  running.

### Fixed

- Connecting or removing an audio device while RWA Creator was running left the
  app deaf: the simulation ran but no sound came out, and the Audio Preferences
  menu still listed the devices from application start. PortAudio enumerates the
  devices once in `Pa_Initialize()` and never updates that list, so the stored
  device index pointed at whatever now sits at that index. The menu was built
  once at startup and never rebuilt either. A rescan now restarts PortAudio
  (`paWrapper::rescanDevices()`), which is the only way to pick up the new device
  list, and repopulates the menu. Selections are tracked by device name rather
  than by index, since the indices are reused: on the machine this was developed
  on, index 2 was "Externe Kopfhörer" with the headset plugged in and
  "MacBook Pro-Lautsprecher" without it.

  Two things that made this hard to diagnose are fixed as well: a failing
  `Pa_OpenStream()` was silently discarded when the simulation started, and is
  now logged with the device name and a pointer to the rescan; and
  `paWrapper::getDeviceName()` returned a dangling pointer into a destroyed
  temporary (it now returns a `QString`).

- `build_debug.sh` aborted before signing the bundle when a Homebrew CppUnit is
  installed: taglib then builds its own test suite, which links against an
  arm64-only library and fails the x86_64 half of the universal build. The
  vendored libraries' test suites are now off (`BUILD_TESTING`).

## [v1.3.1] - 2026-07-30

### Fixed

- The FABIAN HRTF set no longer has to be copied into every project.
  `[rwa_binauralsimple~ 256 fabian_dir256.txt]` resolved its IR file only next
  to the patch that instantiated it: the bundled playback patches worked because
  they sit beside `fabian_dir256.txt` in `Resources/puredata`, while a creator's
  own Pd patcher — opened from the game's `assets/` folder — did not, unless the
  38 MB file was copied in by hand. Two changes:
  - `vas_library` (`vas_pdmaxobject_read`) now resolves the IR through
    `open_via_path`: absolute path, then the patch's own directory, then Pd's
    global search path. A missing file is reported by name instead of being
    passed down as a bad path, and the path is bounded (`fullpath` is 512 bytes,
    `MAXPDSTRING` is 1000).
  - `RwaRuntime` registers the bundled `puredata` directory with
    `libpd_add_to_search_path()`, so any patch resolves the HRTF set from there.

  Existing games that carry their own copy keep working — the patch directory is
  still searched first. **Engine parity**: mirrored in the Player, which adds
  `Bundle.main.resourcePath` to the search path in `RwaGameLoop.init`; both apps
  build the same `vas_library` sources, so the external side is shared.

### Changed

- `vas_library` now tracks `rnd-hsm-klassik/vas_library` (branch
  `rwa-player-fixes`) instead of `funkerresch/vas_library`, the same fork the
  Player already used — one lineage for both apps. The fork additionally carries
  null-terminator fixes in three `vas_mem_alloc` calls, `pd_error` instead of the
  deprecated `error`, and drops its stale bundled `m_pd.h` copies (Pd headers now
  come from `libpd/pure-data/src`).

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
