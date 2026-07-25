# Roadmap — Project Ascend

A north star for finishing Phase 1 and shaping Phase 2. This is a **living design document**,
not a contract: it records the direction we agreed on and *why*, so no idea gets lost and each
build step stays small. Written 2026-07-25, from the data collected so far.

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

### 2b — Objective-aware resolution

- A resolution function that **knows the objective** and draws on different stats/traits per
  type. Traits become **situational**: Brave good for Hold, Reckless good for Slay but bad for
  an escort. This is where composition = strategy.
- Combat is an **abstracted exchange** (enemy HP/attack) for Slay-type objectives — deliberately
  **not** a turn-based tactical engine (initiative, positioning, abilities). That is a possible
  much-later layer, not a requirement for the vision to land.

### 2c — Consequence: the tower has a memory  *(cross-cutting, starts small)*

Two scales, very different cost — do not conflate them:

- **Within-incursion** (floor N affects N+1, resets next run) — cheap, immediate payoff:
  fled → the floor fortifies; brute-forced a Slay → reinforcements next tram; rescued someone →
  they climb with you. Needs a small **`RunState`** (a bag of flags the resolution reads/writes).
  Introduce this seam **during 2a**, even if nearly empty, so later systems write to it naturally.
- **Persistent / campaign** (an event marks the world across runs) — the narrative payoff, and
  where *"an important character dies → it has repercussions"* lives. It hangs on systems that
  already exist: the **Necropolis** and the **generated unit histories**. Phase 0 built
  characters *with* histories; this makes their *end* leave a mark (other units reference it, the
  tower reacts). It closes the GDD §1.1 loop. Grows later, on top of the mission framework.
- Material already parked for this: the third `Encounter` field (`resolution`) and the saved
  clear-flavor texts in the [backlog](backlog.md).

### 2d — Unit autonomy  *(deferred, but thematically central)*

The endpoint of the agent-model dial. It **has already begun**: the varied-deeds system is
literally the first atom of "units react to the situation." It grows from there — units step up
by trait + mission; player indications become soft nudges over units that otherwise act on their
own. This is where the "complete autonomy with player hints" vision fully lands.

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

## 7. Immediate next step

Close Phase 1 (docs + commit), then open Phase 2a by designing the `Objective` representation
and how a floor carries one. The agent-model fork is already resolved (player-directed first).
