# Probando el analizador con 10 casos de prueba definidos directamente como listas de tokens,
# ya que el ejercicio lo pide asi.
import argparse
from dataclasses import dataclass

from parser import AnalizadorSLR, ErrorSintactico
from tokens import Token


@dataclass(frozen=True)
class CasoPrueba:
    nombre: str
    tokens: list[Token]
    valida: bool

# Cadenas validas

# let x = 5;
tokens1 = [
    Token("let", "let", 1, 1),
    Token("id", "x", 1, 5),
    Token("=", "=", 1, 7),
    Token("num", "5", 1, 9),
    Token(";", ";", 1, 10),
]

# let y = 10;
tokens2 = [
    Token("let", "let", 1, 1),
    Token("id", "y", 1, 5),
    Token("=", "=", 1, 7),
    Token("num", "10", 1, 9),
    Token(";", ";", 1, 11),
]

# x = 1 + 2 * 3;
tokens3 = [
    Token("id", "x", 1, 1),
    Token("=", "=", 1, 3),
    Token("num", "1", 1, 5),
    Token("+", "+", 1, 7),
    Token("num", "2", 1, 9),
    Token("*", "*", 1, 11),
    Token("num", "3", 1, 13),
    Token(";", ";", 1, 14),
]

# function suma(a, b) { return a + b; }
tokens4 = [
    Token("function", "function", 1, 1),
    Token("id", "suma", 1, 10),
    Token("(", "(", 1, 14),
    Token("id", "a", 1, 15),
    Token(",", ",", 1, 16),
    Token("id", "b", 1, 17),
    Token(")", ")", 1, 18),
    Token("{", "{", 1, 20),
    Token("return", "return", 1, 22),
    Token("id", "a", 1, 29),
    Token("+", "+", 1, 31),
    Token("id", "b", 1, 33),
    Token(";", ";", 1, 34),
    Token("}", "}", 1, 36),
]

# if (x < 10) { x = x + 1; } else { x = 0; }
tokens5 = [
    Token("if", "if", 1, 1),
    Token("(", "(", 1, 4),
    Token("id", "x", 1, 5),
    Token("<", "<", 1, 7),
    Token("num", "10", 1, 9),
    Token(")", ")", 1, 11),
    Token("{", "{", 1, 13),
    Token("id", "x", 1, 15),
    Token("=", "=", 1, 17),
    Token("id", "x", 1, 19),
    Token("+", "+", 1, 21),
    Token("num", "1", 1, 23),
    Token(";", ";", 1, 24),
    Token("}", "}", 1, 26),
    Token("else", "else", 1, 28),
    Token("{", "{", 1, 33),
    Token("id", "x", 1, 35),
    Token("=", "=", 1, 37),
    Token("num", "0", 1, 39),
    Token(";", ";", 1, 40),
    Token("}", "}", 1, 42),
]

# Cadenas invalidas

# let x = ;
tokens6 = [
    Token("let", "let", 1, 1),
    Token("id", "x", 1, 5),
    Token("=", "=", 1, 7),
    Token(";", ";", 1, 8),
]

# let = 5;
tokens7 = [
    Token("let", "let", 1, 1),
    Token("=", "=", 1, 5),
    Token("num", "5", 1, 7),
    Token(";", ";", 1, 8),
]

# iniciar();
tokens8 = [
    Token("id", "iniciar", 1, 1),
    Token("(", "(", 1, 8),
    Token(")", ")", 1, 9),
    Token(";", ";", 1, 10),
]

# if (x < 2) { x = 3; }
tokens9 = [
    Token("if", "if", 1, 1),
    Token("(", "(", 1, 4),
    Token("id", "x", 1, 5),
    Token("<", "<", 1, 7),
    Token("num", "2", 1, 9),
    Token(")", ")", 1, 10),
    Token("{", "{", 1, 12),
    Token("id", "x", 1, 14),
    Token("=", "=", 1, 16),
    Token("num", "3", 1, 18),
    Token(";", ";", 1, 19),
    Token("}", "}", 1, 21),
]

# function inicio() { return 1; }
tokens10 = [
    Token("function", "function", 1, 1),
    Token("id", "inicio", 1, 10),
    Token("(", "(", 1, 16),
    Token(")", ")", 1, 17),
    Token("{", "{", 1, 19),
    Token("return", "return", 1, 21),
    Token("num", "1", 1, 28),
    Token(";", ";", 1, 29),
    Token("}", "}", 1, 31),
]

# Agrupacion de los casos.
CASOS_PRUEBA = [
    CasoPrueba("let x = 5", tokens1, True),
    CasoPrueba("let y = 10;", tokens2, True),
    CasoPrueba("x = 1 + 2 * 3;", tokens3, True),
    CasoPrueba("function suma(a, b) { return a + b; }", tokens4, True),
    CasoPrueba("if (x < 10) { x = x + 1; } else { x = 0; }", tokens5, True),
    CasoPrueba("let x = ;", tokens6, False),
    CasoPrueba("let = 5;", tokens7, False),
    CasoPrueba("iniciar()", tokens8, False),
    CasoPrueba("if (x < 2) { x = 3; }", tokens9, False),
    CasoPrueba("function inicio() { return 1; }", tokens10, False),
]


def describir_tokens(tokens: list[Token]) -> str:
    """Presenta la entrada en terminos del tipo y el lexema de cada token."""
    return " ".join(f"{token.tipo}({token.lexema!r})" for token in tokens) + " $"


def ejecutar_caso(caso: CasoPrueba, numero: int, traza: bool = False) -> bool:
    print("\n" + "=" * 72)
    print(f"Caso {numero}: {caso.nombre}")
    print(f"Validez esperada: {'valida\n' if caso.valida else 'invalida\n'}")

    if traza:
        print("Tokens de entrada:")
        print(describir_tokens(caso.tokens))

    analizador = AnalizadorSLR(mostrar_traza=traza)

    try:
        resultado = analizador.analizar(caso.tokens)
    except ErrorSintactico as error:
        print(f"Error: {error}\n")
        print("Cadena invalida")
        return False
    
    print(f"Reducciones realizadas: {len(resultado.reducciones)}\n")
    print("Cadena valida\n")
    print("AST (Abstract Syntax Tree) en modo texto:")
    print(resultado.ast.imprimir())

    return True

def main() -> int:
    interfaz = argparse.ArgumentParser(
        description="Motor de ejecucion SLR(1) con listas de tokens definidas manualmente."
    )

    interfaz.add_argument(
        "--caso",
        type=int,
        choices=range(1, len(CASOS_PRUEBA) + 1),
        metavar="N",
        help="ejecuta solo el caso N (del 1 al 10); por defecto ejecuta los diez",
    )

    interfaz.add_argument(
        "--trace",
        "--traza",
        dest="trace",
        action="store_true",
        help="muestra pila, entrada restante y acciones shift/reduce",
    )

    args = interfaz.parse_args()

    if args.caso is None:
        seleccion = list(enumerate(CASOS_PRUEBA, start=1))
    else:
        seleccion = [(args.caso, CASOS_PRUEBA[args.caso - 1])]

    correctas = 0

    for numero, caso in seleccion:
        if ejecutar_caso(caso, numero, args.trace):
            correctas += 1

    print("\n" + "=" * 72)
    print(f"Resumen: {correctas}/{len(seleccion)} pruebas con el resultado esperado.")

    return 0 if correctas == len(seleccion) else 1

if __name__ == "__main__":
    raise SystemExit(main())
