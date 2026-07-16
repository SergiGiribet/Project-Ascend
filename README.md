# Project Ascend — Fase 0

Prototip de roguelike de consola en C++ amb un sol objectiu: **validar una hipòtesi de disseny
abans d'escriure ni una línia del joc gran**.

> **Hipòtesi (GDD §1.1):** quan un personatge generat amb la seva pròpia història mor per sempre,
> el jugador sent una pèrdua real.

Tota la Fase 0 existeix per respondre aquesta pregunta amb el mínim de joc possible. No hi ha
gràfics, no hi ha combat detallat, no hi ha inventari: hi ha personatges amb nom, passat i
personalitat — i una torre que se'ls queda.

## Què fa el prototip

- **Invocació generativa**: cada unitat neix amb nom, raça, 1–2 trets de personalitat i una
  història composta a partir de bancs de dades i plantilles (`app/resources/`).
- **La torre (push-your-luck)**: incursions pis a pis; a cada pis, un encontre i una decisió —
  pujar més o tornar amb el que tens. El perill creix; la cobdícia també.
- **Trets que actuen**: el covard deixa forats a la línia, el valent l'aguanta, el temerari
  carrega de cap — i s'emporta les ferides (però neix més dur).
- **Permadeath de veritat**: quan una unitat mor, s'esborra. No hi ha resurrecció ni partida
  guardada que la retorni.
- **La Necròpolis**: l'únic que queda dels morts — una làpida amb qui era, on va caure i què
  el va matar.
- **Ganxos (la mecànica central)**: les noves invocacions poden arribar marcades pels caiguts
  ("They are searching for whatever Nicodemus left behind on the walls"). La mort d'avui
  escriu la història de demà.

## Compilar i executar

Requisits: MSVC (Visual Studio 2026 Community o equivalent) a Windows.

```
:: des d'un Developer Command Prompt (o despres de vcvars64.bat):
cd app
cl /W4 /w15038 /EHsc /nologo main.cpp src\*.cpp /Fe:ascend.exe
ascend.exe
```

Important: executa'l **des de la carpeta `app/`** — els bancs de dades es carreguen amb la ruta
relativa `resources/`.

(Amb VSCode, la task de build del repositori fa el mateix.)

## Estructura

```
app/
  main.cpp          bucle principal i menus
  src/              una classe per fitxer (Unit, Roster, Team, Generator,
                    Incursion, Necropolis, GameState, Utils)
  resources/        bancs de dades de generacio (noms, trets, oficis, llocs,
                    motivacions, plantilles, ganxos, encontres)
documentacio-tecnica.md   com funciona cada peca i per que
guia-estil-text.md        normes d'estil de tota la sortida per consola
backlog-resolucions.md    idees aparcades per a Fase 1+ i material guardat
```

Convencions del codi: contractes Pre/Post als headers, headers autosuficients,
política de zero warnings amb `/W4`.

## Estat i com avaluar

Passos 1–7 del pla complets (generació, torre, permadeath, ganxos, estil). El pas 8 és
l'avaluació: **jugar sessions llargues i respondre honestament** — quan ha mort la unitat que
portaves des del pis 1, has sentit alguna cosa?

Si proves el joc: no llegeixis el codi abans de jugar. Invoca, puja, arrisca't un pis més del
que toca, i mira què et passa quan la torre es cobra el preu. Després explica-ho.
