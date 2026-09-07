# rwatrace — headless RWA engine trace harness

Runs the real `RwaRuntime` (the Creator's game engine) without GUI, audio
device, or Pure Data: a link-time fake of libpd records every message the
engine sends and lets scenarios inject `playfinished` bangs. Deterministic —
time is tick-driven (25 ms), same as the Creator's simulator.

## Build & run

Built by the normal debug build (`./build_debug.sh`, target `rwatrace`,
toggle with `-DRWA_BUILD_TRACE_TOOLS=OFF`).

```
./build/cmake-debug/rwatrace \
    --game     path/to/game.rwa \
    --scenario tools/trace/scenarios/smoke-background.scenario.json \
    --out      trace.jsonl
```

## Scenario format (`rwa-scenario/1`)

```json
{ "schema": "rwa-scenario/1",
  "startScene": "Scene 0",
  "durationMs": 10000,
  "timeline": [
    { "t": 0,    "pos": { "lon": 7.946, "lat": 47.288 } },
    { "t": 3000, "azimuth": 90 },
    { "t": 3000, "elevation": 10 },
    { "t": 5000, "step": true },
    { "t": 8000, "playFinished": { "asset": "forest.ogg" } } ] }
```

`t` is milliseconds; each input is applied at the first tick where
`tick*25 >= t`. `playFinished` simulates Pd reporting the asset's patch done
(delivered through the queued-bang path, exactly like production). The
`asset` form resolves to the *newest* instance of that asset; to finish an
older instance that has since been superseded (e.g. a background asset
restarted while its predecessor was still fading out), address the patcher
directly with `{ "playFinished": { "tag": 1002 } }`. Tags appear in the
trace's `-play` events and input echoes.

## Trace format (JSONL)

- `{"ev":"pd","recv":"<tag>-<param>","val":…|"bang":true|"sym":…,"asset":…,"t":…,"tick":…}` —
  every libpd message; `asset` is resolved from the tag's `-play` symbol.
- `{"ev":"state","scene":…,"state":…,"prevScene":…,"prevState":…}` — polled
  scene/state transition after each tick.
- `{"ev":"input",…}` — echo of applied scenario inputs.

The `<tag>-seed` init value is a platform-RNG draw in production; the harness pins
`RwaRuntime::seedSource` so it is always `1` (the Player's `ScenarioTraceRunner`
does the same), keeping traces deterministic.

## Purpose

Golden-trace differential testing against the iOS Player (`rwa-client`),
whose Swift engine (`RwaGameLoop.swift`) is a hand-mirrored port. The same
scenario replayed there (XCTest + swizzled `PdBase` sends) must produce an
equivalent trace. See `docs/engine-runtime-investigation.md` for the engine
walkthrough and the list of currently known divergences.

## Files

- `fake_libpd.{h,cpp}` — link-time libpd double (record + inject)
- `stub_backend.cpp` — satisfies `RwaBackend` symbols for headless import;
  `getInstance()` returns nullptr, routing `RwaImport` through its fallbacks
- `rwatracemain.cpp` — scenario runner / trace writer
- `scenarios/` — shared scenario scripts
- `pdmodes/` — fixture game for playback-mode-dependent spatial data to
  Pd-patch assets (`scenarios/pdmodes.scenario.json`): four dummy patches in a
  BACKGROUND state — binaural stereo (expect `azimuth1`+`azimuth2`), binaural
  mono (`azimuth1` only), binaural stereo with "headtracker relative to
  source" off (`azimuth1` only, raw head azimuth/elevation) and binaural
  7 channel (`azimuth1`–`azimuth7` at distinct angular offsets). Each patch
  also gets a `numchannels` init value matching the number of streamed
  channels (2/1/1/7). Run with `--game tools/trace/pdmodes/pdmodes.rwa`; the
  game paths in scenario files are informational, only the CLI options count.
- `scenechange/`: two-scene fixture games for the scene-transition scenarios
  (see its README).
- `spatial/`: fixture + `check_trace.py` for the float distance / azimuth /
  elevation chain (`scenarios/spatial-edge.scenario.json`): fractional, negative
  and > 360 yaw, pitch 90/−95/120/−150, listener on the source, `minDistance`,
  `fixedazimuth`, `fixeddistance` (see its README). `azimuth`/`elevation`
  scenario inputs may be fractional.
- `gainhierarchy/`: fixture for hierarchical gain (scene \* state \* asset,
  `scenarios/gainhierarchy.scenario.json`): expect `-gain` 0.125 for the GPS
  state's asset and 0.5 for the background asset on every tick (see its README).
