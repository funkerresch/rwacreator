# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Changed

- Pooled mono- and stereo playback Pd patches now apply equal-power crossfades
  when looping.

### Fixed

- Rewrite of mono- and stereo playback Pd patches: Through the investigations
  for previous bugs a few new ones became visible:

  - Crossfade and Fade-in were applied simultaneously at the beginning of
    playback. Fixed by separating the initial start bang from the activation of
    the crossfade metro. This behaviour sounds different as before, but it now
    reflects what the attributes actually say.
  
  - Replaced crossfade-controllers by a single writer, one `[line~]` for less
    zipper noise, and fixing the double-crossfade bug.
  
  - After fixing this, the next bug became audible: Exponential distance-scaling
    produced a click in the beginning of playback, through `[line]` jumping from
    0 to the first distance value. The exponential transformation yields one
    frame at unity gain, then the first actual distance-related value. Fixed by
    interpolating from a far distance (1000) to the actual distance, resulting
    an a short crossfade. That might need to be fixed for longer
    `$0-smoothdistance` values. Also, linear distance scaling produced a
    division-by-zero for the same reasons.
  
  - Both patches didn't receive `$0-smoothdistance`, and defaulted to 10 ms
    ramps. Now they accept the attribute they now do.

  - Thinned out some of the redundant left/right scaling further down the dsp
    chain.

## [v1.4.8] - 2026-08-14

### Fixed

- Background assets of a left scene no longer keep playing forever. Two
  cooperating runtime defects, both reproduced headlessly with `rwatrace`:

  - Background assets of a left scene no longer keep playing forever: A background
    asset that is still fading out stays in `backgroundAssets` until Pd reports
    `-playfinished`. Re-entering its scene within that window made
    `startBackgroundState` start a *second* patch instance and then silently fail
    to track it (`std::map::insert` keeps the old entry), so the new instance
    never received `-end`, never got per-tick updates, and was never released,
    causing it to loop forever. The fading instance's patcher is now parked on a
    pending-release list (freed on its `-playfinished`) and the new instance takes
    over the map slot, giving a clean crossfade on quick re-entries. Mirrored in
    the Player, which had the same flaw with different fallout (dictionary
    assignment overwrote the entry: no eternal audio, but the fading patcher
    leaked as busy).
  
  - `RwaRuntime::setEntityScene` switched to any other same-level scene whose
    area contains the hero, even while the hero was still inside the current
    scene's area, causing scene ping-pong at tick rate. Where two scene areas
    overlap (e.g. Ufer > klybeckschlosszug in the H.E.I. Guide Klybeck harbor
    game), standing in the overlap alternated the scene on every tick (82
    switches in 2 s in the trace), each switch ending and restarting the
    background states and burning through the whole 40-patcher pool within a
    second. The runtime now stays in the current scene as long as the hero is
    within its area (exit-offset applies); another scene is only entered after
    the current one has actually been left. Mirrored in the Player's
    `RwaGameLoop.setEntityScene` (parity).

- State checks no longer run against the previous scene right after a scene
  switch. `setEntityState` cached the scene before `setEntityScene` and kept
  using it for the state-entry loop of the same tick, so a GPS state of the
  left scene could be re-activated immediately after switching (its assets
  then played into the new scene). The Swift engine already re-reads the scene
  at this point; the Creator now does the same (parity restored).

- Releasing a mono (WAV) asset's patcher freed the wrong pool slot.
  `releasePatcherFromItem` looked the tag up with `getStereoPatcherIndex`, which
  returns -1 for a mono tag: an out-of-bounds write (`monoPatchers[-1].isBusy`)
  and the actual mono patcher stayed busy forever, exhausting the mono pool over
  a session.

### Added

- `rwatrace` scenarios can now address a specific patch instance:
  `{"playFinished": {"tag": 1002}}` injects the finish for an older instance of
  an asset that has been restarted since (the name lookup always resolves to the
  newest instance).

## [v1.4.7] - 2026-08-13

### Fixed

- **Pasting states into another project now copies the referenced asset files
  into that project's `assets/` folder.** The state clipboard survives switching
  projects, but pasting only ever copied the in-memory asset objects, the
  audio/Pd files stayed in the source project. Playback kept working for the
  rest of the session (the in-memory path still pointed at the source project),
  and broke on the next load: both importers (Creator and Player) have always
  resolved the `url` attribute's basename against the *current* project's
  `assets/` folder, where the file never existed. This predates the v1.4.5
  relative-`url` change, which only removed the stale absolute source path from
  the saved file, not the (already absent) file copy.
  `RwaBackend::pasteStatesFromClipboard` now rebases every pasted asset onto the
  current project and copies the file if it isn't already there:
  - A same-named file with *identical* content is reused (the common case when
    both projects draw on the shared audio library).
  - A same-named file with *different* content is not overwritten; the incoming
    file is copied as `name-2.ext` (`-3`, ... - reusing an identical copy from
    an earlier paste instead of stacking duplicates) and the pasted asset is
    renamed to match. This still can cause inconsistencies, i.e. missing files
    in dynamic Pd patches that reference specific filenames of assets!
  - If the source file has disappeared (e.g. deleted in the source project since
    copying), the paste still succeeds; a warning is logged and the asset shows
    the missing-file badge introduced in v1.4.5. Pasting within the same project
    is unaffected (source and target path are equal, nothing is copied). No
    runtime change, no engine-parity impact.

- **Crash when clicking a state pasted from another project.**
  `RwaState::copyAttributes` deep-copies the assets but copied the raw
  `lastTouchedAsset` pointer, so a clipboard state kept pointing into the
  *source* project's assets, which are freed when another project is opened.
  Selecting the pasted state made the asset list dereference the freed asset
  (`RwaAssetList::setCurrentState` → `setCurrentAsset` →
  `RwaAsset1::getFileName`), reading a garbage string length and aborting on the
  failed allocation. `lastTouchedAsset` is now remapped onto the corresponding
  asset *copy* during `copyAttributes` (nullptr when the source had none),
  which also makes within-project duplicates (Duplicate scene, State from
  current) self-contained instead of pointing at their sibling's assets. The
  stale `myScene` copy is harmless: the paste/duplicate paths overwrite it via
  `setScene`, and `nextState`/`nextScene`/`hintState` are name strings, not
  pointers.

- **Scene and state deletion is now refused while the simulation is running**,
  with a warning, matching the v1.4.6 asset-delete guard. The runtime's entity
  holds raw `currentScene`/`currentState` pointers during simulation, so
  deleting either mid-simulation left the tick loop dereferencing freed memory.
  Guards live in the view/backend delete paths (`RwaSceneView::deleteState`,
  `RwaBackend::removeScene`); no runtime change, no engine-parity impact. A
  refused delete writes no undo snapshot.

- **Scene and state lists no longer remove rows behind the model's back** (same
  family as the v1.4.6 asset-list fix). Both Backspace handlers called
  `takeItem` on the widget and *then* asked the model to delete, so any refusal
  or early return on the model side left a row missing from the list while the
  object stayed in the game:
  - Deleting the last remaining scene is refused by `RwaBackend::removeScene`
    ("keep always one scene"), but the scene list had already dropped the row -
    the only scene vanished from the list while staying in the game, and a no-op
    "Delete Scene" undo snapshot was written on top. The refusal is now logged
    as a warning and the row stays.
  - The stray `takeItem` calls are gone; row removal is now only driven by the
    model change. `RwaBackend::removeScene` emits `updateGame()` on success
    (like `appendScene`/`duplicateScene` already did), which rebuilds the scene
    list and the toolbar's scene menu; the state list was already rebuilt by the
    scene broadcast.
  - The undo snapshot for a scene delete is now written by the backend, only
    when a scene was actually removed. Previously the scene list and the toolbar
    each wrote one unconditionally, so a refused delete produced an undo step
    that appeared to do nothing. Same guard for a state delete whose name lookup
    fails.
  - After a state delete the selection lands on the FALLBACK state (the model
    re-points `lastTouchedState` to `states.front()`), which is immortal.
    Holding Backspace cannot chain-delete states the user never selected. That
    refusal is now a visible warning instead of a debug print.

- **Toolbar "Scene → Remove" left the deleted scene in the scene list.** The
  toolbar path never touched the widget and `removeScene` never triggered a
  rebuild, so the dead scene stayed listed (and selectable) until the next full
  rebuild. Fixed by the same `updateGame()` emission above.

## [v1.4.6] - 2026-08-12

### Fixed

- **Deleting an asset no longer takes a second, unrelated entry out of the asset
  list.** The Backspace handler emitted `deleteAsset`, which removes the asset
  from the model and rebuilds the whole list via the state broadcast (during
  which the backend re-points the state's `lastTouchedAsset` to
  `assets.front()`, selecting row 0), and then *also* called
  `takeItem(getSelectedIndex())`, removing row 0 of the freshly rebuilt list
  from the widget. The first-added asset (typically the Pd patch) vanished from
  view while remaining in the game, and reappeared on the next rebuild (adding
  an asset, switching states, reloading the project), perceived as "deleted
  assets come back". The stray `takeItem` is gone; the rebuild is the single
  source of truth.

- **Deleting an asset while the simulation runs is now visibly refused.** The
  handler silently returned (the runtime's active assets would keep a dangling
  pointer) but the widget still removed the row, so the entry looked deleted
  until the next rebuild or reload resurrected it. The row now stays and a
  warning is logged.

- **Undo restore / opening a project no longer re-broadcasts stale objects.**
  `RwaBackend::clearScenes()` deletes every state but kept the
  `lastTouchedScene`/`lastTouchedState`/`lastTouchedAssetItem` pointers, so the
  post-restore broadcast (`updateLastTouchedSceneStateAndAsset()`) handed the
  views freed states and a scene no longer part of the game. Subsequent edits
  landed in objects that saving never sees and were silently lost. The pointers
  are now cleared with the scenes, and the broadcast guards against an empty
  scene list.

- **"On asset delete: keep/remove ..." no longer permanently deletes the file,
  and undo brings it back.** The undo history can resurrect a deleted asset
  *entry*, but never a permanently removed file, which produced entries pointing
  at nothing. The file now moves to a session trash (`tmp/trash/` inside the
  project, same lifetime as the undo history); restoring an undo snapshot moves
  the files of resurrected entries back into `assets/` automatically and notes
  it in the Log View. When the undo history ends (quit, open, new project), the
  session trash is forwarded to the system trash as a last-resort manual
  recovery path, so the files never silently vanish. If the session trash is
  unavailable, the file goes to the system trash directly, and only if that also
  fails is it removed permanently.

### Added

- **Missing-file badge in the asset list.** An asset entry whose file is absent
  from the `assets/` folder (e.g. manually deleted, or resurrected by a session
  that wasn't saved after removing assets) now shows a corresponding badge
  instead of looking like a healthy asset.

## [v1.4.5] - 2026-08-12

### Fixed

- **A pooled patcher's pending fade-out no longer fires into the next
  simulation.** The pooled player patchers are opened once and never closed, so
  their `$0` tags - and their pending `[delay]` clocks - survive a stop. A
  patcher whose asset had already ended but was still fading out has left
  `activeAssets`, so `freeAllPatchers()` (which resets only active assets) never
  re-armed its `[delay]`: the clock stayed in Pd's clock queue with its absolute
  deadline, logical time froze with the closed stream, and on the next start the
  remaining fade time elapsed *inside the new run*, then the patch switched
  itself off and sent `<tag>-playfinished` for a tag a fresh asset could own by
  then (silent asset drop; patcher assignment is first-free, so re-acquiring the
  same tag is the common case). Verified with an instrumented
  `rwaloopplayerstereo.pd` (`print` taps on `-play`/`-end`): the `END FIRED`
  debug line consistently appeared in the run *after* the one that armed it.
  Custom Pd assets were never affected: dynamic patchers are closed on stop, and
  closing a canvas frees its objects' pending clocks with them.

  The stop routine now completes the patcher release protocol for the whole pool
  instead of leaving it half done, using only messages the protocol already
  defines (no patch-side changes): `RwaRuntime::resetAllPatchers()` sends
  `-free` / `-fadeouttime 0` / `-end` to every pooled patcher and clears the
  busy flags; `RwaSimulator::flushPdScheduler(30)` then advances Pd's scheduler
  by hand (clocks only advance inside `libpd_process_float()`, and the stream is
  already closed) so every (now zero-length) fade matures at stop; the resulting
  `-playfinished` bangs are drained on the spot, harmlessly, since
  `activeAssets` is empty and every patcher idle. The drain previously ran on a
  100 ms single-shot timer that a quick restart could push into the next
  simulation - a second, independent leak channel, now gone. The stop path also
  emits `sendSimulationRunningChanged(false)`, mirroring the start path. The
  `tools/pdtests/` patches (added here) document the underlying Pd mechanics:
  what `[switch~] 0`, `pd dsp 0` and a closed stream each do to `[line]`,
  `[line~]` and pending clocks.

- Update `vas_library` to `f77e306`: a `set` message with unresolvable arrays
  (array-loaded IRs) no longer corrupts the heap, it posts `vas_fir: <name>: no
  such array` / `... missing or empty array, filter unchanged` and keeps the
  current filter. Observed as a crash in Qt painting long after the corruption,
  triggered by a creator patch using `$0-arrayL` in *message boxes*: message-box
  `$0` never expands to the canvas id (only object-box arguments expand), so
  `soundfiler read` and `set` targeted arrays that do not exist. Details in the
  CHANGELOG of vas_library.

- Restarting a simulation no longer re-parses the HRTF filter file for every
  `[rwa_binauralsimple~]` in the game's own Pd patches. Loaded filters are
  shared through `vas_library`'s global `IRs` cache, but the externals never
  removed their cache entry when they were freed, so as a workaround
  `stopRwaSimulation()` wiped the whole cache (`vas_fir_list_clear()`) to avoid
  entries pointing at freed engines. That wipe also discarded the pooled
  patchers' entry for `fabian_dir256.txt`, so from the second run on, every
  creator patch with a file-loading binaural object parsed the 38 MB HRTF file
  again instead of sharing the filter already in memory (visible in the Log
  View: "Load Filter from File" instead of "Use existing filter"). The
  `vas_library` submodule (bumped) now deregisters an engine from the cache in
  `vas_fir_binaural_free()`, and fixes a latent bug in the list's remove
  functions that lost all subsequent nodes when the first one was removed. The
  wipe in the stop path is gone and the cache survives for the whole session.
  Details in the fork's CHANGELOG.

- Fixed crash (heap corruption, `SIGTRAP` in `free_medium`) when stopping a
  simulation whose game contained a `[vas_reverb~]` patch with array-loaded IRs
  (the externals I'm currently integrating into RWA Creator). The deterministic
  cause was a double free in the externals themselves: `vas_reverb~`,
  `vas_partconv~` and `vas_dynconv~` freed the garray buffers they had only
  borrowed via `garray_getfloatwords()`, so `libpd_closefile → garray_free`
  freed each buffer a second time. Fixed in `vas_library` (submodule bumped to
  `d81d8b6`, see the fork's CHANGELOG for the full story).

- Hardened the same stop path against a second, independent hazard: heap
  corruption from tearing libpd down while the audio callback is live.
  `RwaSimulator::stopRwaSimulation()` sent `pd dsp 0` and ran
  `freeDynamicPdPatchers1()`'s `libpd_closefile()` and `vas_fir_list_clear()`
  *before* stopping the PortAudio stream, and none of those calls take
  `pdMutex`. The audio callback kept calling `libpd_process_float()`
  concurrently, so it could execute objects and DSP chains the main thread was
  freeing at that moment: same trap signature as the double free above, which is
  why both were suspects for the observed crash. The teardown now runs strictly
  after `stopAudio()` (`Pa_AbortStream()` guarantees the callback has returned),
  which makes every libpd call in the stop path single-threaded.

  `vas_fir_list_clear()` additionally moved to after the patches are closed. Not
  because the old order corrupted memory (the teardown never reads the `IRs`
  list, and `clear` frees only the cache nodes, never the engines or filter data
  they point to) but as an invariant: the externals' free routines never remove
  their nodes from the list, so after `libpd_closefile()` the nodes point at
  freed engines until the clear. Clearing last removes the window in which a
  lookup would touch freed memory.

  **Engine parity** (for this and the pooled-patcher fade-out fix above):
  Creator-simulator stop path only (`RwaSimulator`); no tick-loop behaviour
  changed and nothing was added to the patcher protocol, so there is nothing to
  mirror in the Player's `RwaGameLoop.swift`, but the audit of RWA Player's own
  stop/teardown ordering is planned next.

### Added

- **Two-phase stop with a master fade - the reference design for the Player's
  stop/start flow.** Stopping/starting a simulation now models the (more
  graceful) procedure that will be implemented into RWA Player:
  `stopRwaSimulation()` now only stops the game loop and fades the master gain
  to zero over 200 ms (phase A, the stream keeps running), then hands over to
  `finishStopRwaSimulation()` (phase B): the existing silent, single-threaded
  teardown: close stream, release actives, pool-wide protocol sweep, scheduler
  flush, drain, close dynamic patchers. A start requested while stopping is
  *queued* and launched automatically once the reset is complete, which is what
  Cmd-R (when simulation is running: stop directly followed by start) now rides
  on; a stop while a start is queued cancels the start. `isSimulationRunning()`
  stays true throughout the stop, so every guard that protects a running
  simulation keeps holding. Starts are soft too: the master gain jumps to 0
  before the stream opens (nothing stale reaches the first blocks) and ramps to
  the user's volume over 100 ms.

  Quitting the application and rescanning the audio devices use
  `stopRwaSimulationNow()` (synchronous, no fade) since a single-shot timer
  never fires while the event loop winds down / PortAudio restarts.

  The master gain in `stereoout.pd` has a new `rwamasterfade` receiver takes
  `<target> <ms>` lists for the stop/start ramps, after `rwamainvolume` (the
  volume slider), which is now smoothed over 20 ms. `setMainVolume()` no longer
  logs every slider tick.

  **Engine parity - porting contract for `RwaGameLoop.swift` / the Player's
  output patch:** receiver `rwamasterfade` (`<target> <ms>`), fade-out 200 ms
  (Creator; the Player should use its own, longer audience-facing length),
  fade-in 100 ms, teardown delay = fade-out + one audio buffer + margin;
  ordering: stop game loop → fade → close stream → complete the patcher release
  protocol for the whole pool → flush scheduler → drain → close dynamic
  patchers; a launch during stop is queued until the reset finished. Constants
  live in `rwasimulator.h` (`masterFadeOutMs`, `masterFadeInMs`,
  `stopTeardownDelayMs`).

### Changed

- Removed leftover debug `[print]` objects from the shipped player patches
  (`switchon`/`switchoff` in `rwaplayermonobinaural_fabian.pd`,
  `rwaplayermonobinauralogg_fabian.pd` and `rwaplayermonobrir1.pd`,
  `startCrossfade` and the `weirdSuddenfadeout` tap on the `stopfades` handler
  in the same files, `dampingfunction:` in `rwaloopplayermono.pd`). They had
  always printed on every asset start/end; the pool-wide release sweep at stop
  amplified them into a burst of hundreds of Log View lines per stop (40
  patchers \* per-patch prints, delivered at once by the immediate drain). The
  didactic prints in `rwaReferencePatch.pd` stay.

- Cleanup-pass on the shipped Pd patches: Many of the patches open with negative
  offsets, requiring to scroll the object into view every time one want to read
  them. Also reset the patcher zoom levels to a common denominator, close open
  subpatcher windows, and move/resize the patcher windows to a (smaller) main
  screen.

- Update `vas_library` to `7368810`: the filter-loading log lines in the Log
  View are now self-identifying (`vas_fir: <key>: use cached filter / new filter
  from file / new filter from arrays / replacing previous filter`, and `read <n>
  samples from array <name>`). The cache lookup's per-node debug post is gone
  (lines like `1342-reverbL /…/fabian_dir256.txt` that looked as if an IR array
  had been mapped to the HRTF file and twice misled debugging). No functional
  change; details in the vas_library's CHANGELOG.

## [v1.4.4] - 2026-08-08

### Fixed

- Fixed a crash (use-after-free) when clicking a stale asset marker in the State
  View map. `RwaGraphicsView::redrawAssetsOfCurrentState` returned early when
  the newly selected state had no assets - *before* clearing the asset layers -
  so the previous state's markers stayed on the map, each still holding a raw
  pointer to its `RwaAsset1`. Once those assets were destroyed (deleting the
  state, undo restore, or game reload all destroy states, whose destructor
  deletes their assets), clicking a leftover marker dereferenced freed memory in
  `RwaAssetList::setCurrentAsset` and segfaulted. Typical trigger: select an
  empty fallback/background state, delete the previously shown state, click one
  of its still-visible asset dots. The layers are now cleared before the
  empty-assets early return.

- Closed the remaining stale-asset-pointer holes of the same class as the crash
  above. `RwaGraphicsView::redrawAssets` only cleared the asset layers when the
  corresponding visibility flag was on, so toggling assets/reflections off left
  old markers on the (hidden) layers, and hidden layers still hit-test their
  geometries, since `Layer::setVisible` does not propagate to geometry
  visibility. Both layers are now cleared unconditionally. Additionally,
  `RwaView::setCurrentState` now resets `currentAsset` to null when the new
  state has no assets (it used to silently keep the previous state's asset
  pointer), and `RwaAssetList` resets its `currentAsset` when the list is
  rebuilt and guards the rename handler against a null current asset.

- Fixed a crash when pressing Backspace in an empty asset list. The delete
  branch of `RwaAssetList::keyPressEvent` dereferenced `currentItem()` without a
  null check. After deleting an asset the backend re-selects the state's first
  remaining asset, so deleting assets one after another works, but once the last
  asset is gone the rebuilt list is empty and `currentItem()` is null; one
  further Backspace press (an extra keystroke or key auto-repeat delivering a
  second event) crashed the app.

- Deleting an asset from the asset list no longer leaks the `RwaAsset1` object:
  `RwaState::deleteAsset` only removed it from the state's list and never freed
  it (the object was only ever deleted with the whole state). It now also resets
  the state's `lastTouchedAsset` reference before deleting; the backend and the
  views drop their references through the existing `sendCurrentState` →
  `receiveLastTouchedState` refresh that deletion already triggers. Since the
  object is now actually freed, deleting assets is refused while the simulation
  is running (same rule as dragging them). The runtime's `activeAssets` map
  holds raw pointers and could otherwise tick a freed asset. `deleteAssetItem`
  also guards against the asset no longer being found.

## [v1.4.3] - 2026-08-07

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
  pre-existing issue left as is for now). **Engine parity:** mirrored in the
  Player (`RwaGameLoop.sendData2Asset` / new `RwaAsset.playbackChannelCount`,
  see rwa-player `CHANGELOG.md` [Unreleased]); the Player's Pd branch also
  aligned its distance/elevation math with the Creator's in the process.

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
  **Engine parity:** the Player had the same 7-channel gap in
  `RwaGameLoop.swift` (`getOffsetForChannel`); fixed there in the same way
  (shared tables hand-mirrored as `RwaAsset.channelCountForPlaybackType` /
  `channelOffsetForPlaybackType`).

- Pd-patch assets no longer inherit a neighbouring audio asset's channel count
  and duration on project load. The importer's `channels`/`length` locals were
  only assigned inside the TagLib block, which is skipped for patch assets, so a
  patch following an audio asset in the `.rwa` silently took over that asset's
  values and wrote them back on the next save (visible in the example corpus,
  e.g. patches with `channelcount="2"` or five-digit durations they never had).
  Both are now reset per asset; patch assets deterministically save
  `channelcount="0"` / `duration="0"`. Harmless either way: the Creator re-reads
  audio properties via TagLib on every load and ignores the XML attributes,
  patches are excluded from the duration-based end-of-asset check, and the
  Player's importer ignores `channelcount` entirely. The importer's dead non-Qt
  `#else` branch (never compiled; read the wrong attribute and had a
  `.tofloat()` typo) was removed with it. The `pdmodes` trace fixture was
  normalised to `channelcount="0"`/`duration="0"` to match what the Creator
  itself writes.

### Added

- Pd-patch assets receive a `$0-numchannels` init value (sent alongside
  `$0-samplerate` etc. on state entry): the number of
  `azimuthN`/`distanceN`/`elevationN` channels the engine will actually stream -
  derived from the playback mode via new `RwaAsset1::playbackChannelCount()`,
  which also accounts for "headtracker relative to source" off (always 1 raw
  data set). Patches can use it to adapt their receiver wiring; it is
  deliberately not the unreliable `channelcount` XML attribute. Audio assets get
  the value too; the built-in player patches simply have no receiver for it.
  **Engine parity:** mirrored in the Player (`sendInitValues2Pd`).

- The Playback Mode dropdown hides "Auto" and "Binaural-Auto" for Pd-patch
  assets. Both dispatch on the audio file's channel count, which a patch does
  not have (TagLib cannot read `.pd` files), so on a patch they were meaningless
  (the patch plays regardless and falls back to a single data channel).
  Already-authored patch assets with an Auto mode still display it; only the
  dropdown choices are filtered.

- `tools/trace/pdmodes/` + `tools/trace/scenarios/pdmodes.scenario.json`:
  checked-in regression fixture for the two engine changes above: four dummy Pd
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
  `.env`, same as release builds) instead of ad-hoc, falling back to ad-hoc with
  a warning when no identity is available. The macOS application firewall
  identifies apps by code signature and its "automatically allow downloaded
  signed software" option only covers identified-developer signatures, so an
  ad-hoc debug build could never be durably allowed. Every rebuild produced a
  new signature, and once the allow/deny prompt stopped appearing (e.g. after an
  MDM policy sync rewrote the firewall rules), incoming OSC from RWA Players on
  UDP :8000 was silently dropped while loopback traffic kept working.
  `debug.entitlements` (`get-task-allow`) is still applied, so lldb can attach
  as before.

- Three asset attribute checkboxes ("Raw Sensors to Pd", "GPS to Pd",
  "Headtracker relative to source") no longer displayed the asset's actual value
  when an asset was selected. Most visibly, "Headtracker relative to source"
  showed unchecked although the field defaults to enabled. Attribute widgets are
  found by their label string (`findChild` on the `objectName` set in
  `addAttrCheckbox`), and the label update in `fb11ef7` renamed only the lookup
  strings in `setCurrentAsset`, not the labels the checkboxes are created with,
  so the lookups silently returned null. The constructor labels now match the
  lookups. Stored values and export were never affected; only the display was
  stale.

### Removed

- The "Required Scenes" field in the Scene view. It was scaffolding for a
  never-implemented scene-entry condition: `RwaScene::requiredScenes` is not
  serialised to the `.rwa` and is evaluated by neither engine, so anything a
  creator entered was silently lost on save. Its display code was additionally
  broken (wrong widget lookup, read the last-touched state's required states
  instead of the scene's) and had never executed. The field is commented out,
  not deleted; what a real implementation needs (visited-scene tracking, a gate
  in `setEntityScene`, serialisation and Player parity) is recorded in
  `docs/planned-features.md`. No existing game is affected: the value was never
  written to disk, and the example corpus uses neither this field nor the
  (working) state-level Required States.

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
  - New document icon `images/rwa-document.icns`, copied into
    `Contents/Resources` by CMake.

- Key commands for the operations a creator repeats all day.

  | Key command | Action                                       | Menu                                |
  | ----------- | -------------------------------------------- | ----------------------------------- |
  | Cmd-N       | New                                          | File                                |
  | Cmd-O       | Open                                         | File                                |
  | Cmd-S       | Save                                         | File (was a hidden shortcut before) |
  | Cmd-Shift-S | Save Version as…                             | File                                |
  | Cmd-Opt-S   | Copy Project to…                             | File                                |
  | Cmd-E       | Export Project for transfer to RWA Player... | File                                |
  | Cmd-Shift-E | Send Project to Sharing Server...            | File                                |
  | Cmd-R       | Run Simulation, restarts a running one       | Simulation                          |
  | Cmd-K       | Stop Simulation                              | Simulation                          |
  | Cmd-Shift-L | Clear Log Window                             | View                                |

- **Simulation** menu with *Run Simulation* and *Stop Simulation*, doing the
  same as the start/stop buttons in the Map View toolbar - the only place from
  which the simulation could be run until now, so there was nothing a key
  command could hang on.

  Cmd-R means "run from the top": on a running simulation it restarts it, taking
  one keystroke for what the toolbar takes two clicks. The menu entry is called
  *Restart Simulation* while the simulation runs, and *Stop Simulation* is
  greyed out while it does not.

- **Clear Log Window** in the View menu, the menu counterpart of the log view's
  *clear* button.

- The sharing server now logs what connected RWA Player apps do, so a creator can see
  whether a phone reached the sharing server at all and which game it pulled:

  ```
  Player 192.168.1.42 requests GET /My Soundwalk.zip
  Player 192.168.1.42 got /My Soundwalk.zip (200, 24.3 MiB)
  Player 192.168.1.42 could not get /Typo.zip (404)
  ```

  The arrival line is separate on purpose: a game of that size takes a while to
  transfer and would otherwise only show up once it is through. Starting the
  server also logs its port and the directory it serves.

  Note that httplib calls its logger even when writing the response failed, and
  the status is by then already sent: a player who walks out of wifi
  mid-download is logged as a completed transfer.

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
  until the entry is selected again. The pick is then remembered by name and
  wins over the default whenever it is present, including after it was unplugged
  and reconnected.

  Both selections survive a restart (`audiooutputdevice` / `audioinputdevice` in
  the settings). They are stored by name, not by index: PortAudio hands out
  indices in whatever order it enumerates the hardware, so they mean nothing in
  the next session. A stored device that is not connected at startup stays
  remembered, RWA Creator runs on the system default until it appears.

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
  project (without saving) and opens a file dialogue (now correctly titled "New
  RWA Project" instead of "Copy entire RWA Project Folder") to save the new
  project. The first save of a game that was never written to disk opens the
  same dialogue as "Save RWA Project". "Copy Project to..." keeps its title. All
  three still write a complete project folder, only the title of the dialogue
  differs.

- The start button in the Map View toolbar now follows the state of the
  simulation instead of only its own clicks, otherwise Cmd-R and Cmd-K would
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
  native style to `QStyleSheetStyle`, which stops tracking palette switches. The
  font is now set with `QFont` (with a monospace style hint as fallback should
  Andale Mono be missing) and no stylesheet remains on the widget.

- Clicking empty space in the state or asset lists no longer clears the
  selection. Previously the click deselected the last touched item while the
  attribute form next to the list allowed changing of values. The empty-space
  click is now simply ignored (`RwaListView::mousePressEvent`, shared by all
  three lists).

- The asset attribute form no longer shows stale values after switching states.
  Previously it kept displaying the previous state's asset when the newly
  selected state had no touched asset yet; edits then either went nowhere or,
  when the new state contained an audio file of the same name, silently landed
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
  (`paWrapper::rescanDevices()`), which is the only way to pick up the new
  device list, and repopulates the menu. Selections are tracked by device name
  rather than by index, since the indices are reused: on the machine this was
  developed on, index 2 was "Externe Kopfhörer" with the headset plugged in and
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
  own Pd patcher (opened from the game's `assets/` folder) did not, unless the
  38 MB file was copied in by hand. Two changes:

  - `vas_library` (`vas_pdmaxobject_read`) now resolves the IR through
    `open_via_path`: absolute path, then the patch's own directory, then Pd's
    global search path. A missing file is reported by name instead of being
    passed down as a bad path, and the path is bounded (`fullpath` is 512 bytes,
    `MAXPDSTRING` is 1000).
  
  - `RwaRuntime` registers the bundled `puredata` directory with
    `libpd_add_to_search_path()`, so any patch resolves the HRTF set from there.

  Existing games that carry their own copy keep working, the patch directory is
  still searched first. **Engine parity**: mirrored in the Player, which adds
  `Bundle.main.resourcePath` to the search path in `RwaGameLoop.init`; both apps
  build the same `vas_library` sources, so the external side is shared.

### Changed

- `vas_library` now tracks `rnd-hsm-klassik/vas_library` (branch
  `rwa-player-fixes`) instead of `funkerresch/vas_library`, the same fork the
  Player already used. The fork additionally carries null-terminator fixes in
  three `vas_mem_alloc` calls, `pd_error` instead of the deprecated `error`, and
  drops its stale bundled `m_pd.h` copies (Pd headers now come from
  `libpd/pure-data/src`).

## [v1.3.0] - 2026-07-29

**Engine parity updates**: align game engine behaviour between RWA Creator and
RWA Player

### Added

- Added `rwatrace`, a headless, deterministic runtime trace harness for
  differential testing against RWA Player, including a smoke scenario and
  runtime investigation documentation.
- Add context (filename, function, line) to log output in release builds.
- Sent asset gain to Pure Data when an asset is initialized (`sendInitValue2pd`)

### Changed

- Improved scene transition handling in `setScene`: **Important**: this diverges
  from the RWA Player's historical behavior (which always cut active assets on
  scene change) and must be mirrored there for engine parity.
  - always end background assets when switching scenes
  - release the current state's block latch
  - switch scene
  - explicitly handle fallback activation: fallback enabled: the scene's
    fallback state; fallback disabled: active assets keep playing until a new
    state is triggered.
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
  allowing for panning and distance attenuation without activating another state
  before.
- `RwaRuntime::setScene` no longer invokes undefined behavior when a scene has
  fallback enabled but no states: it logs a warning and leaves the entity
  without a current state (same as fallback-disabled), instead of calling
  front() on an empty state list.
- Prevent scene-selection signals from clearing the current state after a scene
  transition: Updated `receiveLastTouchedScene()` to prevent the signal
  `sendSelectedScene` from echoing into RwaRuntime::setScene(), where a
  previously set state would be again unset.

### Documentation

- Documented runtime asset fields and the engine's scene/state transition and
  audio-continuity behavior.

## [v1.2.6] - 2026-07-23

### Changed

- `RwaRuntime::setScene`: active assets of the previous state are now only ended
  when the new scene activates a fallback state that contains assets. A scene
  with fallback disabled, or with a silent (asset-less) fallback, lets running
  assets play out until a new state is triggered, instead of cutting to silence
  on the scene transition. Note: this diverges from the RWA Player's historical
  behavior (which always cut active assets on scene change) and must be mirrored
  there for engine parity.

- The hero map icon (`images/hero4.png`) got a transparent background.

- **Update UI strings for save/export/open actions**

  Rename menu items and dialog titles to clarify RWA and Sharing Server
  terminology (e.g. Save Version as..., Copy Project to..., Send Project to
  Sharing Server..., Export Project for transfer to RWA Player..., Save RWA
  File, Open RWA File, File Path Preferences). Update preference labels (Sharing
  Server Path, Project Export Path) and toolbar tooltip.
  
  Also renamed client associated download/export functions/properties to reflect
  update UI strings. The cutoff is keys in settings, as those should be retained
  for users.

- **Add dialog title and reduce widths**
  
  Enhance RwaInputDialog to accept a title parameter
  for better context in file path preferences.
  
- **Engine parity**
  
  send gain when in sendInitValue2pd
  
- **Scene Switch**
  
  Improved scene transition handling in setScene:
  
  - always end background assets when switching scenes
  - release the current state's block latch
  - switch scene
  - explicitly handle fallback activation:
  - fallback enabled: the scene's fallback state
  - fallback disabled: active assets keep playing until a new state is triggered
  - activate background state

### Fixed

- `RwaRuntime::setScene` no longer invokes undefined behavior when a scene has
  fallback enabled but no states: it logs a warning and leaves the entity
  without a current state (same as fallback-disabled), instead of calling
  front() on an empty state list.
- Updated `receiveLastTouchedScene()` to prevent the signal `sendSelectedScene`
  from echoing into RwaRuntime::setScene(), where a previously set state would
  be again unset.

### Documentation

- Documented `setScene`'s contract (post-call state per fallback setting,
  audio-continuity rule, preconditions) and clarified that the latch release on
  the old state exists because the per-tick geographic unblock only scans the
  current scene.

## [v1.2.5] - 2026-07-22

### Fixed

- Fixed a dangling-pointer crash when deleting FALLBACK states: several places
  (asset list, backend, map view, state list) kept references to the deleted
  state or its assets and dereferenced them afterwards.

- Fallback and background states now initialise correctly at simulation start,
  without another state having to be triggered first; fixed dangling pointers to
  the scene's background and fallback state references in the runtime.

### Added

- `debug.entitlements` and a signing step in `build_debug.sh`, so debug builds
  have a code-signing identity and CoreBluetooth allows the headtracker
  connection.

- `NSLocalNetworkUsageDescription` in `Info.plist.in`, so macOS shows the
  local-network permission prompt needed by the OSC listener (incoming data from
  RWA Players on UDP :8000).

## [v1.2.3] - 2026-07-06

### Added

- About menu and dialog (app icon, name, version, commit hash).

### Changed

- Toolbar and map icons are shipped as SVG instead of PNG.
- Debug build script and output path updated.

## [v1.2.2] - 2026-07-02

### Changed

- The app bundle is named **RWA Creator** (was `rwacreator`).
- New icon drafts for the map/toolbar icon set; attempt to fix the app icon's
  scaling issue.
- Attribute views (game, scene, state, asset) have fixed widths and no longer
  show horizontal scroll bars; attribute-row construction was refactored into a
  shared method for consistent layout across the views.
- Release build script updated.
- **Qt5 portability cherry-picks** kept in sync with the `legacy` branch:
  `QOverload`-based signal connections, `QString::fromStdString` in debug
  output. Submodule remotes switched to HTTPS; version string no longer
  duplicated in `main.cpp`.

## [v1.2.1] - 2026-06-30

### Changed

- The qmake project file (`rwacreator.pro`) was deleted: CMake is the only build
  system on this branch.
- Stopped tracking build artifacts (clangd cache, `rwabuild/`).
- Application metadata is set via `QCoreApplication` so `QSettings` uses the
  app/bundle identity; the hardcoded `QSettings("Intrinsic Audio", "Rwa
  Creator")` instances were replaced with default-constructed `QSettings`
  (cherry-picked from `legacy`).
- The release DMG contains an Applications install link; build instructions
  fixed.

## [v1.2.0] - 2026-06-29

### Added

- Log level selector in the Log View: messages below the selected level are
  dropped (new `msgSeverity`). Filename, line and context are only shown when
  the Debug level is selected.

### Changed

- Headtracker data is logged only to the log window (no longer to the console)
  and moved to the "other" log filter.
- Coordinate log output formatted to the WGS-84 convention; the coordinate log
  filter renamed accordingly.
- Log levels across runtime, creator and bluetooth messages revised.
- Bluetooth code consolidated into the `bluetooth/` directory.

### Fixed

- Floating (detached) dock widgets are restored on startup instead of being
  re-docked.

## [v1.1.0] - 2026-06-28

### Added

- Application icon (`images/rwa-creator.icns`).
- Release build/sign/notarize script with `.env`-based credentials; Qt
  deployment (`macdeployqt`) integrated into the CMake build, replacing the old
  standalone shell scripts.
- `README.md` with build prerequisites and IDE setup instructions.

### Changed

- Bluetooth (headtracker) code migrated to Qt 6, including the Qt 6 Bluetooth
  permission handling.
- Log window rewritten on `QPlainTextEdit` for better performance and
  readability; it displays all log levels.
- Pd patches are bundled under `Contents/Resources/puredata`.
- `Info.plist` is generated by CMake from a template and embeds the version and
  git commit hash.
- Map tile requests send a User-Agent header, as required by the OSM tile usage
  policy.
- macOS deployment target raised to 13.0 (legacy branch kept at 11.0); wider
  asset area in the State View.

### Fixed

- Missing-font lookup error in the style definitions.
- Directory-clearing utility no longer touches the current working directory
  when the given path is empty, relative or missing (guards in
  `rwautilities.cpp`; tmp/undo cleanup is skipped while backend paths are
  unset).
- Set title of the headtracker name dialog.

## [v1.0.0] - 2026-06-19

First version under Semantic Versioning ("new version format").

### Changed

- Address search in the map view switched from OSM Nominatim to the swisstopo
  SearchServer API (`api3.geo.admin.ch`, JSON); network errors of the lookup are
  now logged.
- macOS bundle identifier changed to `com.fhnw.rwa.creator`.
- Deployment target lowered to macOS 12.0 and the build made universal (arm64 +
  x86_64).
