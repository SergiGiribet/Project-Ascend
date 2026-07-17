# sessions/ — registre de partides

Cada partida de Project Ascend es grava sola aquí: tot el que surt per consola (intro, menús,
cròniques d'incursió, morts, làpides) es duplica a un fitxer

```
session_AAAAMMDD_HHMMSS.log
```

un per execució, amb la data i hora d'inici al nom.

## Per a què serveix

Aquest registre existeix per l'**objectiu de la Fase 0** (GDD §1.1): avaluar si la mort permanent
d'unitats amb història generada produeix pèrdua real en el jugador.

- **Autoavaluació (pas 8)**: rellegir les partides en fred, reconstruir com va morir cada unitat
  i contrastar-ho amb el que es va sentir al moment. Les notes de "què he sentit i de quin tipus
  era (recurs o persona)" es poden escriure al costat, referenciant el fitxer de sessió.
- **Provadors externs**: quan algú provi el joc i expliqui "he perdut tal unitat i em va saber
  greu", pot adjuntar el seu `.log` i es pot rellegir exactament la seva història — sense
  dependre de la seva memòria.

## Com funciona (resum tècnic)

`SessionLog` ([src/Logger.h](../src/Logger.h)) substitueix el buffer de `std::cout` per un
`TeeBuf` que escriu cada caràcter a la consola i al fitxer alhora — per això cap `cout` del joc
ha hagut de canviar. Es crea al principi del `main` i el destructor restaura `cout` en sortir.

Limitacions conegudes (acceptades per disseny):

- **El que tecleja el jugador no queda gravat** (l'eco del teclat no passa per `cout`); les
  respostes es dedueixen de la línia següent del log.
- Si el joc mor amb un error fatal, el missatge del `catch` surt només per consola (el log ja
  s'ha tancat de manera segura abans).
- Si el fitxer no es pot obrir, el joc avisa i continua sense gravar — mai es perd una partida
  per culpa del registre.

## Git

Els `.log` estan ignorats (`*.log` al `.gitignore` de l'arrel): les partides són locals de cada
màquina. Aquest README sí que es versiona — i de pas fa que la carpeta existeixi als clons nous.
