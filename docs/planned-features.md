# Planned features

Features that were investigated but deliberately deferred. Each entry records
what exists today, what is missing, and what a real implementation has to cover,
so the ground doesn't have to be re-investigated.

## Scene-level "Required Scenes" (scene-entry condition)

**Status: unfinished scaffolding, UI field hidden since v1.4.1+ (commented out
in `rwasceneattributeview.cpp`).**

### Intended behaviour (inferred)

The scene-level counterpart of the working state-level *Required States*
feature: a scene may only be entered once the hero has visited all scenes
listed as required. Presumably planned alongside the never-used
`visitedSceneConditionCount` member in `RwaSceneAttributeView`.

### What exists today

- `RwaScene::requiredScenes` (`rwascene.h`) with getter/setter, written only by
  the (now hidden) UI field's write-back handler, read by nobody.
- The Scene view's line edit "Required Scenes" (commented out): accepted a
  comma-separated list of scene names, validated them against
  `backend->getScenes()`, and stored the matches in
  `currentScene->requiredScenes`.
- A display block in `RwaSceneAttributeView::setCurrentScene` (commented out)
  that was doubly broken and never executed: it looked up the widget under the
  wrong name (`"Required States"`, a copy-paste from the State view present
  since the first commit) and read `currentState->requiredStates` — and
  `RwaView::currentState` is the *globally last-touched state* (set via
  `RwaBackend::sendLastTouchedState`), unrelated to the displayed scene. Do not
  resurrect this block as-is; it must read `currentScene->requiredScenes`.

### What is missing (the actual feature)

1. **Visited-scene tracking**: `RwaEntity` has `visitedStates` but no
   `visitedScenes`. The bookkeeping (append on scene entry, reset on game
   restart) does not exist.
2. **The gate**: `RwaRuntime::setEntityScene` switches scenes on exactly two
   conditions — equal `level` and hero inside the scene area (`rwaruntime.cpp`).
   A required-scenes check would go here, analogous to the required-states gate
   in `setEntityState` (including a decision on whether scenes get an equivalent
   of block-until-radius-left and hint behaviour).
3. **Serialisation**: the exporter writes only the state-level
   `<requiredstates>` element; there is no scene-level element in the `.rwa`
   format. Both `rwaexport.cpp`/`rwaimport.cpp` and the Player's
   `RwaImport.swift` would need a new element (e.g. `<requiredscenes>`), and
   `tools/gamefiles/validate_rwa.py` should learn it.
4. **Engine parity**: the whole gate must be mirrored in the Player's
   `RwaGameLoop.swift` (see CLAUDE.md "Engine parity"). This is what makes the
   feature a deliberate project rather than a quick fix.

Existing games are unaffected by the removal: the field's content was never
saved, and the example corpus contains only empty `<requiredstates/>` elements
and no scene-level element at all.
