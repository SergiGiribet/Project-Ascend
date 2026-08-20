# Roadmap — Project Ascend

A north star for finishing Phase 1 and shaping Phase 2. This is a **living design document**,
not a contract: it records the direction we agreed on and *why*, so no idea gets lost and each
build step stays small. Written 2026-07-25; §5 rewritten 2026-08-06 after the Phase 2b bot
measurement sent the plan somewhere it had not expected to go; §7 (the technical trajectory
to ImGui, persistence and Godot) added 2026-08-18, and §5 reordered the same day when the 2c-3
upper-bound measurement showed the structure had to come before the mechanism; the curve
rebalance was promoted out of §6 and into the phase on 2026-08-20, when Hold and Slay turned out
to have gradients the difficulty curve never lets fire.

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

**Decided 2026-08-18: the scout is picked from the TEAM, never from the roster.** Three reasons,
ascending in weight: the bench is back at base and cannot slip ahead to a floor it never climbed;
a roster-wide pick would mean never packing an Alert at all (leave them home, send them to look),
undoing the very work that makes composition matter; and it would **kill the fatigue cost before
it exists** — a scout who was never going to fight costs nothing, so information would be free
again, which is precisely the failure the 2026-08-06 measurement recorded, just in another
currency. The rule: **the roster decides who enters the tower; once inside, you may only spend
what you brought.** The roster-wide instinct is sound but belongs at the door — in 2d's sorties,
where a scouting party is paid for with expedition slots.

| Step | What | Note |
|---|---|---|
| 2c-1 | `Scouting.h/.cpp`, `Report`, `scoutAhead` returning a *perfect* report; wire it in, retire `SCOUT_COST` | **done** -- behaviour-neutral but for the attribution |
| 2c-2 | Voice: reports attributed and framed as claims; the hindsight line when a report was wrong | **done** -- plus the §1.4/§1.5 rewrite, below |
| 2c-3 | Fatigue: optional `excludeId` on `teamPower`/`teamFit`; the scout sits out the floor they scouted | **done, then measured** -- see below |

Personalities and the closing measurement were 2c-4 and 2c-5. They now live in **2e**, behind the
structural work, for the reason the next section records.

**A style-guide amendment this needs.** §1.4 ("No lying output") is narrower than it sounds — it
forbids the game reporting success for an action that failed. A scout's report is not a result,
it is *a claim by a character*. The rule gets rewritten to separate the two: **the narration
never lies about what happened; a character's report is only ever a claim, and is always
attributed by name.** Three things keep an unreliable report readable as personality instead of
as a bug: attribution (never "the scouts", always "Keira"), honest hedging (the *tone* must not
fake confidence the scout does not have, even when the content is wrong), and a hindsight line on
arrival (*"Keira had promised an easy fight."*) so the player blames Keira and not the game.
Omission is the cleaner half and comes first: a scout who never got close simply brings back less.

### The 2c-3 upper-bound measurement, and the reorder it forced  *(2026-08-18)*

Before pouring character into `scoutAhead`, the mechanism was measured **at its ceiling**:
*perfect* information, priced at one unit-floor. Reports can only get worse from there (2e adds
ways to misreport), so a loss here could never be rescued by flavour. 20 campaigns per arm, 5
incursions each, policies identical but for scouting:

| arm | mean best floor | deaths per campaign |
|---|---:|---:|
| scouting ON, perfect information | 9.50 | 1.50 |
| blind (control) | 10.40 | 1.85 |
| | *p = 0.043* | *p = 0.254* |

(permutation test on the difference of means, 200k resamples — no library needed)

**Scouting costs a floor of record and buys no significant safety.** All 88 scouts were then
dissected:

| | count |
|---|---:|
| sharpened read *better* than the distant one | **1** |
| sharpened read *worse* | 58 |
| unchanged | 29 |
| flipped climb → retreat | 30 |
| flipped retreat → climb | **0** |

Two causes, one local and one structural.

**Local: the fatigue cost is triple the signal it buys.** A level-2 unit is ~28 points of team
power, a forecast tier is 30 wide, and `traitFit` swings ±8 to ±20. So excluding the scout from
the sharpened forecast costs a whole tier while the information is worth half of one — the number
the player reads after paying goes *down* almost always, and not because the floor is worse but
because they just weakened the party. **Information arrives permanently disguised as bad news.**
The fix, if it is still needed later: the scout contributes their **stats but not their traits**
(body present, judgement absent), which costs exactly their `traitFit` — the same order as the
signal, and one line: pass `claim.scoutId` to `teamFit` only, never to `teamPower`.

**Structural, and this is the one that matters: 0 of 88.** Not once did information send the team
*up* a floor it would otherwise have refused. With go/no-go as the only decision on the table,
better information can only ever make you more cautious — and caution is precisely what lowers
the record being measured. Three measurements have now said the same sentence three different
ways: **there is nowhere to spend what you learn.**

**Decision (2026-08-18): structure before mechanism.** Personalities and the closing measurement
move out to 2e, behind 2d. Flavour cannot rescue a mechanism whose prerequisite does not exist,
and measuring it again first would only have been a fourth reading of the same wall.

**A methodological correction comes with it.** The bot climbs on green or yellow and retreats on
red — a strategy with no composition in it whatsoever. Such an agent can only ever be *hurt* by
information, because retreating is the only thing it knows how to do with it. We have been
measuring a game about composition with a player that has none, and that cannot be fixed until
composition is a decision made **between** sorties, which is 2d. Until then the validation is
playing it and reading whether the choices feel like choices — see the amended Rhythm.

### What the first human session changed  *(2026-08-19)*

Nine incursions with recon-by-unit in place (full record in the [backlog](backlog.md)). Six scouts
sent, **zero team changes in response**. The reasons reorder the rest of the phase, and two of them
were invisible from the code:

- **The player cannot see traits.** `printUnit` is called from one place in the entire game, right
  after summoning. Nothing else ever shows a unit's STR, CON, traits, history or hook —
  `printRoster` shows none of them. Phase 2b's whole trait-fit system therefore rests on an input
  the player has no way to read. **Nothing else in this phase can be judged until this is fixed**,
  because every measurement so far has asked the player to compose a team blind.
- **The missions are text, not mechanism.** `rounds` is generated, printed and never read; a Hold
  does not produce rounds to hold. The four types differ only in which traits they favour. Knowing
  the type cannot matter much while the type barely does anything.
- **One floor of intel is worthless across an eight-floor climb** — you meet every type on the way
  up regardless, so only the team's average matters. Confirmed in play: the player scouted floor 14
  and entered at floor 5.
- **The fit's bad news is too small.** +24 best case against -8 worst, on a 30-point tier. The
  near-zero-sum columns that keep any objective type from being secretly easier also make a
  5-unit team mediocre-at-everything: 8-10 traits drawn from a balanced table sum to the middle.
  **Column balance kills variance.** The answer is smaller expeditions (2d-3), not bigger numbers.

**Revised order for the rest of Phase 2:**

| | What | Why here |
|---|---|---|
| **2d-0** | A unit can be inspected at any time; traits visible in the roster; bulk summoning and less tedious camp/team assignment | tiny, and it unblocks every decision the phase is about |
| **2d-2c** | One floor per sortie | makes the scouted floor decisive instead of one of eight |
| **2d-3** | Expedition chosen from the roster, places as the cost | small squads give the fit real variance |
| **2b'** | Missions generate actions: a Hold resolves in rounds, and each type does something a number cannot | the largest piece, and what makes the types real |
| **2e** | Personalities, then the closing measurement | unchanged |

2b' is the one the player has been asking for since the first day of Phase 2 ("the floors are
flat"), and it is where the play-by-play resolution of §7's T1 starts to pay off: a floor that
resolves in steps has something to narrate, and a sortie that is one floor can afford to narrate it.

### 2d — One floor per sortie: the structure that gives information a use

Adopted 2026-08-18 on the user's proposal, and it is the fix all three measurements have been
pointing at. An incursion stops being a thirty-floor climb and becomes **one floor**: the party
enters, resolves it, and comes out. Progress happens by re-entering at `record+1` — which the
game already supports, and which the [backlog](backlog.md) currently files under
"the heal-retreat-reenter exploit". Phase 2 stops treating it as an exploit and makes it the
intended path.

What it unlocks, in order of weight:

- **Composition becomes answerable.** You learn what the floor holds, and *then* you choose who
  goes. That is the lever `traitFit` has been waiting for since 2b, and the missing half of every
  measurement so far.
- **The expedition is picked from the whole roster**, with the number of places as the real cost.
  This retires 2c's team-only rule, which existed solely because composition was frozen mid-climb:
  the roster still decides who enters the tower, it is only that now every floor is an entry.
  **A scouting party becomes a sortie of its own** — pay the toll, risk the people, and what you
  buy is knowledge. Including, explicitly, sending someone cheap to die for it: a life with a name
  spent on information, which is GDD §1.1 aimed straight at a mechanic.
- **The greed tension survives and improves.** Re-entry is immediate, so "push on now or regroup"
  is still there at every floor; what changes is that regrouping becomes *interesting* instead of
  a formality.

**The linchpin: leaving the tower must stop being a free full heal.** Today it is one
(`roster.healAll()`), and the toll is trivial once essence piles into the thousands. Ship
one-floor sorties before fixing that and the only correct play becomes "one floor, go home, heal
free, return" — the loop will feel hollow, and it will read as the *design* failing when only the
sequencing did.

| Step | What | Note |
|---|---|---|
| 2d-1 | Healing stops being automatic and total: only those who did not climb recover (`healRested`), and later rest becomes a lobby resource decision | the linchpin; makes the bench matter and gives surplus essence a use |
| 2d-2 | One floor per sortie: the run ends after a floor, re-entry at `record+1` is the supported path, tolls and framing tuned for it | mostly reframing what already exists |
| 2d-3 | The expedition is chosen from the roster, places as the cost; scouting parties become sorties of their own | retires the team-only rule, which the structure makes obsolete |
| 2d-4 | The cheap-fatigue fix, if it is still needed by then | may well be moot once scouting is its own sortie |

### The curve stops being last  *(2026-08-20)*

Phase 2b' gave Hold and Slay real mechanics, each with a gradient of cost. A session immediately
afterwards produced **zero** costly outcomes in 38 floors: twelve flawless Holds, five one-blow
Slays, nineteen of twenty-one single-roll floors walked (full table in the [backlog](backlog.md)).

The cause is the difficulty curve, and it is arithmetic rather than tuning. `danger` climbs 15 a
floor; `luck` spans 30. So the band in which a team can *sometimes* fail is about **two floors
wide**: below it nothing can go wrong, above it nothing lands at all. Two independent measurements
the same day agree -- 16 of 16 lost Slay fights ended with the enemy untouched, because a team that
cannot reach the threshold cannot reach it at all.

**Section 6 parked "curve rebalance" as *last*, and softer, so that decisions would have room to
breathe. That reasoning was right and its conclusion is now inverted: the decisions exist and
cannot be seen, because no floor is ever close.** Widening the band from two floors to six or eight
is not raising the ceiling -- it is **giving the wall a slope**, and it is what would make
everything built this week visible at all.

It is therefore promoted out of §6 and into the phase, ahead of Retrieve and Rescue. Getting it
wrong is cheap to undo (it is one formula) and the whole point is to be able to *watch* the
mechanics that already exist, so it comes before adding more of them.

Still true, and still the reason not to simply raise team power instead: raising the ceiling only
moves "say yes until red" higher. The target is the width of the uncertain band, not the height of
the wall.

### 2e — Personalities, and the closing measurement

Only once there is somewhere to spend information does it matter *how good* the information is.

| Step | What | Note |
|---|---|---|
| 2e-1 | Omission: Cowardly and Greedy come back without the danger read | the honest half -- a scout who never got close simply brings back less |
| 2e-2 | Distortion: the Boaster's `bias`, the Superstitious scout's wrong `type` | the consequence is already built and verified |
| 2e-3 | Scouting risk: wounds and deaths for whoever goes ahead | Alert and Curious as the reliable end of the scale |
| 2e-4 | The closing measurement | needs a bot that can recompose -- see the gate |

Everything 2e needs is already standing, and was verified on 2026-08-18 with a three-line
temporary lie in `scoutAhead`: a wrong claim cascades correctly through the fit into the forecast,
and the hindsight line names the scout to the player's face. **The punishment was built before the
crime.**

### Where Phase 2 ends

Not a feature checklist — a **falsifiable gate**, the same way Phase 1 closed on bot data:

> **An informed bot beats a blind one.** Same policy, same number of incursions; one scouts and
> recomposes, one does not. Phase 2 closes when the informed arm wins clearly on record floor,
> on deaths, or on both.

As of 2026-08-18 that number is **9.50 against 10.40 mean record floor, p = 0.043 — pointing the
wrong way**, and that is with *perfect* information. It is the honest state of the phase and it is
the number to move. Supporting conditions, all of which the gate implies: a floor asks something
specific (done), composition changes the odds (done), information is obtainable at a price that is
not self-defeating (2c: measured, and found wanting), and there is a lever to answer it with (2d).

**The gate also needs a fairer bot.** The present one climbs on green or yellow and retreats on
red — no composition anywhere in its strategy, so information can only cost it. The closing
measurement needs an agent that chooses *who goes* from the roster in answer to what it learned.
Until that exists the gate cannot be honestly tested, and a failing number says as much about the
bot as about the game.

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
6. **Bot measurement only when a whole loop exists** — amended 2026-08-18, and the amendment
   matters. Three mechanisms were each measured against a metric the structure could not yet
   serve, and each came back "no": demoralising, and only the third reading told us anything the
   second had not. Between measurements the validation is **playing it and reading whether the
   choices feel like choices**. A negative result is still a result — 2b's A/B is why 2c exists,
   and 2c-3's is why 2d now comes before 2e — but three in a row means the question was wrong,
   not the answer.

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

**2d-1**: stop healing being automatic and total. `Roster::healAll()` becomes
`healRested(climbedIds)` — only the units that did not climb recover, wounds ride home with
whoever did, and permanent injuries stay permanent as they always have. It is the linchpin of the
whole structural change: until leaving the tower costs something, one-floor sorties collapse into
"one floor, go home, heal free, return". Small, self-contained, and the first step in weeks that
is not waiting on anything else.
