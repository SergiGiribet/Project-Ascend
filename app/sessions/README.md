# sessions/ — game session records

Every Project Ascend session records itself here: everything printed to the console (intro,
menus, incursion chronicles, deaths, tombstones) is mirrored to a file

```
session_YYYYMMDD_HHMMSS.log
```

one per run, named with its start date and time.

## What it is for

This record exists for the **goal of Phase 0** (GDD §1.1): evaluating whether the permanent
death of units with generated histories produces real loss in the player.

- **Self-evaluation (step 8)**: re-read sessions cold, reconstruct how each unit died and
  contrast it with what was felt in the moment. Notes on "what did I feel, and was it
  resource-loss or person-loss" can be written alongside, referencing the session file.
- **External testers**: when someone plays the game and says "I lost this unit and it stung",
  they can attach their `.log` and their exact story can be re-read — no need to rely on
  their memory.

## How it works (technical summary)

`SessionLog` ([src/Logger.h](../src/Logger.h)) replaces `std::cout`'s buffer with a `TeeBuf`
that writes every character to the console and to the file at the same time — which is why
no `cout` in the game had to change. It is created at the start of `main` and its destructor
restores `cout` on exit.

Known limitations (accepted by design):

- **What the player types is not recorded** (keyboard echo does not go through `cout`);
  answers can be inferred from the next line of the log.
- If the game dies with a fatal error, the `catch` message goes to the console only (the log
  has already been closed safely by then).
- If the file cannot be opened, the game warns and continues without recording — a session is
  never lost because of the recorder.

## Git

The `.log` files are ignored (`*.log` in the root `.gitignore`): sessions are local to each
machine. This README is versioned — and it also makes the folder exist in fresh clones.
