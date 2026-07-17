# Project Ascend — Phase 0

A C++ console roguelike prototype with a single purpose: **validate a design hypothesis
before writing a single line of the full game**.

> **Hypothesis (GDD §1.1):** when a character generated with their own history dies forever,
> the player feels real loss.

All of Phase 0 exists to answer that question with the minimum possible game. There are no
graphics, no detailed combat, no inventory: there are characters with a name, a past and a
personality — and a tower that keeps them.

## What the prototype does

- **Generative summoning**: every unit is born with a name, a race, 1–2 personality traits
  and a backstory composed from data banks and templates (`app/resources/`).
- **The tower (push-your-luck)**: floor-by-floor incursions; on every floor, an encounter and
  a decision — climb higher or return with what you have. Danger grows; so does greed.
- **Traits that act**: the Cowardly leave gaps in the line, the Brave hold it steady, the
  Reckless charge in headfirst — and take the wounds (but are born tougher).
- **Real permadeath**: when a unit dies, it is erased. No resurrection, no save file that
  brings it back.
- **The Necropolis**: all that remains of the dead — a tombstone with who they were, where
  they fell and what killed them.
- **Hooks (the core mechanic)**: new summons can arrive marked by the fallen
  ("They are searching for whatever Nicodemus left behind on the walls"). Today's death
  writes tomorrow's story.

## Build and run

Requirements: MSVC (Visual Studio 2026 Community or equivalent) on Windows.

```
:: from a Developer Command Prompt (or after vcvars64.bat):
cd app
cl /W4 /w15038 /EHsc /nologo main.cpp src\*.cpp /Fe:ascend.exe
ascend.exe
```

Important: run it **from the `app/` folder** — the data banks are loaded through the relative
path `resources/`.

(With VSCode, the repo's build task does the same.)

## Structure

```
app/
  main.cpp          main loop and menus
  src/              one class per file (Unit, Roster, Team, Generator,
                    Incursion, Necropolis, GameState, Logger, Utils)
  resources/        generation data banks (names, traits, jobs, places,
                    motivations, templates, hooks, encounters)
  sessions/         one auto-recorded .log per game session
documentacio-tecnica.md   how every piece works and why
guia-estil-text.md        style rules for all console output
backlog-resolucions.md    parked ideas for Phase 1+ and saved material
```

Code conventions: Pre/Post contracts in headers, self-sufficient headers,
zero-warning policy with `/W4`.

## Status and how to evaluate

Steps 1–7 of the plan are complete (generation, tower, permadeath, hooks, style). Step 8 is
the evaluation: **play long sessions and answer honestly** — when the unit you had carried
since floor 1 died, did you feel anything?

If you try the game: don't read the code before playing. Summon, climb, risk one floor more
than you should, and see what happens to you when the tower takes its price. Then tell us.
Every session is recorded automatically to `app/sessions/` — attach your log when you share
feedback.
