# Backlog: parked ideas and saved material

## Playtest findings — step 8 (2026-07-17)

Findings from the author's own sessions; they define the Phase 0.5 iteration:

1. **Free infinite summons make units fodder.** Even a 6* death doesn't sting because the
   replacement costs nothing: loss = attachment x irreplaceability, and the second factor is
   zero. The hypothesis had a hidden precondition: scarcity.
2. **The heal-retreat-reenter exploit.** Full rest-heal + free re-entry at the record floor
   means the optimal play is: climb one floor, retreat if wounded, re-enter healed at the same
   floor. Retreating has no price, so push-your-luck has no teeth.
3. **Risk is illegible.** The climb decision is a coin flip because the player can't estimate
   the next floor's danger.

**Phase 0.5 — IMPLEMENTED (2026-07-17)**: essence economy (+floor per cleared floor, summon
costs 5, entry toll = startFloor - 1 with floor 1 free, initial stock 25), team cap of 5
(Team::MAX_MEMBERS), and the 4-tier danger forecast before each climb prompt.
Pending re-test: does the 6*'s death hurt *now*? Watch during play: "when someone died,
could I afford the replacement without thinking?" — if yes past incursion 3, the dial is
invokeCost (one good run floors 1-8 yields ~36 essence, so wealth inflates fast; root cause
is the flat difficulty curve for 5-unit teams).
Known minor: at stdin EOF readChoice() returns 0 forever -> infinite menu loop (reachable
via Ctrl+Z+Enter); fix idea: throw on std::cin.eof() so the outer catch exits cleanly.

**Phase 0.5 — residual incident (2026-07-21)**: re-tested the "does death sting now" question
with real play. Finding: any floor cleared "with ease" still carried zero risk forever once a
team was strong enough — no jeopardy ever returned to safe floors, which kept farming
risk-free and undercut the permadeath hypothesis. Fix: a residual incident chance on
ease-tier clears (`incidentChance`, base + floor depth, capped, modified by Boaster/Alert
presence), a weighted (never forced) victim lottery shared with the difficulty-tier wound
(`pickWeightedVictim`, `riskDirection` — Reckless/Boaster more likely, Alert less), a
fatal-or-wound split (`FATAL_CHANCE`), and two flavor-text banks (`BOASTER_INCIDENTS` ironic,
`NEUTRAL_INCIDENTS` neutral) so the cause reads as a freak accident, not an encounter death.
Validated in a real 18-incursion session: fired at the designed rate (~6.7% of ease clears,
~10% of those fatal) and produced a real, narratively legible death (a repeat-Boaster
surviving several close calls before finally falling).
Follow-up finding from automated play-testing (5 sessions, adaptive bot, target floor 25):
none reached it: all plateaued around floor 14-16 and none escaped a death/dilution loop
(fresh Lv1 replacements dragging the team below the floor needed to progress, with essence
piling into the thousands with nothing to spend it on). Root cause isn't that the floors are
unbeatable — it's that recovering from losses requires manually re-leveling replacements at
safe floors, which no automated or impatient strategy does. This directly motivates the
training camp below.

## Phase 1 — in progress: Training Camp (2026-07-21)

Goal: let fresh recruits catch up passively instead of forcing a manual "drag the whole team
back to floor 1" detour after every death — the real bottleneck the Phase 0.5 follow-up
testing exposed. Also gives essence a real sink beyond invoking (sessions were ending with
5-6k essence unspent).

- **Roles**: any roster unit can be assigned as a trainer (pulled out of the active team while
  training — real trade-off, not a free add-on) or a trainee (lives in a separate camp roster,
  does NOT count against `Team::MAX_MEMBERS`).
- **Capacity**: 1 free trainer slot from the start; each trainer handles up to 5 trainees
  (fixed for v1). Extra slots cost essence, one-time, doubling from 250 (250, 500, 1000, 2000...).
- **XP per tick** (1 tick = 1 incursion launched): each trainee gets the full formula, not
  divided among the 5 — `XP = trainerLevel * 15 * raceMultiplier`,
  `raceMultiplier = 1 + (trainerRace - 1) * 0.15`.
- **Risk per tick**: 2% incident chance for the trainee, 0.3% for the trainer (asymmetric —
  the trainer is the experienced one). If it triggers: 85% a normal wound (heals with rest),
  15% a permanent injury.
- **Permanent injuries** (new, shared with the tower): a new `injuries.txt` bank, each with a
  modest real stat penalty (e.g. "One-Handed" -3 STR). Never heal, unlike normal wounds —
  needs its own mechanism, not `takeDamage`/`heal`. Stored in the same `skills` list as
  personality traits (no new field on `Unit`; thematically the scar becomes part of who they
  are). If the unit has Reckless or Boaster, 50% chance the injury swaps it for Alert (the
  lesson learned the hard way); other traits untouched for now. Also hooks into the tower's
  existing wound points (difficulty-tier and incident-tier): ~8% chance a wound there becomes
  permanent instead of healing.
- **Explicitly deferred to a later pass** (core -> risk -> depth, same discipline as the
  residual incident): trait-driven capacity trade-offs (more trainees per trainer for less XP
  each), dedicated training traits (Mentor/Harsh-style), a broader injury-trait swap mapping
  beyond Reckless/Boaster -> Alert, prosthetics/compensating items for injuries.
- **Wound healing v1 (stopgap)**: for now, ordinary (non-permanent) wounds — tower AND camp —
  are all healed at incursion end by resting the WHOLE roster (not just the team that
  climbed), so camp trainees don't accumulate damage across ticks. Permanent injuries never
  heal. Later this stopgap should be replaced by a real system: a dedicated healing/infirmary
  room or healing items, so recovery is a resource decision instead of automatic. Class file
  is `TrainingCamp` (TrainingCamp.h/.cpp).

## Training camp evaluation (2026-07-23, automated bot)

Extended the autoplay bot to use the camp (dedicate/upgrade a trainer, keep a leveling bench,
promote bench units to replace dead team members) and to gamble ~30% into "Something waits
above" floors. Ran batches of 4 sessions (151 incursions each, target floor 25). Results:

| Config | Deaths | Wipes | Record floor |
|---|---|---|---|
| Old bot, no camp (600 inc) | 500+ | many | 14-16 (constant collapse) |
| Camp, Lv1 fixed trainer, risky | 65-75 | 0 | 13-14 |
| Camp, upgrading trainer, risky | 64-71 | 0 | 13-15 |
| Camp, safe play (no gamble) | 0-1 | 0 | 17-19 |

Findings:
1. **The camp fixes the collapse.** 0 wipes in every camp run vs. constant wipes without it;
   every death is instantly backfilled from the bench. The dilution death-spiral is gone. Plus
   essence finally has a sink. This was its designed job — done.
2. **Play style sets the ceiling within a band; the risk tension is real.** Aggressive
   (~1 death/2 incursions) plateaus ~13-15; safe play reaches ~17-19. A ~4-5 floor gap between
   drama and progress — the "when courage becomes greed" tension now has mechanical teeth.
3. **Nobody reached 25, not even flawless safe play (~17-19 ceiling in 151 incursions).** So a
   deeper limit remains: difficulty `20+15*floor` outpaces achievable team power past ~17-19.
   This is the original exponential wall; the camp doesn't address it (nor was meant to). To
   make high floors reachable, rebalance the power/difficulty curves or accept a long grind.
4. **Injuries accumulate regardless of play style** (even safe play: ~8-13 permanent injuries
   per 151-incursion session across tower+camp), slowly eroding the roster — watch the tuning.

Bot NOTE for idea #2 below (injured veterans as trainers): the bot could not detect injuries —
the roster/team print shows Lv/HP/XP but not traits or STR/CON, so injuries are invisible to a
parser. Showing injuries in the roster would enable both players and tooling to repurpose
injured high-level units as trainers (their level still teaches at full value).

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
- **Varied trait-event deeds** — DONE (2026-07-25): `deed` became a `vector<string> deeds`
  (5 variants per trait), one picked at random via `pickRandom`, so "Osric, Brave as ever, holds
  the line steady." no longer repeats every other floor.
- **Self-bootstrapping exe**: embed default banks (raw string literals) and create
  `resources/`/`sessions/` on first run, writing only missing files. Deferred: a GitHub
  release zip covers distribution without a second source of truth.
- **Multi-team floors / expeditions** (idea from 2026-07-17): special floors that demand more
  than one team at once (one holds the gate while another pushes deeper, split wings, etc.).
  Design value: makes the *whole roster* matter (the bench gets stories too) and creates
  sacrifice decisions — which team takes the dangerous wing? Code impact is structural:
  `main` holds a single global `Team`; this needs a `vector<Team>` (or named teams), team
  management menus, and `runIncursion` accepting several teams. Phase 1+ material; pairs
  naturally with the team size cap below (several small teams instead of one big one).
- **Team size cap** (e.g. max 5): pending release feedback. Playtesting showed a 5-unit team
  steamrolls floors 1-7; a cap bounds trivialization and lets encounters be tuned against a
  known maximum power. One-line change when wanted: refuse in `Team::addMember` when
  `memberIds_.size()` hits the cap (plus a menu message). Note: a cap alone does not fix the
  early-floor flatness — the deeper dial is how `danger = 20 + floor*15` scales against team
  power; consider scaling danger with team size instead of (or besides) capping.

## First human session with recon-by-unit (2026-08-19)

Nine incursions, record floor 14, one death, no wipes. Roster ended at 18 units and **197 unspent
essence**. Six scouting missions, all six accepted, none fatal.

**Read this with the caveat the player gave afterwards:** the scouting choices were *instrumented,
not played*. The first mission was to see what happened, the second to see whether the scout would
die, and the last four were deliberate attempts to get him killed to check the death path. So this
session says **nothing** about whether the cost of scouting feels like a decision — a developer
testing a mechanic is not a player using it. (Lesson recorded: ask what someone was trying to do
before reading their behaviour as preference.)

What the six reports were, against the team's fit profile of Hold +24, Rescue +20, Retrieve -4,
Slay -8:

| # | floor | the scout's read | the team's fit |
|---|---:|---|---:|
| 1 | 9 | Cut down what holds the floor | **-8** |
| 2 | 11 | Free the captive | +20 |
| 3 | 12 | Hold the line for 5 rounds | +24 |
| 4 | 13 | Hold the line for 5 rounds | +24 |
| 5 | 14 | Hold the line for 5 rounds | +24 |
| 6 | 14 | Hold the line for 5 rounds | +24 |

**Not one report was followed by a change of team.** Both recompositions in the session happened
before any scouting, not in response to it. Four things explain it, and only the last is about the
player:

1. **The traits the decision depends on are invisible.** `printUnit` — the only thing that shows
   STR, CON, traits, history and hook — is called from exactly one place in the whole game
   (`main.cpp`, immediately after summoning). After that moment a unit can never be looked at
   again. `printRoster` shows id, name, stars, level, HP, XP and injuries; **no traits**. So the
   entire Phase 2b trait-fit system rests on an input the player cannot see. This is the broken
   first link, and it is cheap to fix.
2. **The missions do not do anything.** A floor is still one roll and one line of prose. `Hold the
   line for 5 rounds` does not produce five rounds to survive; `rounds` is generated, printed and
   never read. The four objective types differ only in which traits they favour — the type is
   flavour plus a scalar. So knowing the type cannot change much, because the type barely does
   anything.
3. **Scouting one floor is worthless when you climb eight.** In incursion 8 the player scouted
   floor 14 and then entered at floor 5, meeting all four objective types on the way up. With
   multi-floor climbs, what matters is a team's *average* across types, never one floor's type.
   This is arithmetic, not calibration.
4. **The fit's bad news is too small to act on.** Best case +24 (most of a 30-point forecast
   tier), worst case only -8 (a quarter of one). Good news never changes a decision because you
   were going anyway; bad news never changes one because it does not hurt enough. Cause: the trait
   table's columns were deliberately balanced near zero-sum so no objective type would be secretly
   easier — and a 5-unit team draws 8-10 traits from that table, so the sum concentrates in the
   middle. **Column balance kills variance.** Fewer units per expedition, or larger weights, or
   columns that are allowed to be genuinely punishing.

And the friction the player named, which is its own finding: assigning trainees and reassigning
units is tedious enough to discourage doing it, and there is no way to summon in bulk. With 197
essence spare and invocation at 5, the interesting number of summons is ten at a time, not one.

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
