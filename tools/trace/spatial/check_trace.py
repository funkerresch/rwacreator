#!/usr/bin/env python3
"""Check an rwatrace trace of scenarios/spatial-edge.scenario.json against an
independent implementation of the engine's spatial math.

    ./build/cmake-debug/rwatrace --game tools/trace/spatial/spatial.rwa \
        --scenario tools/trace/scenarios/spatial-edge.scenario.json --out trace.jsonl
    python3 tools/trace/spatial/check_trace.py trace.jsonl

Checks every -distance1 / -azimuth1 / -elevation1 event: finite, azimuth in
[0, 360), elevation in [-90, 90], value equal to the expected one (float32
tolerance), raw-head assets echo the wrapped head pitch, and the horizon source
flips by 180 deg in azimuth when the head pitches past vertical.
"""
import json
import math
import sys

R = 6373000.0
ASSETS = {  # name: (lon, lat, altitude, minDistance, fixedAzimuth, fixedDistance, relative2source)
    "horizon.pd":   (7.58600000, 47.57717981, 0, -1, -1,   -1,  True),
    "elevated.pd":  (7.58600000, 47.57717981, 5, -1, -1,   -1,  True),
    "onspot0.pd":   (7.58600000, 47.57700000, 0, -1, -1,   -1,  True),
    "onspot5.pd":   (7.58600000, 47.57700000, 5, -1, -1,   -1,  True),
    "mindist.pd":   (7.58601333, 47.57700000, 0,  3, -1,   -1,  True),
    "fixedazi.pd":  (7.58600000, 47.57717981, 0, -1, 45.5, -1,  True),
    "fixeddist.pd": (7.58600000, 47.57717981, 0, -1, -1,   2.5, True),
    "rawhead.pd":   (7.58600000, 47.57717981, 0, -1, -1,   -1,  False),
}
TOL = 2e-3  # degrees / metres, generous for the float32 wire


def wrap360(x):
    x = math.fmod(x, 360.0)
    return x + 360.0 if x < 0 else x


def wrap180(x):
    x = math.fmod(x, 360.0)
    if x > 180:
        x -= 360
    elif x <= -180:
        x += 360
    return x


def distance_m(p1, p2):
    lat1, lat2 = math.radians(p1[1]), math.radians(p2[1])
    dlon, dlat = math.radians(p2[0] - p1[0]), math.radians(p2[1] - p1[1])
    a = math.sin(dlat / 2) ** 2 + math.cos(lat1) * math.cos(lat2) * math.sin(dlon / 2) ** 2
    return R * 2 * math.atan2(math.sqrt(a), math.sqrt(1 - a))


def world_bearing(p1, p2):
    phi1, phi2 = math.radians(p1[1]), math.radians(p2[1])
    lam1, lam2 = math.radians(p1[0]), math.radians(p2[0])
    y = math.sin(lam2 - lam1) * math.cos(phi2)
    x = math.cos(phi1) * math.sin(phi2) - math.sin(phi1) * math.cos(phi2) * math.cos(lam2 - lam1)
    return wrap360(math.degrees(math.atan2(y, x)))


def relative_direction(bearing, elevation, yaw, pitch):
    b, e, psi, th = map(math.radians, (bearing, elevation, yaw, pitch))
    v = (math.cos(e) * math.sin(b), math.cos(e) * math.cos(b), math.sin(e))
    f = (math.cos(th) * math.sin(psi), math.cos(th) * math.cos(psi), math.sin(th))
    u = (-math.sin(th) * math.sin(psi), -math.sin(th) * math.cos(psi), math.cos(th))
    r = (math.cos(psi), -math.sin(psi), 0.0)
    dot = lambda a, b: sum(x * y for x, y in zip(a, b))
    xf, xr, xu = dot(v, f), dot(v, r), dot(v, u)
    return wrap360(math.degrees(math.atan2(xr, xf)) + 180), math.degrees(math.atan2(xu, math.hypot(xf, xr)))


def expected(name, pos, yaw, pitch):
    lon, lat, alt, mind, fixaz, fixd, rel = ASSETS[name]
    if not rel:
        return None, yaw, pitch
    if fixd >= 0:
        d = fixd
    else:
        d = distance_m(pos, (lon, lat))
        if mind >= 0:
            d = max(d, mind)
    world_el = math.degrees(math.atan2(alt, d))
    total = math.hypot(d, alt)
    if fixaz >= 0:
        return total, wrap360(fixaz), world_el
    az, el = relative_direction(world_bearing(pos, (lon, lat)), world_el, yaw, pitch)
    return total, az, el


def exp_degenerate(name, pos, yaw, pitch, recv):
    lon, lat, alt, mind, fixaz, fixd, rel = ASSETS[name]
    if not rel or fixaz >= 0:
        return False
    d = fixd if fixd >= 0 else distance_m(pos, (lon, lat))
    if fixd < 0 and mind >= 0:
        d = max(d, mind)
    if d < 1e-6:
        # World bearing undefined (the engine's channel coord carries ~1e-10 m of rounding from
        # calculateDestination1). With altitude 0 the whole direction is undefined; with altitude
        # the source is straight up and only the azimuth is undefined.
        return alt == 0 or recv == "azimuth1"
    if recv != "azimuth1":
        return False
    world_el = math.degrees(math.atan2(alt, d))
    _, el = relative_direction(world_bearing(pos, (lon, lat)), world_el, yaw, pitch)
    return abs(el) > 89.99  # gimbal lock


def main(path):
    pos, yaw, pitch = None, 0.0, 0.0
    errors, checked = [], 0
    horizon_az = {}  # pitch -> azimuth seen for horizon.pd at yaw 0
    for line in open(path):
        ev = json.loads(line)
        if ev.get("ev") == "input":
            k = ev.get("kind")
            if k == "pos":
                pos = (ev["lon"], ev["lat"])
            elif k == "azimuth":
                yaw = wrap360(float(ev["value"]))
            elif k == "elevation":
                pitch = wrap180(float(ev["value"]))
            continue
        if ev.get("ev") != "pd" or "val" not in ev:
            continue
        recv = ev["recv"].split("-", 1)[1]
        if recv not in ("distance1", "azimuth1", "elevation1"):
            continue
        name = ev.get("asset")
        if name not in ASSETS or pos is None:
            continue
        val = ev["val"]
        checked += 1
        where = f"t={ev.get('t')} {name} {recv}={val}"
        if not math.isfinite(val):
            errors.append(f"{where}: not finite")
            continue
        if recv == "azimuth1" and not (0 <= val < 360):
            errors.append(f"{where}: azimuth outside [0,360)")
        exp_d, exp_az, exp_el = expected(name, pos, yaw, pitch)
        if ASSETS[name][6] and recv == "elevation1" and not (-90 <= val <= 90):
            errors.append(f"{where}: relative elevation outside [-90,90]")
        exp = {"distance1": exp_d, "azimuth1": exp_az, "elevation1": exp_el}[recv]
        if exp is None:
            continue
        if recv in ("azimuth1", "elevation1") and exp_degenerate(name, pos, yaw, pitch, recv):
            continue  # direction undefined: listener on the source (both angles if altitude 0), or gimbal lock (azimuth)
        diff = abs(val - exp)
        if recv == "azimuth1":
            diff = min(diff, 360 - diff)
        if diff > TOL:
            errors.append(f"{where}: expected {exp:.4f} (yaw {yaw}, pitch {pitch})")
        if name == "horizon.pd" and recv == "azimuth1" and yaw == 0:
            horizon_az.setdefault(pitch, val)
    for p in (120.0, -150.0):
        if p in horizon_az and 0.0 in horizon_az:
            flip = wrap360(horizon_az[p] - horizon_az[0.0])
            if abs(flip - 180) > TOL:
                errors.append(f"horizon.pd azimuth at pitch {p} is {horizon_az[p]}, expected flip by 180 from {horizon_az[0.0]}")
        else:
            errors.append(f"no horizon.pd azimuth seen at pitch {p}")
    for e in errors[:40]:
        print("FAIL", e)
    print(f"{checked} values checked, {len(errors)} errors")
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1] if len(sys.argv) > 1 else "trace.jsonl"))
