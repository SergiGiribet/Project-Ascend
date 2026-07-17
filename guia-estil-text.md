# Guia d'estil de text — Project Ascend · Fase 0

Regles perquè tota la sortida per consola del joc segueixi la mateixa lògica i estètica.
Referència viva: si un text nou no sap com ser, que copiï un dels formats canònics d'aquí.

---

## 1. Principis

1. **La narració crea el vincle.** Cada esdeveniment que afecta una unitat es narra amb el seu **nom**
   i, si en té, un **tret** teixit a la frase. «Unit_03 ha mort (HP 0)» està prohibit per GDD (§4.1).
2. **Anglès in-game, ASCII pur.** Res d'accents, guions llargs (—), cometes tipogràfiques ni `★`
   a cap `cout`: la consola de Windows els destrossa. L'estrella és `*`, el guió és `-`.
3. **La puntuació la posa qui compon la frase**, mai les dades. Les entrades de banc no porten punt
   final; les plantilles i el codi que narren posen tota la puntuació (vegeu §5 de la documentació tècnica).
4. **Cap sortida mentidera.** Si una acció no fa res (id inexistent, banc buit), el text ho diu;
   mai un missatge d'èxit per una acció que ha fallat.

## 2. Elements visuals

| Element | Forma | Exemple |
|---|---|---|
| Capçalera de secció/menú | `=== Títol ===`, amb **línia en blanc abans** | `=== The Necropolis (3 fallen) ===` |
| Opcions de menú | 2 espais + `N. Acció` | `  1. Invoke a new unit` |
| Prompt d'entrada | `> ` — **només l'imprimeix `readChoice()`**, mai a mà | `> ` |
| Pregunta amb opcions | `Pregunta? [1] Opcio  [2] Opcio` (2 espais entre opcions) | `Climb to floor 5? [1] Yes  [2] Return` |
| Element de llista | 2 espais d'indentacio | `  [3] Beatrice (2*) - Lv 1 - ...` |
| Sub-linia d'un element (ganxo) | 8 espais | `        They found Leander's journal...` |
| Esdeveniment dins d'un pis | 2 espais sota la linia `Floor N:` | `  Hazel reaches level 2!` |
| Fitxa d'unitat | Emmarcada amb `----------------------------------------` (40 guions) | vegeu §3 |
| Separador de camps en una linia | ` \| ` o ` - ` (coherent dins de la mateixa linia) | `Lv 2 - HP 85/120 - XP 40` |

**Línies en blanc**: una abans de cada capçalera `===` i abans de cada fitxa d'unitat. Mai dues seguides.

## 3. Formats canònics

**Línia d'unitat** (roster, team, supervivents — sempre idèntica):
```
  [<id>] <Nom> (<rasa>*) - Lv <nivell> - HP <actual>/<max> - XP <xp>
```

**Fitxa d'unitat** (`printUnit`, en invocar):
```
----------------------------------------
  <Nom> (<rasa>*)  [ID <id>]
  Level <n>  |  XP <xp>
  HP <a>/<m>  |  STR <s>  |  CON <c>
  Traits: <tret>, <tret>
  Story:  <rerefons>
  Hook:   <ganxo>              <- nomes si en te
----------------------------------------
```

**Làpida** (Necròpolis):
```
  <Nom>, <Tret> and <Tret> - fell on floor <pis> (incursion <n>), <causa>.
```

**Crònica d'incursió** (la línia `Floor N:` presenta l'ENCONTRE; tot el que passa va indentat a sota,
en aquest ordre: esdeveniment de tret → desenllaç → level-ups → ferida/mort):
```
Floor 4: A drake coils around the stairwell, breathing fire.
  Osric, Brave as ever, holds the line steady.
  The team advances with difficulty.
  Hazel reaches level 2!
  Beatrice, Reckless as ever, is wounded.
Climb to floor 5? [1] Yes  [2] Return
```

**Tancament d'incursió** (el descans només s'imprimeix si queda algú viu):
```
=== Incursion <n> over ===
Highest floor this run: <x>  |  Tower record: <y>
The survivors rest and tend their wounds.
<llista de supervivents amb el format de linia d'unitat>
```

**Pantalla de text llarg** (intro i futures pantalles narratives):
```
=== TITOL ===

Paragrafs separats per una linia en blanc. Linies de menys
de 60 caracters (consoles estretes). El guio es "--", mai
un em-dash. Tanca amb una pregunta o una frase que ressoni.
```

## 4. Patrons de narració

- **Tret teixit**: `<Nom>, <tret> as ever,` per a accions/ferides; `<Nom>, <tret> to the end,` per a morts.
  La frase es construeix per trossos i el tros del tret nomes s'afegeix si la unitat en te
  (`if (!skills.empty())`) — mai una frase duplicada en if/else.
- **Esdeveniment de tret** (TRAIT_EVENTS a Incursion.cpp): `<Nom>, <Tret> as ever, <gesta>.` —
  la gesta en minuscula, present simple, sense subjecte (`holds the line steady`). El nom del tret
  ha de coincidir EXACTAMENT amb l'entrada de traits.txt.
- **Causes de mort** (segon camp d'encounters.txt): minuscula, sense punt (la lapida el posa), i han
  de passar el TEST DE LA LAPIDA: llegir `fell on floor N, <causa>.` amb la unitat morta com a
  subjecte — `burned to ash by the drake` passa; `silenced by a ringing charm` (aixo li passa al
  monstre, no a la unitat) NO passa. Una causa nomes la poden dir la lapida i la linia de caiguda:
  posar-la en boca d'un viu es mentir.
- **Frases fetes del joc** (no canviar-les a la lleugera, son identitat):
  - `The tower claims them all. No one returns.` (extermini)
  - `The team descends with their spoils and their lives.` (retirada voluntaria)
  - `The survivors rest and tend their wounds.` (descans post-incursio; nomes si queda algu)
  - `The summoning circle glows...` (invocacio)
  - `The tower will be waiting. Goodbye!` (sortida)
- **Separador de llistes de trets**: `, ` en fitxes i llistes; ` and ` a les lapides (llegeix mes solemne).
  Sempre amb el patro del `first` (separador abans de cada element menys el primer).

## 5. Regles per a textos nous

1. Tria el format canonic mes proper (§3) i copia'l exactament.
2. Si es un esdeveniment sobre una unitat: nom sempre, tret si en te (patro §4).
3. Si es un menu o pregunta: capçalera `===` o pregunta `[1]/[2]`, i l'entrada la llegeix `readChoice()`.
4. ASCII pur, angles, sense punt final a les dades, punt final a les frases narrades.
5. Compila amb `/W4` i mira la sortida real abans de donar-ho per bo: els espais que falten
   («Keirareaches») nomes es veuen executant.
