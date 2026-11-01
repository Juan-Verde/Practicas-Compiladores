"""Tablas LALR(1) hardcoded de la Práctica 6.
- sN: shift al estado N.
- rN: reduce mediante la producción N.
- acc: acepta la entrada.
- Las celdas ausentes representan un error sintáctico.

Los estados fusionados se renumeran de forma compacta para que la pila
pueda seguir almacenando identificadores enteros:
    4 = I4 U I11
    5 = I5 U I12
    7 = I7 U I13
    8 = I8 U I10
Los estados 0, 1, 2, 3, 6 y 9 conservan sus identificadores originales.
"""

TERMINALES = ('=', '*', 'id', '$')
NO_TERMINALES = ('S', 'L', 'R')

# Correspondencia entre los estados LR(1) originales y los estados
# LALR(1) compactos. Los valores son los identificadores usados por la pila.
FUSIONES: dict[int, tuple[int, ...]] = {
    4: (4, 11),
    5: (5, 12),
    7: (7, 13),
    8: (8, 10),
}

ESTADOS_LALR: dict[int, tuple[int, ...]] = {
    0: (0,),
    1: (1,),
    2: (2,),
    3: (3,),
    4: (4, 11),
    5: (5, 12),
    6: (6,),
    7: (7, 13),
    8: (8, 10),
    9: (9,),
}

# ACCION[estado][terminal] -> sN, rN o acc.
ACCION: dict[int, dict[str, str]] = {
    0: {'*': 's4', 'id': 's5'},
    1: {'$': 'acc'},
    2: {'=': 's6', '$': 'r6'},
    3: {'$': 'r3'},
    4: {'*': 's4', 'id': 's5'},
    5: {'=': 'r5', '$': 'r5'},
    6: {'*': 's4', 'id': 's5'},
    7: {'=': 'r4', '$': 'r4'},
    8: {'=': 'r6', '$': 'r6'},
    9: {'$': 'r2'},
}

# GOTO[estado][no_terminal] -> estado LALR de destino.
GOTO: dict[int, dict[str, int]] = {
    0: {'S': 1, 'L': 2, 'R': 3},
    1: {},
    2: {},
    3: {},
    4: {'L': 8, 'R': 7},
    5: {},
    6: {'L': 8, 'R': 9},
    7: {},
    8: {},
    9: {},
}
