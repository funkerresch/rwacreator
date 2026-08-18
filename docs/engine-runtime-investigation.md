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
| `<tag>-azimuth<n>` | degrees | bearing relative to head azimuth |
| `<tag>-elevation<n>` | degrees | |

Channel count N depends on `playbackType` for audio *and* (since the
multichannel-patch change) Pd-patch assets: mono/stereo/binaural-mono = 1,
binaural-stereo = 2, 5-channel = 5, 7-channel "Fabian" = 7
(`RwaAsset1::channelCountForPlaybackType`; the effective per-asset count
incl. the Pd minimum of 1 is `RwaAsset1::playbackChannelCount`). Channels are
spread on a circle of `channelRadius` around the source with fixed per-channel
angular offsets (`RwaAsset1::channelOffsetForPlaybackType`; binaural stereo
±60°, 5-ch −60/0/60/−120/120, 7-ch −40/0/40/−80/80/−120/120) plus asset
`rotateOffset`.
Geometry per channel (`calculateChannelBearingAndDistance`,
rwaruntime.cpp:1018-1057): channel coord = `calculateDestination1(assetPos,
channelRadius, offset)`; distance = `calculateDistance1(entity, channelCoord)`
clamped to `minDistance` (or `fixedDistance`), altitude folded in via
`calculateDistanceWithAltitude`; bearing = `calculateBearing1(entity,
channelCoord, entity->azimuth())` (or `fixedAzimuth`). If
`headtrackerRelative2Source` is false (PD assets), raw `entity->azimuth()` /
`elevation()` are sent instead.

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
