# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Fixed

- The Playback Mode of Pd-patch assets now determines how many channels of
  spatial data the patch receives. Previously the runtime ignored the playback
  mode for patch assets entirely and always sent a single data set
  (`$0-distance1`, `$0-azimuth1`, `$0-elevation1`), so multichannel patches
  could never be driven from the Creator. Now the mode selects the channel count
  (binaural stereo → 2, binaural 5 channel → 5, binaural 7 channel → 7, all
  mono-ish modes → 1) and the runtime sends
  `$0-distanceN`/`$0-azimuthN`/`$0-elevationN` per channel, honoring channel
  radius, rotate offset and custom channel positions exactly like for audio
  assets. Patches that only listen to the `...1` receivers are unaffected;
  "Headtracker relative to source" off still sends the raw head
  azimuth/elevation on channel 1 only, as before. The per-playback-type fan-out
  in `RwaRuntime::sendData2Asset` was unified into a single loop over
  `RwaAsset1::channelCountForPlaybackType`; behaviour for audio-file assets is
  unchanged (including AUTO/NATIVE modes not sending spatial data, a
  pre-existing issue left as is for now). **Engine parity:** the same change
  needs to be mirrored in the Player's `RwaGameLoop.swift` (its `sendData2Asset`
  has the identical single-channel Pd branch); until then, exported games with
  multichannel patch assets will behave differently on the Player.

- Binaural 7-channel assets no longer collapse all seven channels onto one point
  during simulation. The engine's per-tick channel placement
  (`RwaRuntime::getOffsetForChannel`) had no case for
  `RWAPLAYBACKTYPE_BINAURAL7CHANNEL_FABIAN` and returned offset 0 for every
  channel, so the authored spread (−40/0/40/−80/80/−120/120°, which
  `RwaAsset1::calculateChannelPositions` used for the initial map display) was
  overwritten with identical coordinates on the first tick. The three
  independent copies of the channel-angle tables (runtime, initial placement,
  map channel handles in `RwaGraphicsView`) are now one:
  `RwaAsset1::channelOffsetForPlaybackType` + `channelCountForPlaybackType` +
  `playbackTypeHasChannelPositions` replace `getOffsetForChannel` and the
  per-mode if-chains in `calculateChannelPositions` and the map view. Map
  behaviour is unchanged (channel handles for the binaural modes when channel
  radius > 0; none for mono/stereo/custom, whose radius the engine ignores).
  **Engine parity:** the Player mirrors the same 7-channel gap in
  `RwaGameLoop.swift` (`getOffsetForChannel`); to be fixed together with the
  multichannel-patch mirror.

### Added

- `tools/trace/pdmodes/` + `tools/trace/scenarios/pdmodes.scenario.json`:
  checked-in regression fixture for the two engine changes above — four dummy Pd
  patches (binaural stereo, binaural mono, binaural stereo with "headtracker
  relative to source" off, binaural 7 channel) whose expected per-channel
  `azimuthN`/`distanceN`/`elevationN` sends are documented in
  `tools/trace/README.md`.

## [v1.4.2] - 2026-08-07

### Changed

- The selected asset's auxiliary map icons (channel positions, the start
  position and the current position of moving/rotating assets) now take on the
  selection color (the orange of the selected-asset icon) in the Map View and
  State Map View. Previously only the asset's location icon changed on
  selection, so with several multichannel or moving assets in view it was hard
  to tell which satellite icons belonged to the selected asset. The colored
  variants are rendered from the same SVGs by substituting the fill color
  (`rwaRenderRecoloredSvg`, which gained a `fromColor` parameter); no new icon
  files were added.

### Fixed

- Debug builds are now signed with the Developer ID identity (`TEAM_ID` from
  `.env`, same as release builds) instead of ad-hoc, falling back to ad-hoc
  with a warning when no identity is available. The macOS application
  firewall identifies apps by code signature and its "automatically allow
  downloaded signed software" option only covers identified-developer
  signatures, so an ad-hoc debug build could never be durably allowed —
  every rebuild produced a new signature, and once the allow/deny prompt
  stopped appearing (e.g. after an MDM policy sync rewrote the firewall
  rules), incoming OSC from RWA Players on UDP :8000 was silently dropped
  while loopback traffic kept working. `debug.entitlements`
  (`get-task-allow`) is still applied, so lldb can attach as before.

- Three asset attribute checkboxes ("Raw Sensors to Pd", "GPS to Pd",
  "Headtracker relative to source") no longer displayed the asset's actual
  value when an asset was selected. Most visibly, "Headtracker relative to
  source" showed unchecked although the field defaults to enabled. Attribute
  widgets are found by their label string (`findChild` on the `objectName` set
  in `addAttrCheckbox`), and the label update in `fb11ef7` renamed only the
  lookup strings in `setCurrentAsset`, not the labels the checkboxes are
  created with, so the lookups silently returned null. The constructor labels
  now match the lookups. Stored values and export were never affected; only
  the display was stale.

### Removed

- The "Required Scenes" field in the Scene view. It was scaffolding for a
  never-implemented scene-entry condition: `RwaScene::requiredScenes` is not
  serialised to the `.rwa` and is evaluated by neither engine, so anything a
  creator entered was silently lost on save. Its display code was additionally
  broken (wrong widget lookup, read the last-touched state's required states
  instead of the scene's) and had never executed. The field is commented out,
  not deleted; what a real implementation needs (visited-scene tracking, a
  gate in `setEntityScene`, serialisation and Player parity) is recorded in
  `docs/planned-features.md`. No existing game is affected: the value was
  never written to disk, and the example corpus uses neither this field nor
  the (working) state-level Required States.

## [v1.4.1] - 2026-08-04

### Fixed

- No microphone input in release builds (Pd patches using `[adc~]` received no
  audio input). The release build signs with the hardened runtime (`--options
  runtime`) but passed no entitlements, and the hardened runtime requires
  `com.apple.security.device.audio-input` for audio capture. New
  `release.entitlements` carries the entitlement and is applied when signing the
  app bundle in `build_release.sh`; the first `Pa_OpenStream` after launch now
  triggers the microphone permission prompt as intended. Debug builds were never
  affected (ad-hoc signed without the hardened runtime).

  `release.entitlements` also documents the App-Sandbox entitlements for
  Bluetooth (headtracker) and network client/server (map tiles, OSC, the
  project-sharing HTTP server). These are inert under the hardened runtime
  alone, kept so intent is recorded should the app ever be sandboxed.

## [v1.4.0] - 2026-08-03

### Added

- Status badges in the State View's asset list: small icons painted
  right-aligned over the asset name, showing at a glance what would otherwise
  require opening the asset's attribute view. An item's tooltip lists the
  meaning of its badges.

  Badges, right to left:

  - **Sample rate mismatch**: the audio file's sample rate differs from the
    project rate (48 kHz). The file's rate is read via TagLib when an asset is
    added; for assets loaded from an existing `.rwa` it is read once on first
    display and cached (new transient `RwaAsset1::originalSampleRate`, not
    serialised to the `.rwa`).
  - **Pd patch**: shown for Pd assets; plain audio assets get no type badge.
  - **Playback type**: speaker icon with channel count for mono/stereo,
    headphones icon with channel count for the binaural 1/2/5/7-channel modes
    (legacy and Fabian variants share one icon). Undetermined, Auto/Native,
    Binaural-Space and the Custom IR sets show no badge.
  - **Muted**, **Looped**, **Moving** (one icon for moving and auto-rotating).

  Implementation: new `RwaListBadgeDelegate` (`rwalistbadgedelegate.{h,cpp}`),
  installed on the shared `RwaListView` base class. Which badges an item shows
  is a `QStringList` of icon basenames under `RwaListBadgeDelegate::BadgeRole`;
  unknown names are skipped. The game and scene lists inherit the delegate and
  only need to fill that role to get badges of their own (planned follow-up).
  The icons' neutral gray (`#434343`) is replaced with the palette's text color
  in dark mode and with the highlighted-text color on selected rows
  (`QSvgRenderer` on the rewritten SVG bytes, cached per name/color/size); in
  light mode the authored gray is kept (the palette color would be plain black).
  Semantic colors like the mismatch red are left as authored. New icons in
  `images/`: `badgeSamplerateMismatch`, `badgePd`, `badgeMuted`, `badgeLooped`,
  `badgeMoving`, `playbackSpeaker1/2`, `playbackHeadphones1/2/5/7` (Material
  Symbols, documented in the rwa-doc icon translation table).

- `.rwa` is now a registered macOS document type owned by RWA Creator. Opening
  an `.rwa` file in Finder opens the project in the running instance if the app
  is already open, otherwise it launches the app first. `.rwa` files get their
  own document icon in Finder and RWA Creator appears as the default "Open With"
  handler.

  Details of the implementation:

  - `Info.plist.in` exports the new UTI `com.fhnw.rwa.document`
    (`UTExportedTypeDeclarations`) and claims it (`CFBundleDocumentTypes`, role
    *Editor*, rank *Owner*). The type conforms to `public.xml`, so Quick Look
    text preview and Spotlight indexing of `.rwa` files work for free.
  - macOS never passes the double-clicked path via `argv`; it arrives as a
    `QFileOpenEvent`. The new `RwaApplication` (QApplication subclass,
    `rwaapplication.h`) catches it and queues requests that arrive before the
    main window is ready. On Windows/Linux, `main()` now opens a project passed
    as the first command-line argument.
  - Opening via Finder replaces the currently loaded project, exactly the
    behaviour of File > Open (which does not prompt for unsaved changes either).
  - New document icon `images/rwa-document.icns`, copied into `Contents/Resources` by CMake.

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

- Swapped the sequence of the coordinate fields in the State View attribute
  list, for consistent latitute/longitude sequence.

- The Map View toolbar icons now follow the system's light/dark appearance: in
  dark mode the SVGs' neutral gray (`#434343`) is replaced with the palette's
  button-text color at paint time, so the icons stay visible on a dark toolbar;
  disabled buttons use the disabled color group. In light mode the authored gray
  is kept, and semantic colors (the green start, red stop, blue asset icon) are
  left as authored everywhere. Implemented as a custom `QIconEngine`
  (`rwathemedicon.{h,cpp}`); the list badge delegate shares the same renderer.
  Icons drawn onto the map are deliberately untouched: the map background does
  not change with the theme. The one remaining `.png` toolbar icon
  (`heroFollowsSceneAndStateButton`) cannot be recolored this way.

- Saved `.rwa` files now reference assets as `assets/<filename>` instead of the
  absolute path of the machine they were saved on. The Creator
  (`rwaimport.cpp`), the Player (`RwaImport.swift`) and `validate_rwa.py`, take
  only the basename of the `url` attribute and resolves it against the `assets/`
  folder next to the `.rwa` file, which is why projects always opened fine on
  other machines despite the paths. Undo snapshots use the same writer and now
  carry the relative form as well. The new form says what actually happens and
  keeps usernames and directory layouts out of shared game files.

  Compatibility is unchanged in both directions: old files load as before, and
  files saved with this version load in older Creators and in the Player, since
  all of them discard everything but the filename.

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

- The Log View now follows the system's light/dark appearance. Its monospace
  font was set via a stylesheet, and any stylesheet moves a widget from the
  native style to `QStyleSheetStyle`, which stops tracking palette switches.
  The font is now set with `QFont` (with a monospace style hint as fallback
  should Andale Mono be missing) and no stylesheet remains on the widget.

- Clicking empty space in the state or asset lists no longer clears the
  selection. Previously the click deselected the last touched item while the
  attribute form next to the list allowed changing of values. The empty-space
  click is now simply ignored (`RwaListView::mousePressEvent`, shared by all
  three lists).

- The asset attribute form no longer shows stale values after switching states.
  Previously it kept displaying the previous state's asset when the newly
  selected state had no touched asset yet; edits then either went nowhere or —
  when the new state contained an audio file of the same name — silently landed
  in that other asset whose values were never shown. Now touching a state
  defaults its touched asset to the first asset before the change is broadcast
  (`RwaBackend::receiveLastTouchedState`), the asset list re-announces its
  (possibly empty) selection on every state switch, and the form clears and
  disables itself when the state has no assets
  (`RwaAssetAttributeView::clearForm`).

  The form's widgets fire their edit handlers on programmatic changes too
  (`textChanged`, `idToggled`), so filling or clearing the form could write the
  displayed values back into whatever assets were still listed as selected. A
  new `RwaAttributeView::updatingForm` guard suppresses write-back while the
  form is being populated or cleared. The state and scene attribute views share
  the helpers with the same hazard and still need the equivalent guard
  (follow-up).

- Changing Loop, Rotate or the Playback Mode in the asset attribute view now
  re-broadcasts the state (`sendCurrentState`), like Mute and Automove already
  did. Before, other views were not notified of these changes; visible now that
  the asset list shows badges for them.

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
