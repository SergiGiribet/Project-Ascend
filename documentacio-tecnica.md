# Documentació tècnica — Project Ascend · Fase 0

Estat del codi a 17/07/2026. Complementa el GDD i l'especificació de Fase 0: allà hi ha el *què* i el *perquè*;
aquí hi ha el *com funciona el que ja està construït*. Vegeu també la [guia d'estil de text](guia-estil-text.md)
i el [backlog d'idees](backlog-resolucions.md).

---

## 1. Visió general

```
main.cpp ──── intro + bucle de menús + SessionLog (registre de partida)
   │
   ├── Roster ─── propietari únic de totes les Unit vives (vector<Unit>)
   │                └── Unit ─── el personatge (conté Stats)
   │
   ├── Team ────── ids de les unitats seleccionades (vector<int>) → resol contra el Roster
   │
   ├── Generator ─ fabrica unitats des dels bancs; consulta la Necropolis per als ganxos
   │
   ├── Incursion ─ la torre: encontres, esdeveniments de tret, XP, ferides i permadeath
   │
   ├── Necropolis ─ registres dels caiguts (DeathRecord), mai unitats
   │
   ├── GameState ── rècord de pis + comptador d'incursions + la Necropolis
   │
   └── Logger ───── TeeBuf/SessionLog: duplica tot el cout a sessions/session_*.log

app/resources/ ── 8 bancs de text per a la generació i els encontres
app/sessions/ ─── un .log per partida (ignorat per git; vegeu el seu README)
```

**Principi de propietat**: cada `Unit` viu en un únic lloc, el `Roster`. Tota altra classe hi refereix
per **id** (un `int`) i demana al Roster que el resolgui. Això evita el bug de les còpies: si dues
estructures guardessin còpies de la mateixa unitat, el dany aplicat a una no es veuria a l'altra.

---

## 2. Convencions del projecte

| Convenció | Regla |
|---|---|
| Contractes | Pre/Post en comentari **només al header**, sota cada declaració, **en anglès**, i sempre dient la veritat (si el codi canvia, el contracte canvia). |
| Un fitxer per classe | `X.h` (declaració + contractes) / `X.cpp` (implementació amb `X::`). |
| Includes | Cada fitxer inclou exactament el que usa; headers autosuficients. |
| Membres privats | Tots els atributs privats; sufix `_` a les classes noves. `Unit`/`Stats` són anteriors i no el porten. |
| Esborrar de vectors | Idioma **erase–remove**: `v.erase(std::remove(_if)(...), v.end())`. |
| Errors | Excepcions (`std::runtime_error`) per a violacions de contracte; `contains()` per validar entrada d'usuari abans de cridar; `try/catch` local als menús perquè un error d'usuari no mati el joc. |
| Sortida | Anglès, ASCII pur, formats de la guia d'estil. |
| Compilació | Sempre amb `/W4 /w15038` i zero avisos. |
| Aleatorietat | Un únic `std::mt19937` seedejat al `main`, passat per referència a tothom. |

---

## 3. Les classes

### 3.1 `Stats` (dins `Unit.h` / `Unit.cpp`)

Camps: `health`, `maxHealth`, `strength`, `constitution` (privats).

- `increaseHealth` / `decreaseHealth` fan **clamp**: la vida mai surt de `[0, maxHealth]`.
- `isAlive()` és **derivat** (`health > 0`) — una sola font de veritat.

### 3.2 `Unit` (`Unit.h` / `Unit.cpp`)

El personatge. Camps privats: `id`, `experience`, `stats`, `name`, `race` (1–6 \*), `level`,
`skills` (trets), `history` (rerefons generat), `hook` (lligam amb un caigut; buit si no en té).

- **Delegats de combat**: `takeDamage` / `heal` / `isAlive` deleguen a `stats`.
- ⚠ `getStats()` retorna **còpia** — per llegir, mai per modificar.
- `addExperience(n)` suma i **consumeix** la XP en pujar: mentre `experience >= level * 100`,
  resta el cost, puja un nivell i dona +10 maxHP (+10 HP), +1 STR, +1 CON. **Retorna els nivells
  guanyats** — la incursió ho usa per narrar els level-ups.
- `printUnit()` imprimeix la fitxa emmarcada (guia d'estil §3).

### 3.3 `Roster` (`Roster.h` / `Roster.cpp`)

La col·lecció de les unitats **vives**. Propietari únic (`std::vector<Unit> units_`).

- `findUnitById(int)` — dues sobrecàrregues (`Unit&` / `const Unit&`); llancen si l'id no existeix.
  ⚠ **Perill dangling**: usar la referència immediatament; `addUnit`/`removeUnitById` la invaliden.
- `contains(int)` — validació sense excepcions (menús).
- `removeUnitById(int)` — esborra si existeix; silenciós si no.

### 3.4 `Team` (`Team.h` / `Team.cpp`)

La selecció per a la incursió. **Guarda ids, mai unitats** (`std::vector<int> memberIds_`).

- `addMember(int, const Roster&)` — llança si l'id no és al roster o ja és a l'equip
  (els menús embolcallen amb try/catch local).
- `purgeDeadMembers(const Roster&)` — treu tot id que el roster ja no contingui
  (lambda `[&roster]` + erase–remove).
- `printTeam(const Roster&)` — línia d'unitat + sub-línia de ganxo (8 espais) si en té.

### 3.5 `Generator` (`Generator.h` / `Generator.cpp`)

Fabrica unitats des dels bancs. Constructor: carrega els **7 bancs** de `resources/`
(name, job, motivation, place, trait, template, hook) en un `map<string, vector<string>>`;
llança si algun és buit. Guarda una **referència** al rng (Pre: el rng ha de sobreviure'l).

`generateUnit(id, necropolis)`:
1. Nom aleatori; 1–2 trets **distints** (moneda + reroll).
2. Raça ponderada 1–6 (`discrete_distribution{40,25,15,10,7,3}`).
3. Stats de raça: `HP = 80 + raça*20`, `STR = CON = 8 + raça*2`.
   **Buff de naixement**: si té `Reckless`, +30 maxHP (+30 HP) i +5 CON — el tanc temerari.
4. Rerefons: `fillTemplate` resol cada forat `{banc}` amb una entrada aleatòria del banc.
5. **Ganxo (mecànica central)**: si la necròpolis no és buida, 40% de possibilitat de portar
   un ganxo amb `{fallen}` substituït pel nom d'un caigut aleatori.

### 3.6 `Necropolis` (`Necropolis.h` / `Necropolis.cpp`)

La memòria dels caiguts. Guarda `DeathRecord` (name, floorDied, cause, turn, skills), **mai** unitats.

- `addDeath(const Unit&, floor, cause, turn)` — cridar **abans** d'esborrar la unitat del roster.
- `pickRandom(rng)` — un registre aleatori per als ganxos; llança si és buida (per això `empty()`).
- `print()` — làpides, els més antics primer.
- Append-only: mai s'esborra res durant una sessió.

### 3.7 `GameState` (`GameState.h`, només header)

El progrés que persisteix entre incursions: `highestFloor` (rècord, mai baixa),
`incursionCount`, i la `necropolis`. El roster i l'equip viuen al `main` i es passen a part.

### 3.8 `Incursion` (`Incursion.h` / `Incursion.cpp`)

La torre. Dues peces:

**`Encounter` + `loadEncounters(path)`**: struct `{description, cause}` carregat
d'`encounters.txt` (format `descripcio|causa`, un per línia; llança si falta el fitxer,
és buit o una línia no té `|`).

**`runIncursion(team, roster, state, encounters, rng)`** — el bucle de pisos:

```
power  = suma de STR + CON de l'equip (recalculat cada pis)
danger = 20 + pis * 15
attack = power + luck(0..30) + modificadors d'esdeveniments de tret

attack >= danger*1.2 → "with ease"        (XP, sense dany)
attack >= danger     → "with difficulty"  (XP + un ferit: 15-40 de dany; pot morir)
attack <  danger     → "overwhelmed"      (mort directa d'un membre i fi de la incursió)
```

- **Tria de pis inicial**: si hi ha rècord, el jugador tria `[1 .. rècord+1]` (push-your-luck).
- **Encontre per pis**: un d'aleatori; la seva `description` presenta el pis i la seva `cause`
  va a la làpida si algú hi mor.
- **Esdeveniments de tret** (màx. 1 per pis, moneda del 50% si hi ha candidats): taula
  `TRAIT_EVENTS` a Incursion.cpp — `Brave` +8, `Cowardly` −12, `Reckless` +12 **i s'emporta
  la ferida** del pis (`forcedVictimId`). Els strings han de coincidir exactament amb `traits.txt`.
- **XP**: `pis * 10` per membre viu, a cada pis superat; level-ups narrats a l'acte.
- **Rècord protegit**: `if (floor > state.highestFloor)` — rejugar pisos baixos no el rebaixa.
- **Descans**: en acabar la incursió, els supervivents es curen del tot (les ferides només
  pesen *dins* de la incursió; els morts segueixen morts).

### 3.9 `Logger` (`Logger.h` / `Logger.cpp`)

El registre de partides (vegeu `app/sessions/README.md`).

- `TeeBuf` (subclasse de `std::streambuf`): `overflow`/`sync` dupliquen cada caràcter a dos
  buffers (consola + fitxer).
- `SessionLog` (RAII): al constructor obre `sessions/session_AAAAMMDD_HHMMSS.log` i instal·la
  el TeeBuf a `std::cout`; al destructor restaura el buffer original. Si el fitxer no s'obre,
  avisa i el joc continua sense gravar.
- ⚠ Ordre de membres al header (`file_`, `tee_`, `original_`) és **de càrrega**: `tee_` es
  construeix amb `file_.rdbuf()`, i l'ordre d'inicialització el mana la declaració, no l'init-list.

---

## 4. `main.cpp`

Ordre d'arrencada (dins del `try`): nom de log amb `strftime` → `SessionLog` (primera variable
de llarga vida: es destrueix l'última) → `Team`/`Roster`/`GameState` → `mt19937` → `Generator`
→ `loadEncounters` → `printIntro()` → bucle de menús.

- `printIntro()` — pantalla de lore inicial: to del joc + la regla que importa (permadeath).
- `readChoice()` (Utils) — únic lloc que imprimeix el prompt `> `; retorna 0 si l'entrada no és
  un número (cau al `default` del switch).
- Els errors d'usuari (id inexistent...) es capturen amb try/catch **local**; el catch extern
  només veu errors fatals (bancs que falten...).

---

## 5. Bancs de dades (`app/resources/`)

Un valor per línia, sense línies buides (els loaders les salten).

| Fitxer | Contingut | Contracte gramatical |
|---|---|---|
| `names.txt` (60) | Noms propis | Sense puntuació |
| `places.txt` | Topònims | Sense puntuació |
| `jobs.txt` | Oficis | Substantiu singular; encaixa a «a {job}» |
| `traits.txt` (10) | Trets | **Adjectiu** amb majúscula inicial; els mecànics (`Brave`, `Cowardly`, `Reckless`) han de coincidir exactament amb `TRAIT_EVENTS` |
| `motivations.txt` | Motivacions | Frase completa en 3a persona, sense punt final |
| `templates.txt` | Plantilles de rerefons | Forats `{job}` `{place}` `{motivation}`; la plantilla posa tota la puntuació |
| `hooks.txt` | Ganxos | Frase amb el forat `{fallen}` (nom d'un caigut); sense punt final (el posa el Generator) |
| `encounters.txt` (35) | Encontres | `descripcio\|causa`. Descripció sense punt final. **Causa = mort de la unitat**: minúscula, sense punt, i ha de completar «fell on floor N, \<causa\>.» amb la unitat com a subjecte |

**Regla d'or de puntuació**: les entrades de banc mai porten punt final; tota la puntuació la posa
qui compon la frase (plantilla o codi). Si surt un «..» o falta un punt, alguna entrada ha violat la regla.

---

## 6. Compilació i execució

- **Tasca del VSCode**: compila `app/main.cpp` + `app/src/*.cpp` amb `/Zi /EHsc /W4 /w15038`,
  surt a `app/ascend.exe`. Qualsevol `.cpp` nou a `src/` s'inclou sol.
- **Manual**: `cl /EHsc /W4 /w15038 /nologo /Feascend.exe main.cpp src\*.cpp` des d'`app/`.
- ⚠ `LNK1168` = l'exe s'està executant; tanca'l (opció 9) i recompila.
- ⚠ Rutes relatives (`resources/`, `sessions/`) es resolen des del **directori d'execució**:
  executa sempre des d'`app/`.
- Artefactes i `.log` coberts pel `.gitignore`.

---

## 7. El flux de mort (implementat — l'ordre importa)

```
1. necropolis.addDeath(unit, pis, enc.cause, torn)  // registrar mentre la unitat encara existeix
2. roster.removeUnitById(id)                        // permadeath real: la Unit es destrueix
3. team.purgeDeadMembers(roster)                    // l'equip neteja els ids orfes
```

Invertir 1 i 2 = registrar una unitat que ja no existeix. La causa de la làpida és la de
l'**encontre** del pis on ha caigut — cada mort té culpable i escena.

---

## 8. Full de ruta (vs. pla del GDD §6)

| Pas GDD | Estat |
|---|---|
| 1. Unit + imprimir | ✅ |
| 2. Roster + menú | ✅ |
| 3. Generador amb bancs | ✅ (7 bancs, raça ponderada, fillTemplate) |
| 4. Incursió | ✅ (push-your-luck amb tria de pis, XP, level-ups) |
| 5. Permadeath + Necròpolis | ✅ (flux registrar→esborrar→purgar) |
| 6. Ganxos condicionats per la Necròpolis | ✅ — LA MECÀNICA CENTRAL FUNCIONA |
| 7. Polir narració | ✅ (guia d'estil + passada completa de textos) |
| 8. Jugar i avaluar (§1.1) | ⬜ ← ARA. Sessions llargues; després de cada mort: què he sentit, i era pèrdua de *recurs* o de *persona*? |

**Afegits fora de pla** (tots al servei del pas 8): encontres amb causes de mort lligades,
esdeveniments de tret en combat (+ arquetip Reckless), cura per descans entre incursions,
pantalla d'intro, registre automàtic de partides a `sessions/`, README del projecte,
[guia d'estil](guia-estil-text.md) i [backlog](backlog-resolucions.md).
