# Text style guide — Project Ascend · Phase 0

Rules so that all of the game's console output follows the same logic and aesthetics.
A living reference: when a new text doesn't know what to look like, it copies one of the
canonical formats below.

---

## 1. Principles

1. **Narration creates the bond.** Every event that affects a unit is narrated with its
   **name** and, if it has one, a **trait** woven into the sentence. "Unit_03 died (HP 0)"
   is forbidden by the GDD (§4.1).
2. **English in-game, pure ASCII.** No accents, em dashes (—), curly quotes or `★` in any
   `cout`: the Windows console mangles them. The star is `*`, the dash is `-`.
3. **Punctuation belongs to whoever composes the sentence**, never to the data. Bank entries
   carry no trailing period; templates and narrating code own all punctuation (see §5 of the
   technical documentation).
4. **No lying output — about what the game did.** If an action does nothing (nonexistent id,
   empty bank), the text says so; never a success message for an action that failed. Numbers
   shown to the player are the numbers the simulation actually used.
5. **Characters may be wrong; the narration may not.** A scouting report is not a result, it is
   a **claim by a named unit**, and it is allowed to be incomplete or mistaken — that is the
   whole point of *who you send* mattering. Three rules keep an unreliable report readable as
   personality instead of as a bug:
   - **Always attributed.** Never "the scouts"; always "Yorick". The player has to be able to
     blame a person.
   - **A claim is framed as a claim.** `Their read: Hold the line for 3 rounds.` — never
     `Floor 4 holds ...`. The narration faithfully reports that the unit said it; it never
     asserts it as fact on its own authority.
   - **The truth arrives, and it names them.** When the floor turns out otherwise, a line says
     so with the scout's name, so the mistake lands on the character and not on the game.

## 2. Visual elements

| Element | Form | Example |
|---|---|---|
| Section/menu header | `=== Title ===`, with a **blank line before** | `=== The Necropolis (3 fallen) ===` |
| Menu options | 2 spaces + `N. Action` | `  1. Invoke a new unit` |
| Input prompt | `> ` — **printed only by `readChoice()`**, never by hand | `> ` |
| Question with options | `Question? [1] Option  [2] Option` (2 spaces between options) | `Climb to floor 5? [1] Yes  [2] Return` |
| List item | 2-space indent | `  [3] Beatrice (2*) - Lv 1 - ...` |
| Sub-line of an item (hook) | 8 spaces | `        They found Leander's journal...` |
| Event inside a floor | 2 spaces under the `Floor N:` line | `  Hazel reaches level 2!` |
| Unit card | Framed with `----------------------------------------` (40 dashes) | see §3 |
| Field separator within a line | ` \| ` or ` - ` (consistent within the same line) | `Lv 2 - HP 85/120 - XP 40` |

**Blank lines**: one before every `===` header and before every unit card. Never two in a row.

## 3. Canonical formats

**Unit line** (roster, team, survivors — always identical):
```
  [<id>] <Name> (<race>*) - Lv <level> - HP <current>/<max> - XP <xp>
```

**Unit card** (`printUnit`, on summoning):
```
----------------------------------------
  <Name> (<race>*)  [ID <id>]
  Level <n>  |  XP <xp>
  HP <hp>/<max>  |  STR <s>  |  CON <c>
  Traits: <trait>, <trait>
  Story:  <backstory>
  Hook:   <hook>              <- only if it has one
----------------------------------------
```

**Tombstone** (Necropolis):
```
  <Name>, <Trait> and <Trait> - fell on floor <floor> (incursion <n>), <cause>.
```

**Incursion chronicle** (the `Floor N:` line presents the ENCOUNTER; everything that happens is
indented below it, in this order: trait event → outcome → essence yield → level-ups →
wound/death; the danger forecast is unindented — it is the narrator speaking about what's
above, not an event of this floor):
```
Floor 4: A drake coils around the stairwell, breathing fire.
  Osric, Brave as ever, holds the line steady.
  The team advances with difficulty.
  The floor yields 4 essence.
  Hazel reaches level 2!
  Beatrice, Reckless as ever, is wounded.
The air grows heavier.
Climb to floor 5? [1] Yes  [2] Return
```

**Danger forecast** (exactly these four lines, one per tier — they are computed from real
odds and must stay truthful; phrased as feelings because trait events can still shift the roll):
```
The way up looks clear.                                <- guaranteed success
The air grows heavier.                                 <- luck decides between ease and wounds
Something waits above, and it is not afraid of you.    <- only max luck survives
Climbing further is death.                             <- mathematically guaranteed defeat
```

**Incursion closing** (the rest line prints only if someone is left alive):
```
=== Incursion <n> over ===
Highest floor this run: <x>  |  Tower record: <y>
The survivors rest and tend their wounds.
<survivor list using the unit line format>
```

**Long text screen** (intro and future narrative screens):
```
=== TITLE ===

Paragraphs separated by one blank line. Lines under 60
characters (narrow consoles). The dash is "--", never an
em dash. Close with a question or a line that lingers.
```

## 4. Narration patterns

- **Woven trait**: `<Name>, <trait> as ever,` for actions/wounds; `<Name>, <trait> to the end,`
  for deaths. The sentence is built piece by piece and the trait piece is only added when the
  unit has one (`if (!skills.empty())`) — never a duplicated sentence in an if/else.
- **Trait event** (DAMAGE_TRAIT_EVENTS in Incursion.cpp): `<Name>, <Trait> as ever, <deed>.` — the
  deed in lowercase, simple present, no subject (`holds the line steady`). The trait name must
  match the traits.txt entry EXACTLY.
- **Death causes** (second field of encounters.txt): lowercase, no period (the tombstone adds
  it), and they must pass the TOMBSTONE TEST: read `fell on floor N, <cause>.` with the dead
  unit as the subject — `burned to ash by the drake` passes; `silenced by a ringing charm`
  (that happens to the monster, not the unit) does NOT. A cause may only be spoken by the
  tombstone and the falling line: putting it in a living unit's mouth is lying.
- **The game's signature phrases** (don't change them lightly — they are identity):
  - `The tower claims them all. No one returns.` (wipe)
  - `The team descends with their spoils and their lives.` (voluntary retreat)
  - `The survivors rest and tend their wounds.` (post-incursion rest; only if anyone remains)
  - `The summoning circle glows...` (summoning)
  - `The tower will be waiting. Goodbye!` (exit)
- **Trait list separator**: `, ` in cards and lists; ` and ` on tombstones (reads more solemn).
  Always with the `first` pattern (separator before every element except the first).

## 5. Color

Colors are ANSI escape sequences (constants in Utils.h; `enableConsoleColors()` activates them
at startup). Three rules:

1. **Color reinforces, never carries.** The text must say everything on its own — session logs
   are colorless (TeeBuf strips ANSI codes from the file) and not every eye tells green from
   red. That's why the in-team highlight also prints a textual `[in team]` tag.
2. **Semantic palette, used sparingly**:
   - `COLOR_GREEN` / `COLOR_YELLOW` / `COLOR_RED` — risk and scarcity (danger forecast tiers).
     `COLOR_RED` also marks **permanent injuries** wherever a unit is listed (a lasting harm,
     shown with its stat cost as `{name (-cost)}` on the roster and an `Injuries:` line on the
     card — both formatted by `injuryLabel`).
   - `COLOR_CYAN` — identity/membership (the essence counter, units in the team `[in team]`).
   - `COLOR_MAGENTA` / `COLOR_BLUE` — training-camp roles on the roster: magenta `[trainer]`,
     blue `[trainee]` (deliberately off the green/yellow/red risk palette).
   - Nothing else is colored. A new color needs a new *meaning*, not a new decoration.
3. **Reset discipline**: every color opens right before the text it paints and closes with
   `COLOR_RESET` right after the last painted character, before the `std::endl`. An unclosed
   color tints the whole console.

## 6. Rules for new texts

1. Pick the closest canonical format (§3) and copy it exactly.
2. If it is an event about a unit: always the name, the trait when it has one (§4 pattern).
3. If it is a menu or question: `===` header or `[1]/[2]` question, and input is read by
   `readChoice()`.
4. Pure ASCII, English, no trailing period in data, full punctuation in narrated sentences.
5. Compile with `/W4` and look at the real output before calling it done: missing spaces
   ("Keirareaches") only show up at runtime.
