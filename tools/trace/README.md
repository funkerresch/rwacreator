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
(delivered through the queued-bang path, exactly like production).

## Trace format (JSONL)

- `{"ev":"pd","recv":"<tag>-<param>","val":…|"bang":true|"sym":…,"asset":…,"t":…,"tick":…}` —
  every libpd message; `asset` is resolved from the tag's `-play` symbol.
- `{"ev":"state","scene":…,"state":…,"prevScene":…,"prevState":…}` — polled
  scene/state transition after each tick.
- `{"ev":"input",…}` — echo of applied scenario inputs.

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
