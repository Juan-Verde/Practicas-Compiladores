"""Producciones y acciones semanticas de la gramatica para LALR(1).

La produccion (1), S' -> S, es la produccion aumentada y se utiliza para
la aceptacion. Las reducciones de la tabla estan numeradas de r2 a r6,
como en el desarrollo manual de la Practica 6.
"""
from nodos_ast import NodoAST

# Produccion n -> (lado izquierdo, lado derecho)
# La produccion aumentada (1) no se reduce: su item completo produce `acc`
PRODUCCIONES: dict[int, tuple[str, tuple[str, ...]]] = {
    2: ('S', ('L', '=', 'R')),
    3: ('S', ('R',)),
    4: ('L', ('*', 'R')),
    5: ('L', ('id',)),
    6: ('R', ('L',)),
}


def _lexema(valor) -> str:
    """Extrae el lexema de un terminal almacenado como Token."""
    return valor.lexema


def construir_valor_reduccion(numero_regla: int, valores: list):
    """Construye el nodo AST correspondiente a una reduccion LALR(1).

    Convencion semantica:
    - S -> L = R: asignacion con lado izquierdo y derecho.
    - S -> R: expresion independiente.
    - L -> *R: desreferencia.
    - L -> id: identificador.
    - R -> L: conserva el nodo de L, sin agregar un nodo redundante.
    """
    if numero_regla == 2:  # S -> L = R
        return NodoAST('Assignment', hijos=[valores[0], valores[2]], regla=numero_regla)
    if numero_regla == 3:  # S -> R
        return NodoAST('ExpressionStatement', hijos=[valores[0]], regla=numero_regla)
    if numero_regla == 4:  # L -> * R
        return NodoAST('Dereference', hijos=[valores[1]], regla=numero_regla)
    if numero_regla == 5:  # L -> id
        return NodoAST('Identifier', _lexema(valores[0]), regla=numero_regla)
    if numero_regla == 6:  # R -> L
        return valores[0]
    raise ValueError(f"No hay accion semantica para la produccion r{numero_regla}.")
