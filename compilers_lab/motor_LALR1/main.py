"""Ejecución de diez casos de prueba para el parser LALR(1) de apuntadores."""
import argparse
from dataclasses import dataclass

from parser import AnalizadorLALR, ErrorSintactico
from tokens import Token


@dataclass(frozen=True)
class CasoPrueba:
    nombre: str
    tokens: list[Token]
    valida: bool


# Casos válidos para S -> L = R | R; L -> *R | id; R -> L.
# 1. id = id
TOKENS_1 = [
    Token('id', 'x', 1, 1),
    Token('=', '=', 1, 3),
    Token('id', 'y', 1, 5),
]

# 2. id
TOKENS_2 = [
    Token('id', 'x', 1, 1),
]

# 3. *id
TOKENS_3 = [
    Token('*', '*', 1, 1),
    Token('id', 'x', 1, 2),
]

# 4. **id
TOKENS_4 = [
    Token('*', '*', 1, 1),
    Token('*', '*', 1, 2),
    Token('id', 'x', 1, 3),
]

# 5. **id = ***id
TOKENS_5 = [
    Token('*', '*', 1, 1),
    Token('*', '*', 1, 2),
    Token('id', 'x', 1, 3),
    Token('=', '=', 1, 5),
    Token('*', '*', 1, 7),
    Token('*', '*', 1, 8),
    Token('*', '*', 1, 9),
    Token('id', 'y', 1, 10),
]

# Casos inválidos.
# 6. Cadena vacía: la gramática no tiene una producción epsilon.
TOKENS_6: list[Token] = []

# 7. = id
TOKENS_7 = [
    Token('=', '=', 1, 1),
    Token('id', 'x', 1, 3),
]

# 8. id =
TOKENS_8 = [
    Token('id', 'x', 1, 1),
    Token('=', '=', 1, 3),
]

# 9. * = id
TOKENS_9 = [
    Token('*', '*', 1, 1),
    Token('=', '=', 1, 3),
    Token('id', 'x', 1, 5),
]

# 10. id id
TOKENS_10 = [
    Token('id', 'x', 1, 1),
    Token('id', 'y', 1, 3),
]


CASOS_PRUEBA = [
    CasoPrueba('id = id', TOKENS_1, True),
    CasoPrueba('id', TOKENS_2, True),
    CasoPrueba('* id', TOKENS_3, True),
    CasoPrueba('* * id', TOKENS_4, True),
    CasoPrueba('* * id = * * * id', TOKENS_5, True),
    CasoPrueba('cadena vacía', TOKENS_6, False),
    CasoPrueba('= id', TOKENS_7, False),
    CasoPrueba('id =', TOKENS_8, False),
    CasoPrueba('* = id', TOKENS_9, False),
    CasoPrueba('id id', TOKENS_10, False),
]


def describir_tokens(tokens: list[Token]) -> str:
    """Presenta la entrada en términos del tipo y el lexema de cada token."""
    contenido = ' '.join(f"{token.tipo}({token.lexema!r})" for token in tokens)
    return f'{contenido} $' if contenido else '$'


def ejecutar_caso(caso: CasoPrueba, numero: int, traza: bool = False) -> bool:
    print('\n' + '=' * 72)
    print(f'Caso {numero}: {caso.nombre}')
    print(f"Validez esperada: {'válida' if caso.valida else 'inválida'}\n")

    if traza:
        print('Tokens de entrada:')
        print(describir_tokens(caso.tokens))

    analizador = AnalizadorLALR(mostrar_traza=traza)

    try:
        resultado = analizador.analizar(caso.tokens)
    except ErrorSintactico as error:
        print(f'Error: {error}\n')
        print('Cadena inválida')
        return not caso.valida

    print(f'Reducciones realizadas: {len(resultado.reducciones)}\n')
    print('Cadena válida\n')
    print('AST (Abstract Syntax Tree) en modo texto:')
    print(resultado.ast.imprimir())
    return caso.valida


def main() -> int:
    interfaz = argparse.ArgumentParser(
        description='Motor de ejecución LALR(1) con tablas hardcoded y listas de tokens definidas manualmente.'
    )
    interfaz.add_argument(
        '--caso',
        type=int,
        choices=range(1, len(CASOS_PRUEBA) + 1),
        metavar='N',
        help='ejecuta solo el caso N (del 1 al 10); por defecto ejecuta los diez',
    )
    interfaz.add_argument(
        '--trace', '--traza', dest='trace', action='store_true',
        help='muestra pila, entrada restante y acciones shift/reduce',
    )
    args = interfaz.parse_args()

    if args.caso is None:
        seleccion = list(enumerate(CASOS_PRUEBA, start=1))
    else:
        seleccion = [(args.caso, CASOS_PRUEBA[args.caso - 1])]

    correctas = sum(
        ejecutar_caso(caso, numero, args.trace)
        for numero, caso in seleccion
    )

    print('\n' + '=' * 72)
    print(f'Resumen: {correctas}/{len(seleccion)} pruebas con el resultado esperado.')
    return 0 if correctas == len(seleccion) else 1


if __name__ == '__main__':
    raise SystemExit(main())
