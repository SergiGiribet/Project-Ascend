# Backlog: parked ideas and saved material

## Future ideas (Phase 1+)

- **Trait/skill fusion through experience**: traits and skills could merge or transform based
  on what a unit lives through in the incursions (e.g. a Cowardly unit that survives enough
  times becomes something else). Idea from 2026-07-16; it pairs well with the contradictory
  traits the generator already produces (Cowardly + Brave) — they would be starting material,
  not a bug.
- **Epithets for repeated names** (GDD): the real fix for name collisions; enlarging the bank
  (done: 20 -> 60 names) only reduces their frequency, it does not remove them (birthday paradox).
- **Healing through items**: out of scope for Phase 0 (drags in inventory/drops). The Phase 0
  dial was "rest between incursions", implemented on 2026-07-17.
- **Third `Encounter` field** (`description|cause|resolution`): narrative flavor when a floor
  is cleared successfully — see the material below.
- **Varied trait-event deeds**: `deed` becomes a `vector<string>` with 2-3 variants per trait,
  so "Osric, Brave as ever, holds the line steady." doesn't repeat every other floor.
- **Self-bootstrapping exe**: embed default banks (raw string literals) and create
  `resources/`/`sessions/` on first run, writing only missing files. Deferred: a GitHub
  release zip covers distribution without a second source of truth.

## Saved encounter resolutions

These texts were originally written as the second field of `encounters.txt`, but they
described *how the encounter is overcome*, not how a unit dies there (the field is a death
cause: it must complete `fell on floor N, <cause>.` with the unit as the subject).

They are kept here because they are good material for a possible **third `Encounter` field**
(e.g. `resolution`: narrative flavor when the floor is cleared) — Phase 1 territory.

Proposed format if implemented: `description|cause|resolution` (the loader would do two `find('|')`).

| Encounter | Saved resolution |
|---|---|
| A shattered mirror shows your face twisted in rage | shattered into a thousand regrets |
| A wailing banshee drifts through the corridor | silenced by a ringing charm |
| Columns of living ivy reach for your throat | cut back with a blade of silver |
| A caravan of skeletal merchants peddles empty boxes | bartered away for a memory |
| A clockwork hound patrols the hall, whirring teeth bared | overloaded and frozen in place |
| An overturned altar bleeds black oil into the stone | cleansed with salt and sunlight |
| A chorus of disembodied children hums a lullaby | calmed by a borrowed toy |
| Glass butterflies swarm from a cracked lantern | melted into a single glowing bead |
| A statue of a hero steps down from its plinth | bowed and left to rest |
| Roots burst from the floor and drag you under | pried apart with a rusted key |
| An old map flutters from the ceiling, ink crawling away | captured in a silver jar |
| Two-faced merchants argue over the price of shadows | sold to the highest whisper |
| Fog-filled alcoves hide hungry eyes | scattered by a torch of ironwood |
| Candles burn in reverse, flames sinking to the wick | upright and relit by a steady hand |
| A ruined library coughs up drifting pages | stitched back into a new grimoire |
| Marionette knights perform a macabre waltz | strings cut and left to clatter |
| A pool reflects a city that never was | stepped into and left a token behind |
| An urn screams when opened, spilling starlight | bottled and labeled "a wish" |
| Gargoyles gossip from the eaves about your past | bribed into silence with a borrowed name |
| An iron bridge hums with trapped voices | tuned until the chorus fades |
| A butchered feast rises as runes glow on the table | turned to ash by a cooling spell |
| Wind-carved runes rearrange themselves into a door | opened and sealed on the other side |
| A pair of glowing eyes watches from the rafters | found to be a child's lantern on a string |
| A field of frost flowers shatters beneath your boots | kept frozen in a glass vial |
| A hungry echo repeats your worst secret | fed a lie until it forgot |
| The moonlight pools into a sleeping wolf | woken and befriended with a scrap of meat |
