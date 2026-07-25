# Technical documentation — Project Ascend · Phase 1

State of the code as of 2026-07-25. This documents what Phase 0.5 and Phase 1 **added or
changed** on top of the [Phase 0 baseline](technical-documentation.md); read that first for the
class map, ownership principle and conventions (all still hold). See also the
[style guide](style-guide.md) and the [backlog](backlog.md) (which holds the design rationale,
playtest data and the parked ideas).

---

## 1. What changed, at a glance

| Area | Change |
|---|---|
| `Incursion` | Residual **incident** on floors cleared "with ease"; **weighted** victim lottery (`riskDirection`/`pickWeightedVictim`) replacing the old forced-Reckless wound; wounds can leave a **permanent injury**; trait deeds now vary (`deeds` bank, one picked at random) instead of a single fixed line per trait. |
| New `Injury` | `Injury` struct + `loadInjuries` + `applyInjury` + `injuryLabel` (shared display format); new `injuries.txt` bank. |
| New `TrainingCamp` | Trainers/trainees leveling between incursions; essence sink (slots); once-per-incursion `tick`. |
| `Unit` | Separate `injuries` list stored as `std::vector<Injury>` (carries each injury's stat cost), apart from `skills`, with `addInjury`/`getInjuries`; roster and card show injuries in red **with their cost**. |
| `Roster` | `healAll()` (whole-roster rest); role-colored roster (team/trainer/trainee) + injuries shown in red. |
| `Utils` | `COLOR_MAGENTA`, `COLOR_BLUE` added. |
| `main` | New "Manage the training camp" menu; camp `tick` + `healAll` wired into the incursion cycle. |
| Rename | `TRAIT_EVENTS` → `DAMAGE_TRAIT_EVENTS` (the combat-modifier table; leaves the name free for other trait systems). |

---

## 2. Residual incident (Incursion.cpp)

The Phase 0 tower gave a floor cleared "with ease" **zero** risk — once strong enough, farming
was risk-free and no jeopardy ever returned to safe floors. Phase 0.5 adds a small residual
incident on those clears so the tower is never a certainty.

New file-local constants: `BASE_INCIDENT_CHANCE` (3), `DEPTH_INCREMENT` (0.3 %/floor),
`INCIDENT_CAP` (18), `FATAL_CHANCE` (18), `WOUND_INJURY_CHANCE` (8),
`VICTIM_WEIGHT_BONUS`/`VICTIM_WEIGHT_PENALTY`.

Helpers (all file-local `static`, like `teamPower`):
- `riskDirection(skills)` → `+1` if the unit has `Reckless`/`Boaster`, `-1` if `Alert`, else `0`.
  The single source of truth for "does this trait raise or lower risk", shared by the two
  functions below.
- `pickWeightedVictim(team, roster, rng)` — a `std::discrete_distribution` victim lottery: base
  weight 1, `+VICTIM_WEIGHT_BONUS` for a risk-raising trait, `−VICTIM_WEIGHT_PENALTY` (min 1) for
  Alert. **Never forced** — replaces the old `forcedVictimId` (Reckless was 100 % the victim).
  Used by both the difficulty-tier wound and the incident.
- `incidentChance(floor, team, roster)` → `BASE + floor*DEPTH_INCREMENT`, `+5`/`−3` per
  risk-raising/lowering member, clamped to `INCIDENT_CAP`.
- `pickIncidentFlavor(victim, rng)` → draws from `BOASTER_INCIDENTS` (ironic, if the victim is a
  Boaster) or `NEUTRAL_INCIDENTS` (a freak accident), each an `{cause, woundLine}` pair.

Flow on an ease-tier clear: roll `incidentChance`; if it hits, `pickWeightedVictim`, then a
`FATAL_CHANCE` roll splits fatal vs. wound, with its own flavor text so a death here reads as a
freak accident, not an encounter death.

**Permanent injury on survived wounds**: at both wound points (difficulty tier and incident
tier), a survived wound has a `WOUND_INJURY_CHANCE` (~8 %) chance to also leave a permanent
injury via `applyInjury` (see §3).

## 3. Permanent injuries (`Injury.h` / `Injury.cpp`, `injuries.txt`)

```
struct Injury { std::string name; int strPenalty; int conPenalty; };
```

- `loadInjuries(path)` — mirrors `loadEncounters` but splits **two** `|` (`name|str|con`);
  throws if a line lacks three fields or the bank is empty.
- `applyInjury(unit, bank, rng)` → **returns the applied injury's name**. Picks a random injury;
  subtracts STR/CON (floored at 1); adds the `Injury` to the unit's **injuries** list (not skills);
  and if the unit has `Reckless`/`Boaster`, 50 % chance it swaps that trait for `Alert` (the
  lesson learned the hard way). Permanent: injuries are **never healed** (unlike ordinary wounds).
- `injuryLabel(injury)` → the shared display format: the name plus its non-zero penalties in
  parentheses, e.g. `One-Handed (-3 STR)`, `Broken Ribs (-2 STR, -3 CON)` (parenthetical omitted
  if both penalties are zero). Used by both the roster and the unit card, so an injury's cost is
  legible wherever a unit is shown — the display never needs the injury bank on hand.
- `injuries.txt`: `name|strPenalty|conPenalty`, e.g. `One-Handed|3|0`, `Shattered Knee|0|3`.

## 4. `Unit` — injuries kept apart from traits, and carrying their cost

Private `std::vector<Injury> injuries` (separate from `skills`), with `addInjury(const Injury&)`
and `getInjuries()`. Storing the whole `Injury` (not just its name) lets any display show the
**stat cost** without needing the injury bank on hand. Keeping injuries apart from `skills` means
`riskDirection` and the incident/death flourish only see real personality traits, and the display
can treat injuries distinctly. `printUnit()` prints an `Injuries:` line in **red** (`COLOR_RED`)
when the unit carries any, each formatted by the shared `injuryLabel` helper (§3).

Because `Unit` now holds `Injury` by value, `Unit.h` includes `Injury.h`; to avoid an include
cycle, `Injury.h` **forward-declares** `class Unit` (its free functions only take a `Unit&`, so a
declaration suffices) and `Injury.cpp` includes `Unit.h` for the full definition.

## 5. `TrainingCamp` (`TrainingCamp.h` / `TrainingCamp.cpp`)

A bench where units level passively between incursions instead of climbing — the answer to the
death→dilution problem (fresh Lv1 replacements dragging a team down) and a real essence sink.
Like `Team`, it stores **ids, never units**, resolved through the `Roster`.

Internals: `std::vector<Assignment>` where `Assignment = {trainerId, vector<int> traineeIds}`;
`purchasedSlots_` (starts at `STARTING_SLOTS` = 1). `TRAINEES_PER_TRAINER` = 5.

- A **trainer** is pulled out of the active `Team` (real trade-off); **trainees** live outside
  `Team::MAX_MEMBERS`.
- `nextSlotCost()` = `250 * 2^(purchasedSlots_-1)` (250, 500, 1000…); `buySlot()` only bumps the
  count — **the menu charges the essence** (Camp never touches `GameState`).
- `addTrainer` / `assignTrainee` throw `std::runtime_error` on bad input (menu wraps in try/catch,
  like `Team::addMember`); `removeTrainer`/`removeTrainee`/`purgeDead` use erase–remove.
- `tick(roster, injuries, rng)` — **once per incursion**: each trainee gains
  `trainerLevel * 15 * (1 + (trainerRace-1)*0.15)` XP (trainers gain none); a small training
  incident may strike (2 % trainee, 0.3 % trainer, via a 1..1000 roll), 15 % of which is a
  permanent injury, the rest a non-fatal scrape (HP floored at 1). **Never fatal.**
- `print(roster)` — compact listing (trainer then indented trainees) + free slots and next cost.

## 6. `Roster` additions

- `healAll()` — heals **every** unit to full. Called at the end of each incursion so the camp's
  transient training wounds heal too (permanent injuries and the dead stay). Replaces the old
  team-only heal loop.
- `printRoster(teamIds, trainerIds, traineeIds)` — each unit colored by role (mutually exclusive,
  in order): team → cyan `[in team]`, trainer → magenta `[trainer]`, trainee → blue `[trainee]`;
  and any injuries appended in **red** `{…}`.

## 7. `main.cpp` wiring

- New main-menu option **3 “Manage the training camp”** (tower moved to 4): a submenu to assign a
  trainer (and pull them from the team), assign trainees, dismiss either, buy a slot (with an
  essence check), and view the camp. Same nested-menu + local-try/catch pattern as team management.
- Incursion cycle (tower → start): guarded on a non-empty team, then
  `runIncursion(...)` → `tcamp.tick(roster, injuries, rng)` → `roster.healAll()`. The guard keeps
  an empty-team entry from training the camp for free.

## 8. Evaluation (2026-07-23)

An automated bot exercised the camp over many sessions (full table and analysis in the
[backlog](backlog.md)). Headline: the camp **eliminates the death-spiral collapse** (0 wipes;
every death backfilled from the bench) and gives essence a sink, but does **not** raise the
tower ceiling — aggressive play plateaus ~13-15, safe play ~17-19, and nobody reached floor 25.
The ceiling is the difficulty-curve limit (`danger = 20 + 15*floor` outpacing team power), not a
camp problem — the next lever for high floors is a power/difficulty rebalance, not more camp.
