# Roadmap — Project Ascend

A north star for finishing Phase 1 and shaping Phase 2. This is a **living design document**,
not a contract: it records the direction we agreed on and *why*, so no idea gets lost and each
build step stays small. Written 2026-07-25; §5 rewritten 2026-08-06 after the Phase 2b bot
measurement sent the plan somewhere it had not expected to go; §7 (the technical trajectory
to ImGui, persistence and Godot) added 2026-08-18.

Read alongside: the [Phase 0 baseline](technical-documentation.md), the
[Phase 1 additions](technical-documentation-phase1.md), the [style guide](style-guide.md), and
the [backlog](backlog.md) (which holds the raw playtest data, evaluation tables, and the parked
ideas this roadmap places into a sequence).

---

## 1. Where we are

- **Phase 0** — baseline: generated units with histories, the tower, permadeath, the Necropolis.
- **Phase 0.5** — scarcity and legibility: essence economy, team cap of 5, the 4-tier danger
  forecast, and the residual incident (safe floors are never *certain*).
- **Phase 1** — loss with continuity: permanent injuries (`injuries.txt`, never healed, real
  stat penalties) and the Training Camp (a bench where units level between incursions; an
  essence sink; trainers pulled from the active team as a real trade-off).

**Phase 1 is essentially feature-complete for its thesis, and validated.** The bot showed the
camp *eliminates the death-spiral collapse* (0 wipes, every death backfilled); a real human
session showed the intended loop — an **injured veteran becomes a trainer** — emerging on its
own, with no prompting. Today's polish pass finished the legibility: injury stat costs are now
visible (`{One-Handed (-3 STR)}` on the roster and card, via a shared `injuryLabel`), trait
deeds vary instead of repeating, and the camp's slot label is unambiguous.

---

## 2. The diagnosis: is the "wall" a real problem, or just RNG?

Both — but not as it looks. The wall is a real *mathematical ceiling*: `danger = 20 + 15*floor`
climbs relentlessly while team power grows slowly (XP curve + churn from deaths). There is a
floor past which even maximum luck loses. That ceiling exists.

**But the wall is the symptom, not the disease.** Each floor, the only decision is *climb or
retreat*, and the forecast dictates it. There is no in-floor tactics, no composition that
changes the math, no mid-incursion resource management. The player is a **spectator to the RNG
with a stop button**. That is why it "feels like all RNG": not too much chance, but *no lever
that shapes it*.

The data agrees, all pointing the same way (see the [backlog](backlog.md) evaluation table and
the human session log):
- The camp removed collapse but did **not** move the ceiling (~13-15 aggressive, ~17-19 safe;
  trainer level does not raise it).
- The human session was 9 incursions of the identical "climb until red" pattern; the record
  crawled 6 → 10.

**Operative conclusion:** raising the ceiling (rebalancing danger vs. power) does **not** fix
the game — it just moves "say yes until red" from floor 15 to floor 30. The real work is to add
**decisions that let planning and skill move the odds**. The rebalance comes *after*, and
softer, so those decisions have room to breathe.

---

## 3. The organizing principle for Phase 2

Separate two things that are easy to conflate:

1. **The mission framework** — *what* a floor asks and *what* resolves it. Built once.
2. **The agent model** — *who* decides how to solve it. This is a **dial we turn up over time
   without rebuilding the framework**:
   - **player-directed** (the player picks composition, recon, and approach; units execute),
   - then **units decide, player nudges** (soft indications),
   - then **full autonomy** with hints.

**Decision (2026-07-25): start player-directed, grow autonomy once there is a base from which a
unit can actually decide.** This is not a compromise — it is the correct dependency order. A
unit that "knows the mission and chooses how to solve it" has nothing to act on until both the
mission framework and a resolution model exist. Building the AI first would be a brain with no
body. (The unit-autonomy layer is nonetheless *thematically central* — see 2d.)

A second cross-cutting principle: **the tower gains a memory.** Today every floor is stateless.
For outcomes to echo, systems need a shared world-state to read and write. See 2c.

---

## 4. Closing Phase 1 (near-term)

- **Done (2026-07-25):** injury stat cost visible (`injuryLabel`, roster + card); varied trait
  deeds (`deeds` bank in `DAMAGE_TRAIT_EVENTS`); "Free trainer slots" label.
- **Remaining:** refresh the docs (this file + the Phase 1 doc + style guide) with the above;
  commit; optional closing tag / release for the colleague's test.
- **Explicitly NOT in Phase 1 closure** (parked to Phase 2 or the backlog): roster bloat / "make
  the whole bench matter", and camp tuning (the data says trainer level is a low-value dial).

---

## 5. Phase 2 — the plan

The pivot: **the mission replaces the scalar `danger`.** A floor stops being one number to beat
and becomes an **objective** with requirements. (Structurally the same move as `deed` → `deeds`:
a flat value becomes structured, data-driven.) This alone breaks the binary: not "do I climb?"
but "can *this* team solve *this* objective — do I have the composition, do I scout first?"

### 2a — Missions as the floor's core  *(first milestone)*

- An `Objective` representation (type + data), data-driven like `encounters.txt`.
- Start with 3-4 objective types, each rewarding a *different* composition, so composition
  becomes strategy:
  - **Slay** — defeat the enemy → STR + offensive traits.
  - **Hold N** — survive N rounds → CON + defensive traits + team size.
  - **Retrieve / Search** — find something → perception/luck + speed, under time pressure.
  - **Rescue** — free a trapped unit → and on success it **joins the roster** (ties straight
    into the permadeath/roster theme).
- **Reconnaissance**: spend essence to reveal the next floor's objective (later, enemy detail).
  This is what finally makes recon *worth it*.
- Resolution stays **player-directed** (per §3): the player chooses composition, whether to
  scout, and an approach; units execute.

This is the smallest thing that makes decisions matter, and it forces the architectural
decisions (how an `Objective` is represented, where floor data lives) everything else builds on.
Good C++ learning vehicle: an `enum` of types + a resolution switch now; possibly virtual
dispatch (an objective class hierarchy) later.

### 2b — Objective-aware resolution  *(done, 2026-08-06)*

The plan was "different stats per objective type". **Measurement killed that before it was
built:** `Generator` gives every unit `STR == CON` (`8 + race*2` for both) and levelling raises
both by 1, so `STR*2 + CON` and `CON*2 + STR` are *the same number*. Only Reckless (+5 CON) and
injuries split them, worth 3-7 points on a team — invisible against a 30-point forecast tier.

So the differentiation went entirely into **traits**, which is where the roadmap always said the
strategy lived:

- `traitFit(type, traits)` — a 12-trait x 4-type table in `Objective.cpp` (+-8 strong fit, +-4
  mild). Columns are near-zero-sum so no objective type is secretly easier; deliberate
  difficulty differences belong in `makeObjective`, where they are visible.
- `teamFit` sums it over the members and it is **added to the team's power for that floor**,
  with a plain-language disposition line under the briefing (`This team is well suited to it
  (+16).`). The number is always shown: an invisible modifier reads as RNG, which is the disease
  we are treating.
- Combat stays an **abstracted exchange**, deliberately not a turn-based tactical engine.

Verified in play: a team of Stubborn/Boaster/Reckless/Cowardly/Greedy/Alert scores +8 to Slay
and -12 to Hold — the same five people, worth twenty points more at killing than at holding
ground.

### The 2026-08-06 measurement, and what it changed

A bot A/B ran 8 campaigns per arm (5 incursions each, identical policy but for scouting):

| arm | best floor min/median/max | deaths | scouts | decisions changed |
|---|---|---|---|---|
| scout ON | 9 / 10 / 12 | 14 | 60 | 8 |
| scout OFF | 9 / **11** / 13 | 14 | 0 | — |

**Scouting lost.** Not for the obvious reason: of 60 scouts the sharpened read was *better* than
the distant one 12 times and worse only 4, so the information encouraged climbing rather than
retreat. Two real causes:

1. **Wrong currency.** 3 essence x 60 = 180, about 22 per campaign — the same essence that pays
   entry tolls and replacements. Information priced against bodies always loses, at any price.
2. **Nowhere to spend it.** 73% of scouts changed nothing, because **the team is locked before
   the incursion**. The only possible answer to "a Hold you don't fit is next" is to turn back.
   Information can subtract; it can never add.

Both consequences are now adopted as the rest of Phase 2, and they replace the old 2c/2d (see
"Beyond Phase 2" below for where those went).

### 2c — Recon by unit  *(current work)*

Scouting stops costing essence and starts costing **people**. You send someone ahead; what comes
back depends on who they are. The cost is force (they sit out the floor) and risk (they can come
back hurt, or not at all) — the same currency as everything else in the game, which is the point:
information now carries the GDD §1.1 stakes.

One report, three things it can carry, each losable or corruptible on its own:

```cpp
struct Report {
    std::string scout;    // who went -- every line is attributed to them by name
    bool sawType;         // did they bring the objective back at all
    ObjectiveType type;   // what they claim it is (only meaningful if sawType)
    bool sawDanger;       // did they get close enough to judge the odds
    int bias;             // added to the power they report: >0 tells it rosier than it is
};
```

The design keystone: **one lie, cascading.** If a Superstitious scout claims Rescue when the
floor holds a Hold, the sharpened forecast is computed with the *Rescue* fit and comes out wrong
on its own. No second falsehood to author, no second thing to keep consistent.

Lives in a new `Scouting.h/.cpp` — `Objective.h` deliberately does not know `Unit`, and
`Incursion.cpp` is already 450 lines.

| Step | What | Note |
|---|---|---|
| 2c-1 | `Scouting.h/.cpp`, `Report`, `scoutAhead` returning a *perfect* report; wire it in, retire `SCOUT_COST` | behaviour-neutral but for attribution -- prove the seam before adding character |
| 2c-2 | Voice: reports named and hedged; the hindsight line when a report was wrong | plus the §1.4 rewrite, below |
| 2c-3 | Fatigue: optional `excludeId` on `teamPower`/`teamFit`; the scout sits out the floor they scouted | the force cost |
| 2c-4 | Personalities: omission first (Cowardly, Greedy), then distortion (Boaster bias, Superstitious wrong type), then scouting risk | Alert/Curious as the reliable end |
| 2c-5 | Re-run the A/B | does an informed bot now beat a blind one? |

**A style-guide amendment this needs.** §1.4 ("No lying output") is narrower than it sounds — it
forbids the game reporting success for an action that failed. A scout's report is not a result,
it is *a claim by a character*. The rule gets rewritten to separate the two: **the narration
never lies about what happened; a character's report is only ever a claim, and is always
attributed by name.** Three things keep an unreliable report readable as personality instead of
as a bug: attribution (never "the scouts", always "Keira"), honest hedging (the *tone* must not
fake confidence the scout does not have, even when the content is wrong), and a hindsight line on
arrival (*"Keira had promised an easy fight."*) so the player blames Keira and not the game.
Omission is the cleaner half and comes first: a scout who never got close simply brings back less.

### 2d — Somewhere to spend the information

Recon is only half an answer while composition is frozen at the tower door. The fix is
structural, and most of it already exists: the game *already* lets you clear one floor, retreat,
and re-enter at `record+1` for a toll. The [backlog](backlog.md) files that under
"the heal-retreat-reenter exploit". Phase 2 stops treating it as an exploit and makes it the
intended path.

**The linchpin, and it must land first or alongside:** leaving the tower is currently a *free
full heal* (`roster.healAll()`), and the toll is trivial once essence piles into the thousands.
So today going home costs nothing, and greed only survives because retreating is *inconvenient*
rather than *expensive*. Ship one-floor sorties before fixing that and the only correct play
becomes "one floor, go home, heal free, return" — and the playtest will read as a bad design when
only the sequencing was wrong.

| Step | What | Note |
|---|---|---|
| 2d-1 | `healRested(climbedIds)` — only those who stayed behind recover; wounds ride home with whoever climbed | one function; makes the bench matter and finally gives surplus essence a use |
| 2d-2 | Make extract-and-re-enter the supported path: toll tuning, retreat/re-entry framing | not new machinery -- reframing |
| 2d-3 | Verify recomposition is worth doing: does knowing floor N+1 change *who you send*, not just *whether* | the whole point of 2c |
| 2d-4 | Closing measurement | see the gate below |

Note the greed tension is **not** lost to one-floor sorties — re-entry is immediate, so "push now
or regroup" survives, and it gets better: both options become interesting instead of one being
obvious. It only survives if 2d-1 lands.

### Where Phase 2 ends

Not a feature checklist — a **falsifiable gate**, the same way Phase 1 closed on bot data:

> **An informed bot beats a blind one.** Same policy, same number of incursions; one scouts and
> recomposes, one does not. Phase 2 closes when the informed arm wins clearly on record floor,
> on deaths, or on both.

Today that number is 10 vs 11 — pointing the wrong way. That is the honest state of the phase,
and it is the number to move. Supporting conditions, all of which the gate implies: a floor asks
something specific (done), composition changes the odds (done), information is obtainable at a
price that is not self-defeating (2c), and there is a lever to answer it with (2d).

### Explicitly NOT in Phase 2

Named here because the vision is much larger than the phase, and scope creep is the main risk:

- Real-time expeditions (6-12h missions running while the player does other things). Needs a
  **save system, which the project does not have** — the only `ofstream` in the codebase is the
  session log. It is also a genre change, from a session game to a management game with offline
  progression. 2d is its prerequisite; build it after the loop convinces.
- Persistence / save-load. Same reason. Build it once, knowing what has to be saved.
- Armory, shop, items, equipment. Phase 3 — but note 2d-1 is the seam they plug into.
- Real-time visualisation of a floor resolving. Presentation layer, and it wants the abstracted
  resolution to be settled first.
- Unit autonomy, and the tower's persistent memory (both below).

### Beyond Phase 2

The two open-ended layers the old plan called 2c and 2d, unchanged in substance, moved out
because a phase has to end somewhere:

- **The tower has a memory.** Within-run first (floor N affects N+1 via a small `RunState`;
  fled → the floor fortifies; rescued someone → they climb with you), then persistent, where
  *"an important character dies and it has repercussions"* lives. It hangs on the Necropolis and
  the generated histories that Phase 0 already built: Phase 0 gave characters a past, this gives
  their *end* a mark. Material parked for it: the third `Encounter` field (`resolution`) and the
  saved clear-flavor texts in the [backlog](backlog.md).
- **Unit autonomy** — the endpoint of the agent-model dial of §3, and thematically central. It
  **has already begun**: the varied-deeds system is the first atom of "units react to the
  situation", and 2c's unreliable reports are the second — a unit that reports what *it* noticed
  is already deciding something. It grows from there into units stepping up by trait and mission,
  with player indications as soft nudges.

### Rhythm

The loop that worked all through Phase 1 and 2a-2b, kept deliberately:

1. One numbered step at a time, small enough to hold in your head.
2. **The user writes the code.** Guidance names the shape, the trade-off, and what will break.
3. Review, compile, and verify the behaviour *in game* — reading the session log, not assuming.
4. Pre/Post contracts and player-facing text are maintained alongside, and every change reported.
5. Commit at each step boundary.
6. **Bot measurement only at the end of a layer**, never mid-way, where it is noise. A negative
   result is a result: 2b's A/B is why 2c and 2d exist at all.

---

## 6. Parked ideas, placed in the sequence

From the [backlog](backlog.md), where they fit once the framework exists:

- **Multi-team expeditions + team size cap** → "make the whole roster matter" (composition and
  sacrifice decisions). Pairs naturally with missions (one team holds while another pushes).
- **Trait/skill fusion through experience** → deepens 2b/2d (what a unit lives through changes
  who they are).
- **Camp depth** (infirmary/healing room, training traits like Mentor, prosthetics for injuries)
  → the Phase 1 "deferred" list; turns recovery into a resource decision.
- **Curve rebalance** → **last**, and softer, *after* decisions exist — so it doesn't just move
  the wall.

---

## 7. Technical trajectory: presentation, persistence, engine  *(decided 2026-08-18)*

The design keeps being built where iterating is cheapest — **the console** — and the presentation
layer arrives once the Phase 2 gate flips, not before. A new engine does not move that gate: it
would freeze the design in a state the data says is unproven, and spend weeks rebuilding what
already works to learn nothing about whether the loop is good. **The order below is deliberate.**

### The real blocker (measured 2026-08-18)

| File | Lines | `std::cout` |
|---|---:|---:|
| `main.cpp` | 375 | 85 |
| `Incursion.cpp` | 478 | 56 |
| `Objective.cpp` | 81 | 0 |
| `Generator.cpp` | 84 | 0 |
| `Injury.cpp` | 76 | 0 |

`Objective`, `Generator` and `Injury` port to anything as they stand. `Incursion.cpp` does not,
because there the resolution **is** the printing — one body of code does both. That fusion, not
the absence of an engine, is what blocks every step below.

### T1 — Separate simulation from presentation  *(begins inside 2c)*

Resolution stops printing and starts **returning data**: an ordered list of events that a caller
renders. The console becomes one renderer among later others, and nothing in the rules cares
which one is attached.

It pays off before any engine exists: **the bot could call the simulation directly** instead of
scraping stdout. Most of the 2026-08-06 measurement's cost went into the stdout protocol (the
`"> "` prompt marker; the roster screen printing the team underneath it, whose `[id]` lines carry
no marker and read as free units). With T1 done, running the phase gate drops from an afternoon
to instant and repeatable.

**2c-1 is already the first brick.** `struct Report` is logic that returns data and lets the
caller print it. Build it, watch it work, then decide whether to generalise the pattern across
`Incursion.cpp` — incrementally, one screen at a time, never as one big rewrite.

### T2 — A Dear ImGui front-end

Not an engine — a **window plus widgets** bolted onto the existing C++:

- **raylib** (or SDL2) opens the window, owns the frame loop, reads the mouse and keyboard.
- **Dear ImGui** draws the panels: roster tables, unit cards, the tower view, tooltips, timers.

Why it fits this project specifically: ImGui is **immediate mode** — every frame you rebuild the
whole interface from current state, top to bottom. That is *already* what this game does each time
it reprints the roster on opening a menu. The mental model transfers with no translation, so it is
a **change of façade, not a rewrite**, and 100% of the C++ (and of the C++ being learned) survives.

Sequence: T1 must be far enough along that at least one screen is fed by data instead of `cout`.
Start with the roster (pure display, no input), then the team screen, then the incursion view.
The console front-end keeps working the whole time — two renderers over one simulation is the
point of T1, and the console one is what the bot and the session logs use.

### T3 — Save system

Independent of any engine, and a hard prerequisite for the long-form expeditions in the vision.
The project has **no persistence today**: the only `ofstream` in the codebase is the session log,
and Roster, Team, GameState, Necropolis and the camp all live in `main()` and die with it.

Build it **once**, and only when it is known what has to be saved — which is why it comes after
Phase 2 settles the shape of a run. It must cover the roster and its histories, the Necropolis
(the whole point of the game remembering), essence and the tower record, the camp with its
in-flight training, and eventually expeditions in progress with their clocks.

### T4 — Godot 4  *(the chosen destination)*

**Decided 2026-08-18**, and not a fresh evaluation: it is the engine already intended, already
worked with, and open source — which matters to this project on principle.

It arrives last because by then everything it needs exists: a simulation that speaks in data (T1),
a UI whose shape has been proven in ImGui (T2), and state that can be written to disk (T3). The
simulation stays C++ through **GDExtension**, so Godot owns presentation, input, scenes and
export, and the rules stay in the code they were learned in.

Ruled out on the way: **Unreal** (C++ native but wildly oversized for a management game — you
fight the engine) and **Unity** (C#, which would mean abandoning both the codebase and the
language being learned).

### Ordering rule

None of T2, T3 or T4 begins before **the Phase 2 gate flips** (an informed bot beating a blind
one, §5). T1 is the exception and starts now, because it is the only one that makes the current
design work *faster* rather than prettier.

## 8. Immediate next step

**2c-1**: create `Scouting.h/.cpp` with the `Report` struct and a `scoutAhead` that always
returns a perfect report (`sawType`, `sawDanger`, `bias = 0`, no traits consulted yet). Wire it
into the climb prompt in place of the essence-priced scout and retire `SCOUT_COST`. Behaviour
stays as it is today apart from the report being attributed to a named unit — the seam gets
proven before any character is poured into it, exactly as `Objective` was introduced
behaviour-neutral in 2a.
