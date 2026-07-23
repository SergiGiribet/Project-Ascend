# Technical documentation — Project Ascend · Phase 0

State of the code as of 2026-07-17. Complements the GDD and the Phase 0 spec: those hold the
*what* and the *why*; this holds *how what is already built works*. See also the
[text style guide](style-guide.md) and the [idea backlog](backlog.md).

> **Status (2026-07-23):** Phase 0 is complete and shipped (GitHub + itch.io); step 8 (play &
> evaluate) was carried out and validated the permadeath hypothesis. This document is the
> **Phase 0 baseline** and describes the code as it stood at 2026-07-17 — some class sections
> below have since been extended. The Phase 1 additions (residual incident, permanent injuries,
> training camp) are documented separately in
> [technical-documentation-phase1.md](technical-documentation-phase1.md).

---

## 1. Overview

```
main.cpp ──── intro + menu loop + SessionLog (session recording)
   │
   ├── Roster ─── single owner of every living Unit (vector<Unit>)
   │                └── Unit ─── the character (contains Stats)
   │
   ├── Team ────── ids of the selected units (vector<int>) → resolved through the Roster
   │
   ├── Generator ─ builds units from the banks; consults the Necropolis for hooks
   │
   ├── Incursion ─ the tower: encounters, trait events, XP, wounds and permadeath
   │
   ├── Necropolis ─ records of the fallen (DeathRecord), never units
   │
   ├── GameState ── floor record + incursion counter + the Necropolis
   │
   └── Logger ───── TeeBuf/SessionLog: mirrors all cout to sessions/session_*.log

app/resources/ ── 8 text banks for generation and encounters
app/sessions/ ─── one .log per session (git-ignored; see its README)
```

**Ownership principle**: every `Unit` lives in exactly one place, the `Roster`. Every other
class refers to it by **id** (an `int`) and asks the Roster to resolve it. This avoids the
copy bug: if two structures stored copies of the same unit, damage applied to one would not
be visible in the other.

---

## 2. Project conventions

| Convention | Rule |
|---|---|
| Contracts | Pre/Post comments **in the header only**, under each declaration, **in English**, and always truthful (if the code changes, the contract changes). |
| One file per class | `X.h` (declaration + contracts) / `X.cpp` (implementation with `X::`). |
| Includes | Every file includes exactly what it uses; headers are self-sufficient. |
| Private members | All attributes private; `_` suffix in the newer classes. `Unit`/`Stats` predate the rule and don't carry it. |
| Erasing from vectors | The **erase–remove** idiom: `v.erase(std::remove(_if)(...), v.end())`. |
| Errors | Exceptions (`std::runtime_error`) for contract violations; `contains()` to validate user input before calling; local `try/catch` in menus so a user error never kills the game. |
| Output | English, pure ASCII, formats from the style guide. |
| Compilation | Always `/W4 /w15038` and zero warnings. |
| Randomness | A single `std::mt19937` seeded in `main`, passed everywhere by reference. |

---

## 3. The classes

### 3.1 `Stats` (inside `Unit.h` / `Unit.cpp`)

Fields: `health`, `maxHealth`, `strength`, `constitution` (private).

- `increaseHealth` / `decreaseHealth` **clamp**: health never leaves `[0, maxHealth]`.
- `isAlive()` is **derived** (`health > 0`) — a single source of truth.

### 3.2 `Unit` (`Unit.h` / `Unit.cpp`)

The character. Private fields: `id`, `experience`, `stats`, `name`, `race` (1–6 \*), `level`,
`skills` (traits), `history` (generated backstory), `hook` (bond with a fallen unit; empty if none).

- **Combat delegates**: `takeDamage` / `heal` / `isAlive` delegate to `stats`.
- ⚠ `getStats()` returns a **copy** — for reading, never for modifying.
- `addExperience(n)` adds and **consumes** XP on level-up: while `experience >= level * 100`,
  it subtracts the cost, gains a level and grants +10 maxHP (+10 HP), +1 STR, +1 CON.
  **Returns the levels gained** — the incursion uses this to narrate level-ups.
- `printUnit()` prints the framed unit card (style guide §3).

### 3.3 `Roster` (`Roster.h` / `Roster.cpp`)

The collection of **living** units. Single owner (`std::vector<Unit> units_`).

- `findUnitById(int)` — two overloads (`Unit&` / `const Unit&`); both throw if the id doesn't exist.
  ⚠ **Dangling danger**: use the reference immediately; `addUnit`/`removeUnitById` invalidate it.
- `contains(int)` — exception-free validation (menus).
- `removeUnitById(int)` — erases if present; silent otherwise.

### 3.4 `Team` (`Team.h` / `Team.cpp`)

The selection for the incursion. **Stores ids, never units** (`std::vector<int> memberIds_`).
Capped at `MAX_MEMBERS` (5) — a class constant: the cap is Team's own invariant, so `main`
never needs to know it.

- `addMember(int, const Roster&)` — throws if the id is not in the roster, already in the
  team, or the team is full (menus wrap it in a local try/catch).
- `purgeDeadMembers(const Roster&)` — removes every id the roster no longer contains
  (`[&roster]` lambda + erase–remove).
- `printTeam(const Roster&)` — unit line + hook sub-line (8 spaces) when present.

### 3.5 `Generator` (`Generator.h` / `Generator.cpp`)

Builds units from the banks. Constructor: loads the **7 banks** from `resources/`
(name, job, motivation, place, trait, template, hook) into a `map<string, vector<string>>`;
throws if any is empty. Holds a **reference** to the rng (Pre: the rng must outlive it).

`generateUnit(id, necropolis)`:
1. Random name; 1–2 **distinct** traits (coin flip + reroll).
2. Weighted race 1–6 (`discrete_distribution{40,25,15,10,7,3}`).
3. Race stats: `HP = 80 + race*20`, `STR = CON = 8 + race*2`.
   **Birth buff**: a `Reckless` unit gets +30 maxHP (+30 HP) and +5 CON — the reckless tank.
4. Backstory: `fillTemplate` resolves every `{bank}` slot with a random entry from that bank.
5. **Hook (the core mechanic)**: if the necropolis is not empty, a 40% chance of carrying a
   hook with `{fallen}` replaced by the name of a random fallen unit.

### 3.6 `Necropolis` (`Necropolis.h` / `Necropolis.cpp`)

The memory of the fallen. Stores `DeathRecord` (name, floorDied, cause, turn, skills),
**never** units.

- `addDeath(const Unit&, floor, cause, turn)` — call **before** erasing the unit from the roster.
- `pickRandom(rng)` — a random record for the hooks; throws if empty (hence `empty()`).
- `print()` — tombstones, oldest first.
- Append-only: nothing is ever erased during a session.

### 3.7 `GameState` (`GameState.h`, header-only)

The progress that persists between incursions: `highestFloor` (the record, never decreases),
`incursionCount`, the `necropolis`, and the **essence economy**: `essence` (starts at 25) and
`invokeCost` (5). Essence is earned by clearing floors (+floor number), and spent on summoning
and on the entry toll. The roster and the team live in `main` and are passed separately.

### 3.8 `Incursion` (`Incursion.h` / `Incursion.cpp`)

The tower. Two pieces:

**`Encounter` + `loadEncounters(path)`**: a `{description, cause}` struct loaded from
`encounters.txt` (format `description|cause`, one per line; throws if the file is missing,
empty, or a line has no `|`).

**`runIncursion(team, roster, state, encounters, rng)`** — the floor loop:

```
power  = sum of the team's STR + CON (recomputed every floor)
danger = 20 + floor * 15
attack = power + luck(0..30) + trait event modifiers

attack >= danger*1.2 → "with ease"        (XP, no damage)
attack >= danger     → "with difficulty"  (XP + one wounded: 15-40 damage; may die)
attack <  danger     → "overwhelmed"      (one member dies outright; incursion ends)
```

- **Start floor choice**: if there is a record, the player picks `[1 .. record+1]` (push-your-luck).
- **Essence economy**: entering above floor 1 charges a toll of `startFloor - 1` essence
  (floor 1 is always free — no economic soft-lock); an unaffordable floor falls back to 1.
  Every cleared floor yields its floor number in essence, narrated in place.
- **Danger forecast**: before each climb prompt, one of 4 fixed lines computed from the fresh
  team power (recomputed after level-ups/deaths via the file-local `teamPower` helper) vs the
  next floor's danger, using the same integer arithmetic as the outcome branch and the shared
  `MAX_LUCK` constant. Truthful by construction; phrased as a feeling because next floor's
  trait events can still shift the roll.
- **One encounter per floor**: chosen at random; its `description` presents the floor and its
  `cause` goes on the tombstone if someone dies there.
- **Trait events** (max 1 per floor, 50% coin when there are candidates): the `DAMAGE_TRAIT_EVENTS`
  table in Incursion.cpp — `Brave` +8, `Cowardly` −12, `Reckless` +12 **and draws the wound**
  of the floor (`forcedVictimId`). The strings must match `traits.txt` exactly.
- **XP**: `floor * 10` per living member on every cleared floor; level-ups narrated on the spot.
- **Guarded record**: `if (floor > state.highestFloor)` — replaying low floors never lowers it.
- **Rest**: when the incursion ends, survivors heal to full (wounds only matter *inside* an
  incursion; the dead stay dead).

### 3.9 `Logger` (`Logger.h` / `Logger.cpp`)

Session recording (see `app/sessions/README.md`).

- `TeeBuf` (subclass of `std::streambuf`): `overflow`/`sync` mirror every character to two
  buffers (console + file).
- `SessionLog` (RAII): the constructor opens `sessions/session_YYYYMMDD_HHMMSS.log` and
  installs the TeeBuf into `std::cout`; the destructor restores the original buffer. If the
  file can't be opened, it warns and the game continues unrecorded.
- ⚠ The member order in the header (`file_`, `tee_`, `original_`) is **load-bearing**: `tee_`
  is built from `file_.rdbuf()`, and initialization order follows declaration order, not the
  init-list.

---

## 4. `main.cpp`

Startup order (inside the `try`): log name via `strftime` → `SessionLog` (first long-lived
variable: destroyed last) → `Team`/`Roster`/`GameState` → `mt19937` → `Generator`
→ `loadEncounters` → `printIntro()` → menu loop.

- `printIntro()` — opening lore screen: the game's tone + the one rule that matters (permadeath).
- `readChoice()` (Utils) — the only place that prints the `> ` prompt; returns 0 when the
  input is not a number (falls through to the switch's `default`).
- User errors (nonexistent id...) are caught with a **local** try/catch; the outer catch only
  sees fatal errors (missing banks...).

---

## 5. Data banks (`app/resources/`)

One value per line, no empty lines (the loaders skip them).

| File | Content | Grammar contract |
|---|---|---|
| `names.txt` (60) | Proper names | No punctuation |
| `places.txt` | Place names | No punctuation |
| `jobs.txt` | Professions | Singular noun; must fit «a {job}» |
| `traits.txt` (10) | Traits | **Adjective**, capitalized; the mechanical ones (`Brave`, `Cowardly`, `Reckless`) must match `DAMAGE_TRAIT_EVENTS` exactly |
| `motivations.txt` | Motivations | Full third-person sentence, no trailing period |
| `templates.txt` | Backstory templates | `{job}` `{place}` `{motivation}` slots; the template owns all punctuation |
| `hooks.txt` | Hooks | Sentence with the `{fallen}` slot (a fallen unit's name); no trailing period (the Generator adds it) |
| `encounters.txt` (35) | Encounters | `description\|cause`. Description without trailing period. **Cause = the unit's death**: lowercase, no period, and it must complete «fell on floor N, \<cause\>.» with the unit as the subject |

**Golden punctuation rule**: bank entries never carry a trailing period; all punctuation is
owned by whoever composes the sentence (template or code). A «..» or a missing period means
some entry broke the rule.

---

## 6. Build and run

- **VSCode task**: compiles `app/main.cpp` + `app/src/*.cpp` with `/Zi /EHsc /W4 /w15038`,
  outputs `app/ascend.exe`. Any new `.cpp` in `src/` is picked up automatically.
- **Manual**: `cl /EHsc /W4 /w15038 /nologo /Feascend.exe main.cpp src\*.cpp` from `app/`.
- ⚠ `LNK1168` = the exe is running; close it (option 9) and rebuild.
- ⚠ Relative paths (`resources/`, `sessions/`) resolve from the **working directory**:
  always run from `app/`.
- Build artifacts and `.log` files are covered by `.gitignore`.

---

## 7. The death flow (implemented — order matters)

```
1. necropolis.addDeath(unit, floor, enc.cause, turn)  // record while the unit still exists
2. roster.removeUnitById(id)                          // real permadeath: the Unit is destroyed
3. team.purgeDeadMembers(roster)                      // the team cleans up orphaned ids
```

Swapping 1 and 2 = recording a unit that no longer exists. The tombstone's cause is the one
from the **encounter** on the floor where the unit fell — every death has a culprit and a scene.

---

## 8. Roadmap (vs. GDD §6 plan)

| GDD step | Status |
|---|---|
| 1. Unit + print | ✅ |
| 2. Roster + menu | ✅ |
| 3. Generator with banks | ✅ (7 banks, weighted race, fillTemplate) |
| 4. Incursion | ✅ (push-your-luck with floor choice, XP, level-ups) |
| 5. Permadeath + Necropolis | ✅ (record→erase→purge flow) |
| 6. Hooks conditioned by the Necropolis | ✅ — THE CORE MECHANIC WORKS |
| 7. Narrative polish | ✅ (style guide + full text pass) |
| 8. Play and evaluate (§1.1) | ✅ Done — long sessions + tester logs. Deaths of generated-history units read as *person* loss, not just resource loss; hypothesis validated. Findings drove Phase 0.5 and Phase 1 (see [backlog](backlog.md)). |

**Out-of-plan additions** (all in service of step 8): encounters with bound death causes,
trait events in combat (+ the Reckless archetype), rest-healing between incursions, intro
screen, automatic session recording in `sessions/`, project README,
[style guide](style-guide.md) and [backlog](backlog.md).

**After Phase 0**: Phase 0.5 (essence economy, team cap, danger forecast, residual incident on
safe floors) and Phase 1 (permanent injuries, training camp) — see
[technical-documentation-phase1.md](technical-documentation-phase1.md).
