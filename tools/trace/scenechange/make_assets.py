#!/usr/bin/env python3
"""Generate the four silent 60 s / 48 kHz mono WAVs the scenechange fixture
games reference (not committed: ~23 MB of silence). Run once before using the
scenechange scenarios:

    python3 tools/trace/scenechange/make_assets.py
"""
import wave
from pathlib import Path

assets = Path(__file__).parent / "assets"
assets.mkdir(exist_ok=True)

for name in ["bg0", "bg1", "fb0", "fb1"]:
    path = assets / f"{name}.wav"
    with wave.open(str(path), "w") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(48000)
        w.writeframes(b"\x00\x00" * 48000 * 60)
    print("wrote", path)
