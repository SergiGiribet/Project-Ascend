# Documentació tècnica — Project Ascend · Fase 0

Estat del codi a 14/07/2026. Complementa el GDD i l'especificació de Fase 0: allà hi ha el *què* i el *perquè*;
aquí hi ha el *com funciona el que ja està construït*.

---

## 1. Visió general

```
main.cpp ──── bucle de menús (UI de text)
   │
   ├── Roster ─── propietari únic de totes les Unit vives (vector<Unit>)
   │                │
   │                └── Unit ─── el personatge (conté Stats)
   │
   ├── Team ────── ids de les unitats seleccionades (vector<int>) → resol contra el Roster
   │
   ├── Necropolis ─ [PENDENT] registres dels caiguts (DeathRecord), mai unitats
   │
   └── GameState ── [PENDENT] agrupa roster + necropolis + pisMesAlt + torn

app/resources/ ── bancs de text per al generador (pas 3, pendent de connectar)
```

**Principi de propietat**: cada `Unit` viu en un únic lloc, el `Roster`. Tota altra classe hi refereix
per **id** (un `int`) i demana al Roster que el resolgui. Això evita el bug de les còpies: si dues
estructures guardessin còpies de la mateixa unitat, el dany aplicat a una no es veuria a l'altra.

---

## 2. Convencions del projecte

| Convenció | Regla |
|---|---|
| Contractes | Pre/Post en comentari **només al header**, sota cada declaració. El cpp no els repeteix. |
| Un fitxer per classe | `X.h` (declaració + contractes) / `X.cpp` (implementació). |
| Includes | Cada fitxer inclou exactament el que usa (no confiar en inclusions transitives). |
| Membres privats | Sufix `_` a les classes noves (`units_`, `memberIds_`). `Unit`/`Stats` són anteriors i no el porten. |
| Esborrar de vectors | Idioma **erase–remove**: `v.erase(std::remove(_if)(...), v.end())`. |
| Errors | Excepcions (`std::runtime_error`) per a violacions de contracte; `contains()` per validar entrada d'usuari abans de cridar. |
| Compilació | Sempre amb `/W4 /w15038` i zero avisos. |

---

## 3. Les classes

### 3.1 `Stats` (dins `Unit.h` / `Unit.cpp`)

Camps: `health`, `maxHealth`, `strength`, `constitution` (tots privats).

- `increaseHealth` / `decreaseHealth` fan **clamp**: la vida mai surt de `[0, maxHealth]`
  (`std::min` / `std::max`). Gràcies a això, el codi de combat pot repartir dany sense comprovar límits.
- `isAlive()` és **derivat**: `health > 0`. No hi ha cap flag booleà separat — una sola font de veritat,
  impossible que es contradigui.
- `printStats()` imprimeix les estadístiques per consola.

### 3.2 `Unit` (`Unit.h` / `Unit.cpp`)

El personatge. Camps privats: `id`, `experience`, `stats` (un objecte `Stats`), `name`, `race` (1–6 ★),
`level`, `skills` (vector de trets), `history` (rerefons generat), `hook` (lligam amb un caigut; buit si no en té).

- Dos constructors: per defecte (valors estàndard) i complet.
- **Delegats de combat**: `takeDamage(n)` / `heal(n)` / `isAlive()` deleguen a `stats`. El codi de joc
  parla amb la unitat, mai amb les seves tripes.
- Atenció: `getStats()` retorna **còpia** — serveix per llegir, no per modificar. Per ferir/curar,
  sempre els delegats.
- `addExperience(n)` suma; `setExperience(n)` estableix. Els noms diuen la veritat.
- `printUnit()` imprimeix la fitxa sencera (inclou `stats.printStats()`).

### 3.3 `Roster` (`Roster.h` / `Roster.cpp`)

La col·lecció de les unitats **vives**. Propietari únic (`std::vector<Unit> units_`).

- `addUnit(const Unit&)` — pren possessió (hi entra una còpia: l'única còpia legítima del projecte,
  perquè és el moment de la ingesta).
- `findUnitById(int)` — **dues sobrecàrregues**:
  - `Unit&` (roster modificable): per aplicar dany/XP a la unitat real.
  - `const Unit&` (roster const): per consultar. El compilador tria sola segons el context.
  - Totes dues llancen `std::runtime_error` si l'id no existeix.
  - ⚠ **Perill dangling**: la referència apunta dins del vector. Usar-la immediatament i no
    guardar-la mai més enllà d'un `addUnit`/`removeUnitById` (el vector es pot recol·locar).
- `contains(int) -> bool` — validació sense excepcions (per a entrada d'usuari als menús).
- `removeUnitById(int)` — esborra si existeix, no fa res si no. **No** comprova `isAlive`:
  esborrar és esborrar (el flux de permadeath esborra precisament unitats mortes).
- `printRoster()` — llista resumida (id, nom, raça, nivell).

### 3.4 `Team` (`Team.h` / `Team.cpp`)

La selecció per a la incursió. **Guarda ids, mai unitats** (`std::vector<int> memberIds_`).

- `addMember(int id, const Roster&)` — valida dues coses i llança si fallen:
  1. l'id existeix al roster (`roster.contains(id)`);
  2. l'id no està ja a l'equip (`std::find` sobre `memberIds_`).
- `removeMember(int id)` — erase–remove per valor; silenciós si no hi era.
- `purgeDeadMembers(const Roster&)` — treu tot id que el roster ja no contingui. La lambda captura
  el roster per referència: `[&roster](int id) { return !roster.contains(id); }`.
- `printTeam(const Roster&)` — resol cada id amb la sobrecàrrega `const` de `findUnitById` i
  imprimeix la fitxa breu. Exemple viu de per què existeixen les dues sobrecàrregues.

Com que `addMember` llança excepcions i l'id vindrà del teclat, **els menús han de validar abans
o embolcallar amb `try/catch`**.

### 3.5 `Necropolis` — PENDENT (contracte escrit al header)

- Guarda `DeathRecord` (name, floorDied, cause, turn, skills), **mai** unitats mortes.
- API prevista: `addDeath(const Unit&, floor, cause, turn)` · `pickRandom(rng)` (per als ganxos
  del pas 6) · `print()`.
- Append-only: mai s'esborra res durant una sessió.

### 3.6 `GameState` — PENDENT

Agruparà `roster + necropolis + pisMesAlt + torn` (GDD §3.3) per no passar quatre paràmetres
a cada funció. No urgent fins que el nombre de paràmetres molesti.

---

## 4. `main.cpp`

- **Bucle principal** `do-while` amb menú de text; submenús (equip, incursió) amb el mateix patró.
- `readChoice()` — lectura robusta d'enters: si `cin >> c` falla (lletres, EOF), fa
  `cin.clear()` (treu l'estat d'error) + `cin.ignore(...)` (llença la línia dolenta) i retorna 0,
  que cau al `default` del switch. Sense això, una lletra provoca bucle infinit.
- Cada `case` porta claus `{}` pròpies per poder declarar variables locals.
- **Pendent de cablejar**: instàncies de `Roster`/`Team` abans del bucle, comptador `nextId`,
  i que els menús cridin els mètodes reals.

---

## 5. Bancs de dades (`app/resources/`)

Un valor per línia, sense línies buides. `loadBank()` (pendent d'escriure) ha de saltar línies buides.

| Fitxer | Contingut | Contracte gramatical |
|---|---|---|
| `names.txt` (20) | Noms propis | Sense puntuació |
| `places.txt` (20) | Topònims | Sense puntuació |
| `jobs.txt` (8) | Oficis | Substantiu singular; encaixa a «a {job}» |
| `traits.txt` (10) | Trets de personalitat | **Adjectiu**; meitat virtuts, meitat defectes |
| `motivations.txt` (12) | Motivacions | **Frase completa en 3a persona** («They ...»), sense punt final |
| `templates.txt` (14) | Plantilles de rerefons | Forats `{job}` `{place}` encastats; `{motivation}` només com a frase independent al final |

**Regla d'or de puntuació**: les entrades de banc mai porten punt final; **tota** la puntuació la posa
la plantilla. Si una frase generada surt amb «..» o sense punt, alguna entrada ha violat la regla.

**Mecanisme previst (pas 3)**: `map<string, vector<string>> banks` amb la clau = nom del banc; el forat
`{job}` es resol amb una entrada aleatòria de `banks.at("job")`. Afegir un banc nou = crear el fitxer +
registrar-lo al map; zero canvis de lògica. Aleatorietat: **un únic** `std::mt19937` seedejat al `main`
amb `std::random_device`, passat per referència; `uniform_int_distribution` per triar; per la raresa (★),
`discrete_distribution` amb pesos decreixents.

---

## 6. Compilació i execució

- **Tasca del VSCode** (`.vscode/tasks.json`): compila `app/main.cpp` + `app/src/*.cpp` amb
  `/Zi /EHsc /W4 /w15038`, surt a `app/ascend.exe`. Qualsevol `.cpp` nou a `src/` s'inclou sol.
- **Manual (Developer Command Prompt o vcvars64)**:
  `cl /EHsc /W4 /w15038 /nologo /Feascend.exe main.cpp src\*.cpp` des d'`app/`.
- ⚠ `LNK1168: cannot open ascend.exe for writing` = l'exe està **executant-se**; tanca'l (opció 9) i recompila.
- ⚠ Les rutes relatives (`resources/names.txt`) es resolen des del **directori d'execució**, no des d'on
  és l'exe: executa sempre des d'`app/`.
- Els artefactes (`.obj`, `.exe`, `.pdb`...) estan coberts pel `.gitignore`.

---

## 7. El flux de mort (disseny acordat, pendent d'implementar)

Quan una unitat cau a la incursió, **l'ordre importa**:

```
1. necropolis.addDeath(unit, pis, causa, torn)   // registrar mentre la unitat encara existeix
2. roster.removeUnitById(unit.getId())           // permadeath real: la Unit es destrueix
3. team.purgeDeadMembers(roster)                 // l'equip neteja els ids orfes
```

Invertir 1 i 2 = intentar registrar una unitat que ja no existeix.

---

## 8. Full de ruta (vs. pla del GDD §6)

| Pas GDD | Estat |
|---|---|
| 1. Unit + imprimir | ✅ Fet |
| 2. Roster + menú | ✅ Fet (roster viu al main, invoke + view stats reals) |
| 3. Generador amb bancs | ✅ Fet (Generator amb 6 bancs, rasa ponderada, fillTemplate) |
| 4. Incursió (selecció + resolució + XP) | 🟡 Team fet (la selecció); falta la resolució ← ARA |
| 5. Permadeath + Necròpolis | ⬜ Contracte escrit; falta implementar |
| 6. Ganxos condicionats per la Necròpolis | ⬜ La mecànica central de l'experiment |
| 7. Polir narració | ⬜ |
| 8. Jugar i avaluar (§1.1) | ⬜ El veritable objectiu |
