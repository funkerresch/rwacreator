# scenechange - scene-transition fixture games

Two-scene fixture games for the scene-switching regressions fixed in August
2026 (see CHANGELOG, "Background assets of a left scene no longer keep playing
forever"). Both scenes have a FALLBACK and a BACKGROUND state with one looping
mono asset each (`alwaysplayfrombeginning="0"`, `fadeout="1000"`).

- `twoscene.rwa`: circle areas radius 100 m, centers ~300 m apart (disjoint).
- `overlap.rwa`: same, radius 300 m: the areas overlap between the centers.
- `crossscene-required.rwa`: `twoscene.rwa` plus a GPS state per scene:
  `KeyState` (Scene 0) and `Door` (Scene 1), where `Door` **requires**
  `KeyState`: a cross-scene required state (reuses the twoscene WAVs).

The referenced WAVs are generated (not committed), run once:

```bash
python3 tools/trace/scenechange/make_assets.py
```

Scenarios (in `../scenarios/`, run with `--game tools/trace/scenechange/<game>`):

| Scenario | Game | Checks |
| --- | --- | --- |
| `scenechange.scenario.json` | twoscene | clean walk A→B→A: `-end` on scene exit, playhead resume (`-playheadposition` ≠ 0) on re-entry after the fade completed |
| `reenter-during-fade.scenario.json` | twoscene | re-entry *within* the fade-out window: the superseded instance's patcher must still be released (pending-release list) and the new instance must be tracked, i.e. it receives `-end` on the next scene exit. Pre-fix, the new instance was orphaned and looped forever |
| `overlap.scenario.json` | overlap | hero stands in the overlap of both areas: the scene must *not* switch (pre-fix: one switch and two `-play`s per 25 ms tick until the patcher pool was exhausted) |
| `cross.scenario.json` | overlap | walking through the overlap into the far scene: exactly one switch, one `-end`/`-play` pair per asset |
| `crossscene-required.scenario.json` | crossscene-required | visit `KeyState` in Scene 0, then walk into `Door` (Scene 1): `Door` must be entered: required states resolve against the game-wide `visitedStates` list, not the current scene |
| `crossscene-required-unmet.scenario.json` | crossscene-required | skip `KeyState`, stand in `Door`'s area: `Door` must **not** be entered |
