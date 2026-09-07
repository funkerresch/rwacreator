# RWA Engine Runtime Investigation

Investigation of the game/runtime mechanics of the RWA engine, centered on
`sendData2activeAssets`, state/scene activation, and the data protocol sent to
assets — in both **RWA Creator** (this repo, Qt/C++ desktop) and **RWA Player**
(`rwa-client`, iOS Swift). Date: 2026-07-24.

---

## 1. Architecture overview

Both apps contain a **hand-mirrored copy of the same engine** — the iOS Player
did *not* reuse the C++ runtime; it re-implements it in Swift:

| | Creator (C++) | Player (Swift) |
|---|---|---|
| Engine core | `rwaruntime.cpp` (`RwaRuntime`) | `rwaClient/src/RwaGameLoop.swift` |
| Loop driver | `RwaSimulator` `QTimer` @ **25 ms (40 Hz)** | `Foundation.Timer` in hidden `SecondViewController` @ **10 ms (100 Hz)** |
| Position input | OSC UDP :8000 `/position`, map drag | CoreLocation events |
| Head orientation | `RwaHeadtrackerConnect` (BT/serial) | BLE from ESP32 headset |
| Audio backend | libpd (Pure Data) + portaudio | libpd via `PdBase` |
| Game data | `.rwa` XML + `assets/` (rwaexport/rwaimport.cpp) | same XML via `RwaImport.swift` |
| Tests | none | none |

The shared artifact between the two is the `.rwa` XML export
(`<rwa version="1.0">` → scene → state → enterconditions/actions/assets) plus a
co-located `assets/` folder. Sample: `rwa-client/rwaGames/rwatest/`.

## 2. The game loop tick

`RwaRuntime::update(entity)` (rwaruntime.cpp:1781) runs every tick:

1. `emptyPdMessageQueue()` — drain libpd's queued hooks (delivers `-playfinished` bangs → asset/patcher release via `bangpd`/`bangpdHelp`, rwaruntime.cpp:271-321).
2. `sendData2activeAssets(entity)` — push fresh spatial data to all playing assets.
3. `setEntityState(entity)` — evaluate scene/state enter/exit conditions.
4. `processAssets(entity)` — activate **at most one** newly-eligible asset per tick (note the `break`, rwaruntime.cpp:952).

Swift's `updateGameState()` (RwaGameLoop.swift:1574) mirrors steps 2–4 (Pd
callbacks arrive via delegate instead of an explicit queue drain).

Time is **tick-accumulated** in both engines (`timeInCurrentState +=
schedulerRate/1000`), not wall-clock — scripted simulation is deterministic.

## 3. sendData2activeAssets — the asset data protocol

`sendData2activeAssets` (rwaruntime.cpp:1257-1299) iterates each entity's
`activeAssets` and `backgroundAssets` maps (`std::map<uniqueId, AssetMapItem
{RwaAsset1*, patcherTag}>`, rwaentity.h:31-48) and calls `sendData2Asset` per
item. Background assets belong to the *scene* and are serviced even when the
entity has no current state.

**Transport is libpd, not OSC.** Every message goes to a Pd receive symbol
named `"<patcherTag>-<param>"`, where `patcherTag` is the `$0` of a
pre-allocated Pd voice patcher (pool of 40 + 4×5-channel + 4×7-channel,
rwaruntime.h:36-38). OSC (:8000) is only a side channel for live phone↔Creator
position preview; HTTP :8088 transfers game files.

### Per-tick messages (`sendData2Asset`, rwaruntime.cpp:1086-1255)

| Receiver | Value | Notes |
|---|---|---|
| `<tag>-gain` | effective gain = asset \* state \* scene gain (`RwaRuntime::effectiveGain`) | every tick (C++); the Player computes the same product once at activation |
| `<tag>-lon` / `<tag>-lat` | listener GPS | every tick (C++) |
| `<tag>-step` | bang | when headtracker registered a footstep |
| `<tag>-distance<n>` | meters | per channel n=1..N |
| `<tag>-azimuth<n>` | degrees, float, [0, 360) | source direction relative to the head (yaw + pitch), +180 convention: 180 = straight ahead, 270 = right |
| `<tag>-elevation<n>` | degrees, float, [-90, 90] | source elevation relative to the head |

Channel count N depends on `playbackType` for audio *and* (since the
multichannel-patch change) Pd-patch assets: mono/stereo/binaural-mono = 1,
binaural-stereo = 2, 5-channel = 5, 7-channel "Fabian" = 7
(`RwaAsset1::channelCountForPlaybackType`; the effective per-asset count
incl. the Pd minimum of 1 is `RwaAsset1::playbackChannelCount`). Channels are
spread on a circle of `channelRadius` around the source with fixed per-channel
angular offsets (`RwaAsset1::channelOffsetForPlaybackType`; binaural stereo
±60°, 5-ch −60/0/60/−120/120, 7-ch −40/0/40/−80/80/−120/120) plus asset
`rotateOffset`.

Geometry per channel, all `double` (`calculateChannelBearingAndDistance` +
`sendData2Asset`, rwaruntime.cpp; mirrored in `RwaGameLoop.swift`):

- channel coord = `calculateDestination1(assetPos, channelRadius, wrap360(offset + rotateAngle))`
- horizontal distance `d` = `calculateDistanceInMeters(entity, channelCoord)`
  (haversine, R = 6373000 m), then `max(d, minDistance)` if `minDistance >= 0`,
  or `fixedDistance` if set. No other floor: `d` may be 0, and it is never
  negative (the `minDistance`/`fixedDistance` sentinels `-1` are never sent).
- world elevation `e` = `atan2(altitude, d)` (finite at `d = 0`: 0 or ±90);
  the distance sent is `hypot(d, altitude)`.
- world bearing `b` = `calculateWorldBearing(entity, channelCoord)`, clockwise
  from north.
- `(azimuth, elevation)` = `RwaUtilities::calculateRelativeDirection(b, e,
  entity->azimuth(), entity->elevation())`: builds the source unit vector
  `v = (cos e sin b, cos e cos b, sin e)` (east, north, up) and the head axes
  forward/up/right from yaw ψ and pitch θ, then
  `azimuth = wrap360(atan2(v·r, v·f) + 180)`,
  `elevation = atan2(v·u, hypot(v·f, v·r))`. Reduces to `wrap360(b − ψ + 180)`
  and `e` at pitch 0; pitch past vertical flips the azimuth by 180°. Roll is
  ignored.
- `fixedAzimuth`: azimuth = `wrap360(fixedAzimuth + channelOffset)`, elevation
  = `e`; neither yaw nor pitch is applied.
- If `headtrackerRelative2Source` is false (PD assets), the raw
  `entity->azimuth()` (wrapped to [0, 360)) / `entity->elevation()` (wrapped to
  (−180, 180], **not** clamped) are sent instead.

Head orientation enters as `double`: `RwaSimulator::receiveAzimuth/Elevation`
→ `RwaEntity::setAzimuth/setElevation` (non-finite input ignored). The
`send*` functions drop non-finite values. Distance / azimuth / elevation were
integers before this (whole meters on the default path, whole degrees
everywhere) — see CHANGELOG. Distance-to-gain ("damping") is computed entirely
in the Pd patches, see §7 for the renderer side.

`sendData2Asset` also advances per-tick asset dynamics: source movement toward
end position, loop-until-end (`-end` bang), auto-stop of non-looping assets at
`duration+offset`, auto-rotate, fade/playhead bookkeeping (rwaruntime.cpp:1200-1254).

### One-time init on activation (`sendInitValues2pd`, rwaruntime.cpp:794-924)

`-assetlon -assetlat -samplerate -numchannels -dampingfunction -dampingfactor
-dampingtrim -dampingmin -dampingmax -smoothdist -offset -loop -fadeintime
-fadeouttime -crossfadetime -crossfadeafter -firstcrossfade
-playheadposition -seed`, then
`libpd_symbol("<tag>-play", <full asset path>)` loads and starts the file.

`-seed` is a new platform-RNG draw per activation, `1 + (rng & 0xFFFFFE)` so it
is exact in float32. It exists because Pd's `[random]` seeds from a fixed constant,
every launch replays the same sequence unless the patch
reseeds. Both engines draw independently (a *wanted* divergence, see §5); the trace harnesses pin
`RwaRuntime::seedSource` / `RwaGameLoop.seedSource` so the trace shows `-seed 1`.

### Stop / release

`sendEnd2activeAssets` (rwaruntime.cpp:524-564) bangs `<tag>-end` (Pd
fades out); Pd answers with a `<tag>-playfinished` bang → runtime erases the
asset from `activeAssets` and releases the patcher back to the pool.

## 4. State & scene activation (`setEntityState`, rwaruntime.cpp:1455-1698)

Evaluated every tick (C++), gated by:

- `minimumStayTime` of current state and scene (default 4 s, rwaarea.h:91)
- `leaveOnlyAfterAssetsFinish` while assets still play

**Scene switch** (`setEntityScene` → `setScene`, rwaruntime.cpp:1366-1408):
among scenes of the *same level*, if the entity is inside another scene's ENTER
area, switch: end active + background assets, current state ← scene's **front
state (fallback)** unless `fallbackDisabled`, reset timers, start background
state.

**State enter** (GPS states only, not current, not `blockUntilRadiusHasBeenLeft`):

1. `entityIsWithinArea(state, ENTER)` — circle / rectangle / polygon
   (rwaruntime.cpp:1301-1364), with separate exit-offset geometry for
   hysteresis.
2. `requiredStates ⊆ visitedStates` — else entry is blocked
   (`blockUntilRadiusHasBeenLeft`) and an optional `hintState` is routed to.
   `visitedStates` is a flat, game-wide list of state *names* on the entity
   (never cleared on scene change), so required states may reference states of
   **any** scene, but duplicate state names across scenes will make any
   requirement referring to that name match. The Creator's attribute view warns
   on unknown/ambiguous names. `hintState` remains scene-scoped in both engines.
3. `enterOnlyOnce` respected against `visitedStates`.
4. On enter: `sendEnd2activeAssets`, set current state, append to
   `visitedStates`, `unblockAssets`, reset state timer.

**State exit** triggers: state `timeOut`; scene/background `timeOut` → next
scene; `leaveAfterAssetsFinish` when `activeAssets` empties; leaving the EXIT
area; pending hint. Exit routing precedence (`exitState`,
rwaruntime.cpp:1645-1697): **hint → explicit nextScene → state.nextScene →
state.nextState → fallback (scene front state)**, unless `fallbackDisabled`.

**Entity** (`rwaentity.h`): the hero/listener — owns GPS coordinates and head
azimuth/elevation, plus a full copy of the game graph and its own runtime
containers (`currentScene/State`, `activeAssets`, `backgroundAssets`,
`visitedStates`, timers). Inputs write asynchronously; the timer loop reads.

## 5. Confirmed divergences Creator ↔ Player

Verified in source on both sides:

| # | Aspect | Creator (C++) | Player (Swift) | Impact |
|---|---|---|---|---|
| 1 | Tick rate | 25 ms | 10 ms | movement/fade/playhead per-tick math scales differently |
| 2 | State evaluation | every tick | ~1 Hz — `if (fmod(hero.timeInCurrentState, 1) >= 0.01) return` (RwaGameLoop.swift:864) | up to ~1 s transition latency on iOS |
| 3 | Per-tick sends | `-gain`, `-lon`, `-lat` every tick, all assets | `-gain` only at init; `-lon/-lat/-step` only for PD-type assets | runtime gain changes inaudible on iOS. Harmless for hierarchical gain (scene \* state \* asset, both engines send the same product): on the Player nothing edits gain while a game runs and an asset's owning state/scene never changes, so the init-time value stays correct |
| 4 | Background assets w/o state | serviced regardless (rwaruntime.cpp:1285 comment) | `sendData2ActiveAssets` early-returns if `currentState == nil` **or** `activeAssets.isEmpty` (RwaGameLoop.swift:1369-1375) — background assets then get no data | background audio starves on iOS in fallback-disabled scenes / empty states |
| 5 | Patcher pools | 40 + 4×5ch + 4×7ch | 30 (+15 stereo etc.) | exhaustion behavior differs |
| 6 | Latent C++ quirk | `backend->sampleRate` hardcoded 48000 used in `sendInitValues2pd`/playhead math while ctor receives the real device rate (rwaruntime.cpp:809, 846, 1243-1252) | n/a | wrong playhead/offset math on 44.1 kHz devices |
| 7 | `-seed` init value | `QRandomGenerator::global()` | `UInt32.random` | **intended**: values differ per engine and per run, real randomness preferred over parity. Both harnesses pin the source so traces show `-seed 1`; diff by receiver only |
| 8 | `blockUntilRadiusHasBeenLeft` re-check after required-state loop | only consulted in the outer enter condition (rwaruntime.cpp:1530); the required-state failure path sets it during the same iteration | additionally re-checks the flag *after* the required-state loop (`RwaGameLoop.swift` ~:1002) and forces `enterConditionsFulfilled = false` | under some orderings C++ can still enter a state in the same tick where Swift cannot |
| 10 | Spatial resolution (historical) | int meters on the default distance path, int degrees for head yaw/pitch and bearing | float meters, int head yaw/pitch | **closed**: both engines now run the same double-precision chain (`calculateRelativeDirection`); verified by `scenarios/spatial-edge.scenario.json` + `tools/trace/spatial/check_trace.py` |
| 9 | Blocking on unmet required states | sets `blockUntilRadiusHasBeenLeft` unconditionally on an unmet requirement (rwaruntime.cpp:1543), hint state or not | sets it only inside the `hintState != ""` branch (`RwaGameLoop.swift` ~:989) | without a hint state, the Player re-evaluates (and re-fails) the state every pass instead of blocking until the area is left |

## 6. Sync strategy: golden-trace differential testing

Since the engines are parallel ports, sync must be **behavioral**: shared
scenario inputs (a `.rwa` game + scripted timeline of GPS/heading/step/
playfinished events) are fed to each engine headlessly; each emits a canonical
JSONL trace (state transitions + every pd message); traces are canonicalized
(patcherTag → asset basename via the `-play` message) and diffed with numeric
tolerances and a known-divergence allowlist.

- **C++ side**: headless `rwatrace` tool (see `tools/trace/`) — links the
  engine sources against a *fake libpd* that records messages instead of
  making sound. Feasible because rwaruntime.cpp touches `RwaBackend` only via
  the fields `logSim`/`logPd`/`sampleRate` and uses ~20 libpd functions.
- **Swift side**: XCTest target hosted in the app; swizzle the four `PdBase`
  send methods to record; drive `rwagameloop.updateGameState()` directly from
  a scenario runner; inject `receiveBang(fromSource: "<tag>-playfinished")`.
- **Debugger**: use lldb/Xcode breakpoints (e.g. `RwaRuntime::setEntityState`
  transition points, `sendInitValues2pd`, `bangpdHelp`) for *exploration*;
  everything regression-shaped belongs in the automated traces.
- **Protocol**: goldens live with the Creator (source of truth, pending
  confirmation), mirrored into the iOS test bundle; an engine-behavior PR in
  either repo must match the goldens or regenerate them from the
  source-of-truth engine — the sibling repo's CI failure *is* the sync signal.

## 7. Binaural external: rounding and binning of azimuth / elevation

The engine now sends floats, but the renderer quantises them. The `*_fabian`
patches feed `[r $0-azimuthN] → [expr 360 - $f1] → [+ 180]` and
`[r $0-elevationN]` (→ `[clip -90 89]`) into `rwa_binauralsimple~`
(`vas_library/examples/PureData/rwa_binauralsimple~.c`, compiled into both
apps), which calls `vas_dynamicFirChannel_setAzimuth/setElevation`
(`vas_library/source/vas_dynamicFirChannel.c`). Those take **`int`**, so the
float is truncated toward zero at the call:

- **Azimuth**: `if (azi < 0) azi += 360; azi %= 360;` (only one wrap, so
  values ≤ −360 would index negatively — the engine guarantees [0, 360) and the
  patch adds at most 540), then `index = azi / aziStride` with `aziStride = 3`
  for `fabian_dir256.txt` (120 bins). Integer division: 0–2.99° → bin 0,
  3–5.99° → bin 1, … i.e. **truncation to the lower 3° edge, not
  round-to-nearest**.
- **Elevation**: accepted only if `eleMin ≤ e < eleMax` = `[-90, 90)`;
  `index = e / eleStride + eleZero` = `e / 3 + 30` (60 bins covering −90…+87).
  C integer division truncates toward zero, so the horizon bin (index 30)
  spans −2.99…+2.99° (5° wide) while every other bin is 3° wide. Values ≥ 90
  or < −90 are **silently ignored** (the previous direction stays) — hence the
  patch-side `[clip -90 89]`.
- There is **no directional interpolation** between HRIRs; only a temporal
  crossfade between the old and new filter when the index changes
  (`vas_dynamicFirChannel_crossfadeBetweenOldAndNewFilter`). Sub-degree
  precision from the engine therefore only makes bin transitions land at the
  right moment; the audible resolution stays 3°.
- `(int)NaN` is undefined behaviour in C, which is why the engine refuses to
  send non-finite values.

A follow-up on the vas fork (round-to-nearest, clamp elevation to the last
bin) is possible but needs a fork commit plus submodule bumps in both repos;
it is deliberately not part of the float-chain change.
