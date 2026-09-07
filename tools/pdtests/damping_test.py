#!/usr/bin/env python3
"""Headless numeric test of the distance -> gain ("damping") chain, the late
reflection send and the elevation guard in the RWA player patches.

Drives a copy of a player patch in Pd (no audio device, externals may
fail to create, the chains under test are control-rate) with the exact
init protocol RwaRuntime::sendInitValues2pd sends, steps `-distanceN` through
a list of values under each damping function, and compares every
`$0-distancescalingN` value, the `wet` message of every `latereflections*`
subpatch and the value reaching `rwa_binauralsimple~`'s elevation inlet inside
each `pd binaural256vs` wrapper with the closed forms the patches are supposed
to implement:

  exponential (1):  g = clip(trim * (d + 1) ^ (-factor/20), min, max)
  linear      (2):  g = clip(trim / max(d, 0.01), min, max)
  none        (0):  g = 1 on EVERY channel (not only channel 1)
  late reflections: wet = min(1, 0.0003 * (d + 1) ^ 1.5)
  elevation:        clip(e, -90, 89)   (the HRTF external rejects exactly 90;
                    the [clip] lives inside pd binaural256vs)

Distances are >= 0 by engine contract (haversine, minDistance/fixedDistance
sentinels are never sent, hypot with the altitude), so no negative case.

and fails on any "divide by zero" Pd console line. The patch on disk is not
modified: a copy is instrumented ($0 -> TESTID, print taps) in a temp dir.

Usage:
    python3 tools/pdtests/damping_test.py [patch.pd ...]      # default: all six *_fabian patches
    python3 tools/pdtests/damping_test.py --expect-broken puredata/rwaplayermonobinaural_fabian.pd

--expect-broken passes when at least one check fails (self-check against the
patches before the guards were added).
"""
import argparse
import re
import subprocess
import sys
import tempfile
from pathlib import Path

PD_DEFAULT = "/Applications/Pd-0.56-5.app/Contents/Resources/bin/pd"
REPO = Path(__file__).resolve().parents[2]
DEFAULT_PATCHES = [
    "rwaplayermonobinaural_fabian.pd", "rwaplayermonobinauralogg_fabian.pd",
    "rwaplayerstereobinaural_fabian.pd", "rwaplayerstereobinauralogg_fabian.pd",
    "rwaplayer5_1channelbinaural_fabian.pd", "rwaplayer7channelbinaural_fabian.pd",
]

FACTOR, TRIM, DMIN, DMAX, SMOOTH = 30.0, 2.0, 0.0, 1.0, 10.0
DISTANCES = [0, 0.005, 0.01, 0.5, 1, 10, 100, 370, 500, 3000]   # the engine contract is distance >= 0
FUNCTIONS = [1, 2, 0]
ELEVATIONS = [90, -95, 45.5, 0]
STEP_MS = 150
TOL = 2e-4


def clip(x, lo, hi):
    return max(lo, min(hi, x))


def expected_gain(func, d):
    if func == 1:
        return clip(TRIM * (d + 1) ** (-FACTOR / 20), DMIN, DMAX)
    if func == 2:
        return clip(TRIM / max(d, 0.01), DMIN, DMAX)
    return 1.0


def expected_wet(d):
    return min(1.0, 0.0003 * (d + 1) ** 1.5)


def expected_elevation(e):
    return clip(e, -90, 89)


def instrument(patch_path: Path, out_path: Path):
    """Return (channel count, latereflections count, binaural count)."""
    content = patch_path.read_text().replace("\\$0", "TESTID")
    records = re.split(r";\s*\n", content.rstrip())
    if records and records[-1].endswith(";"):
        records[-1] = records[-1][:-1]

    n_channels = len(set(re.findall(r"distancescaling(\d+)", content)))
    out = []
    stack = []          # [name, objrecords, connects]
    late_count = 0
    binaural_count = 0

    for rec in records:
        if rec.startswith("#N canvas"):
            parts = rec.split()
            name = parts[6] if len(parts) >= 8 else "__root__"  # "#N canvas x y w h name 0"
            stack.append([name, [], []])
            out.append(rec)
            continue
        if rec.startswith("#X restore"):
            name, objs, conns = stack[-1]
            count = len(objs)
            if name.startswith("latereflections"):
                late_count += 1
                wet_idx = next(i for i, r in enumerate(objs) if r.startswith("#X msg") and r.endswith("wet \\$1"))
                out.append(f"#X obj 10 10 print WET{late_count}")
                out.append(f"#X connect {wet_idx} 0 {count} 0")
            elif name == "binaural256vs":
                # Tap whatever feeds the external's elevation inlet (inlet 2) -
                # the [clip -90 89] sits inside this wrapper.
                binaural_count += 1
                ext_idx = next(i for i, r in enumerate(objs) if " rwa_binauralsimple~" in r)
                out.append(f"#X obj 10 10 print EL{binaural_count}")
                for (src, outlet, dst, inlet) in conns:
                    if dst == ext_idx and inlet == 2:
                        out.append(f"#X connect {src} {outlet} {count} 0")
            stack.pop()
            stack[-1][1].append(rec)
            out.append(rec)
            continue
        if re.match(r"#X (obj|msg|text|floatatom|symbolatom|listbox)", rec):
            stack[-1][1].append(rec)
        elif rec.startswith("#X connect"):
            stack[-1][2].append(tuple(int(v) for v in rec.split()[2:6]))
        out.append(rec)

    n = len(stack[0][1])
    extra, conns = [], []

    def add(rec):
        extra.append(rec)
        return n + len(extra) - 1

    # taps
    for k in range(1, n_channels + 1):
        r = add(f"#X obj 10 10 r TESTID-distancescaling{k}")
        p = add(f"#X obj 10 40 print DS{k}")
        conns.append((r, 0, p, 0))
    step_print = add("#X obj 10 100 print STEP")

    # driver
    lb = add("#X obj 10 130 loadbang")
    t = 100
    init = (f"#X msg 200 130 \\; TESTID-dampingfactor {FACTOR:g} \\; TESTID-dampingtrim {TRIM:g}"
            f" \\; TESTID-dampingmin {DMIN:g} \\; TESTID-dampingmax {DMAX:g} \\; TESTID-smoothdist {SMOOTH:g}")
    d0 = add(f"#X obj 10 160 delay {t}")
    m0 = add(init)
    conns += [(lb, 0, d0, 0), (d0, 0, m0, 0)]

    def step(marker, sends):
        nonlocal t
        t += STEP_MS
        d = add(f"#X obj 10 190 delay {t}")
        m = add("#X msg 200 190 " + " ".join(f"\\; {s}" for s in sends))
        mk = add(f"#X msg 400 190 symbol {marker}")
        conns.extend([(lb, 0, d, 0), (d, 0, mk, 0), (mk, 0, step_print, 0), (d, 0, m, 0)])

    for f in FUNCTIONS:
        step(f"FUNC{f}", [f"TESTID-dampingfunction {f}"])
        for dist in DISTANCES:
            step(f"D{dist:g}", [f"TESTID-distance{k} {dist:g}" for k in range(1, n_channels + 1)])
    for e in ELEVATIONS:
        step(f"E{e:g}", [f"TESTID-elevation{k} {e:g}" for k in range(1, n_channels + 1)])
    t += STEP_MS
    dq = add(f"#X obj 10 220 delay {t}")
    mq = add("#X msg 200 220 \\; pd quit")
    conns += [(lb, 0, dq, 0), (dq, 0, mq, 0)]

    body = ";\n".join(out) + ";\n" + ";\n".join(extra) + ";\n"
    body += "".join(f"#X connect {a} {ao} {b} {bi};\n" for a, ao, b, bi in conns)
    out_path.write_text(body)
    return n_channels, late_count, binaural_count


def run(pd_bin, patch):
    proc = subprocess.run([pd_bin, "-nogui", "-noaudio", "-open", str(patch)],
                          capture_output=True, text=True, timeout=60, cwd=str(patch.parent))
    return (proc.stdout + proc.stderr).splitlines()


def analyse(lines, n_channels, n_late, n_binaural):
    """Group print output by STEP marker -> {marker: {tag: [values]}}."""
    steps, cur = [], None
    dbz = [ln for ln in lines if "divide by zero" in ln]
    for ln in lines:
        m = re.match(r"STEP: symbol (\S+)", ln)
        if m:
            cur = (m.group(1), {})
            steps.append(cur)
            continue
        m = re.match(r"(DS\d+|WET\d+|EL\d+): (?:wet )?(-?[\d.e+-]+)", ln)
        if m and cur is not None:
            cur[1].setdefault(m.group(1), []).append(float(m.group(2)))
    checks = []  # (name, ok, detail)
    func = None
    for marker, vals in steps:
        if marker.startswith("FUNC"):
            func = int(marker[4:])
            if func == 0:
                for k in range(1, n_channels + 1):
                    got = vals.get(f"DS{k}", [])
                    checks.append((f"function 0: channel {k} receives 1", got[-1:] == [1.0], f"got {got}"))
        elif marker.startswith("D"):
            d = float(marker[1:])
            for k in range(1, n_channels + 1):
                if func == 0:
                    break  # damping off: distance steps send nothing, checked at the FUNC0 step
                got = vals.get(f"DS{k}", [])
                exp = expected_gain(func, d)
                ok = bool(got) and abs(got[-1] - exp) <= TOL
                checks.append((f"function {func}, distance {d:g}: channel {k} gain {exp:.5f}", ok, f"got {got[-1] if got else None}"))
            for k in range(1, n_late + 1):
                got = vals.get(f"WET{k}", [])
                exp = expected_wet(d)
                ok = bool(got) and abs(got[-1] - exp) <= TOL
                checks.append((f"distance {d:g}: latereflections{k} wet {exp:.5f}", ok, f"got {got[-1] if got else None}"))
        elif marker.startswith("E"):
            e = float(marker[1:])
            for k in range(1, n_binaural + 1):
                got = vals.get(f"EL{k}", [])
                exp = expected_elevation(e)
                ok = bool(got) and abs(got[-1] - exp) <= TOL
                checks.append((f"elevation {e:g}: binaural {k} receives {exp:g}", ok, f"got {got[-1] if got else None}"))
    checks.append(("no 'divide by zero' on the Pd console", not dbz, f"{len(dbz)} lines"))
    return checks, len(steps)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("patches", nargs="*")
    ap.add_argument("--pd", default=PD_DEFAULT)
    ap.add_argument("--expect-broken", action="store_true", help="pass iff at least one check fails")
    ap.add_argument("-v", "--verbose", action="store_true", help="print passing checks too")
    args = ap.parse_args()
    patches = [Path(p) for p in args.patches] or [REPO / "puredata" / p for p in DEFAULT_PATCHES]

    exit_code = 0
    for patch in patches:
        with tempfile.TemporaryDirectory() as td:
            inst = Path(td) / patch.name
            n_ch, n_late, n_bin = instrument(patch, inst)
            lines = run(args.pd, inst)
        checks, n_steps = analyse(lines, n_ch, n_late, n_bin)
        if n_steps == 0:
            print(f"{patch.name}: driver did not run (no STEP output); Pd said:\n  " + "\n  ".join(lines[-10:]))
            exit_code = 1
            continue
        failed = [c for c in checks if not c[1]]
        print(f"{patch.name}: {n_ch} channel(s), {n_late} latereflections, {n_bin} binaural; "
              f"{len(checks) - len(failed)}/{len(checks)} checks pass")
        for name, ok, detail in checks:
            if not ok or args.verbose:
                print(("   PASS  " if ok else "   FAIL  ") + f"{name}  ({detail})")
        if args.expect_broken:
            good = bool(failed)
            print("   " + ("PASS" if good else "FAIL") + "  expected-broken: at least one check fails")
        else:
            good = not failed
        exit_code |= 0 if good else 1
    sys.exit(exit_code)


if __name__ == "__main__":
    main()
