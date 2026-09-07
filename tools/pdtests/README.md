# pdtests: headless Pd tests for the player patches

## damping_test.py: distance -> gain, late reflections, elevation guard

Numeric test of the control-rate chains that turn the engine's per-tick
`-distanceN` / `-elevationN` floats into audio parameters. It instruments a
*copy* of a patch ($0 -> TESTID, `[print]` taps on every `$0-distancescalingN`,
on the `wet $1` message of every `latereflections*` subpatch and on whatever
feeds each `pd binaural256vs` elevation inlet), sends the exact
`sendInitValues2pd` damping values (factor 30, trim 2, min 0, max 1, smoothdist
10), then steps the distance through `0, 0.005, 0.01, 0.5, 1, 10, 100, 370, 500,
3000` under damping function 1 (exponential), 2 (linear) and 0 (off), and the
elevation through `90, -95, 45.5, 0`. Every value is compared with the closed
form the patch is supposed to implement:

| Chain | Expected |
| --- | --- |
| exponential | `clip(trim · (d+1)^(−factor/20), min, max)`, with the defaults `min(1, 2·(d+1)^−1.5)` |
| linear | `clip(trim / max(d, 0.01), min, max)` |
| off | `1` on **every** channel |
| late reflections | `wet = min(1, 0.0003 · (d+1)^1.5)` |
| elevation | `clip(e, −90, 89)` (the HRTF external ignores exactly 90) |

plus: no `divide by zero` line on the Pd console.

The engine contract is `distance ≥ 0` (haversine distance, the `minDistance`
/ `fixedDistance` sentinels are never sent, `hypot` with the altitude), in
every Creator and Player version, so the patches need no guard against
negative distances and the test has no negative case.

```bash
python3 tools/pdtests/damping_test.py                       # all six *_fabian patches
python3 tools/pdtests/damping_test.py puredata/rwaloopplayermono.pd -v
python3 tools/pdtests/damping_test.py --expect-broken       # self-check: passes when something fails
```

Needs Pd at `/Applications/Pd-0.56-5.app` (`--pd` to override); the externals
may fail to create under `-nogui -noaudio`, the chains under test are pure
control objects. Exit 0 = pass.

### What the unguarded patches get wrong (September 2026)

Against the patches as shipped the test reports, per channel:

- linear damping at `d = 0`: **0** (Pd's `expr` divide-by-zero guard): the
  source goes silent exactly at the closest point, `divide by zero` on the
  console. Reachable by default: `minDistance` is −1 and mono/stereo force
  `channelRadius` 0.
- late reflections: `wet` 2.14 at 370 m and 3.36 at 500 m (unbounded), then
  **0** from ~2153 m on (the clipped gain reaches 0 -> `1/0`).
- damping function 0 on the 5.1 and 7‑channel patches: only
  `$0-distancescaling1` receives `1`; channels 2..N stay at `[*~ 0]`: muted.
- elevation 90 / −95 reach the external unclipped (ignored there, so the last
  direction sticks).

### The patch edits that make it pass

Apply in the Pd GUI (Pd 0.56), save, then run the test; copy the saved files
to `../rwa-player/pd-patches/` unchanged (the six `*_fabian` patches and the
four `rwaloopplayer*` patches are byte-identical between the repos). The
engine guarantees finite values, distance ≥ 0 and azimuth in `[0, 360)`; the
patch guards the singularities that lie inside that contract (distance 0,
elevation exactly 90).

1. `pd calculatedamping`, linear branch, every copy (mono: 1, stereo: 2,
   5.1: 5, 7‑ch: 7, loop players: 1): insert `[max 0.01]` on the *distance*
   somewhere before `[expr (1 / $f1)]`, so the unit is
   metres: the smoothed distance is floored at 1 cm before the reciprocal.
2. `pd latereflections*`, every copy: replace
   `[+ 1] -> [expr abs(30 * log10($f1))] -> [expr (100 - $f1)] -> [dbtorms] -> [clip 0 1] -> [expr (1 / $f1) * (0.0003)]`
   by `[expr min(1, 0.0003 * pow($f1 + 1, 1.5))]` into the existing
   `[wet $1(`. Identical below ~370 m, saturates at 1, no division.
3. `pd selectdamping` (5.1 and 7‑channel patches): connect the `[1(` message
   of the `route 1 2 0` third outlet (function 0) additionally to new
   `[s $0-distancescaling2]` … `[s $0-distancescalingN]`.
4. Inside each `pd binaural256vs` wrapper, insert `[clip -90 89]` between the
   elevation `[inlet]` and `rwa_binauralsimple~`'s inlet 2. This is cheap
   protection for one edge case: a state activated while the listener stands
   directly under an elevated asset sends exactly 90 as the first value, the
   external ignores it and stays at ear level until the listener moves. The
   azimuth chain `[expr 360 - $f1] -> [+ 180]` stays as it is.
5. `puredata/rwaReferencePatch.pd` comment: distance ≥ 0 m float, azimuth
   `[0, 360)` float, elevation `[−90, 90]` float; raw-head mode
   (`headtrackerrelative2source` off) gets the head pitch in `(−180, 180]`.

Textual editing is possible (append new objects after the last `#X obj` of
the subpatch so existing indices stay valid, then rewrite only the affected
`#X connect` lines), but with 18 `calculatedamping` copies the GUI is the
safer route.

