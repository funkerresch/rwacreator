# spatial: float distance / azimuth / elevation fixture

Eight 1-channel binaural Pd assets (`playbacktype` 8, `channelradius` 0) in one
BACKGROUND state around the scene centre (7.586, 47.577):

| Asset | Position | Purpose |
| --- | --- | --- |
| `horizon.pd` | 20 m north, altitude 0 | reference source dead ahead at yaw 0 |
| `elevated.pd` | 20 m north, altitude 5 | world elevation approx. 14° |
| `onspot0.pd` | scene centre, altitude 0 | listener exactly on the source, `atan2(0, 0)` |
| `onspot5.pd` | scene centre, altitude 5 | source straight overhead, world elevation 90 |
| `mindist.pd` | 1 m east, `mindistance` 3 | distance floor |
| `fixedazi.pd` | 20 m north, `fixedazimuth` 45.5 | fractional fixed orientation, ignores the head |
| `fixeddist.pd` | 20 m north, `fixeddistance` 2.5 | fractional fixed distance |
| `rawhead.pd` | 20 m north, `headtrackerrelative2source` 0 | echoes the wrapped head yaw/pitch |

Scenario `../scenarios/spatial-edge.scenario.json` drives head yaw 12.5, −45,
725 and pitch 90, −95, 120, −150 (past vertical), then moves the listener
0.5 m north (sub-metre distances). Run and check:

```bash
./build/cmake-debug/rwatrace --game tools/trace/spatial/spatial.rwa \
    --scenario tools/trace/scenarios/spatial-edge.scenario.json --out trace.jsonl
python3 tools/trace/spatial/check_trace.py trace.jsonl
```

`check_trace.py` re-implements the engine's spatial math independently
(haversine distance, world bearing, `calculateRelativeDirection`) and asserts
every `-distance1/-azimuth1/-elevation1` value: finite, azimuth in `[0, 360)`,
relative elevation in `[−90, 90]`, raw-head assets echo the wrapped pitch
(120, −150 unclamped), and `horizon.pd` flips by 180° in azimuth when the head
pitches past vertical. The Player replays the same scenario in
`EngineParityTests`.
