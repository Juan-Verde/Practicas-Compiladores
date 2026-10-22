"""Producciones y acciones semánticas para la gramática de la práctica.

La producción 0 es la gramática aumentada. Las tablas ACCION usan reducciones
r1 a r33; sus números coinciden con la numeración del desarrollo manual.
"""
from nodos_ast import NodoAST

# Producción n -> (lado izquierdo, lado derecho).
PRODUCCIONES: dict[int, tuple[str, tuple[str, ...]]] = {
    0: ("S'", ('Script',)),
    1: ('Script', ('Decls',)),
    2: ('Decls', ('Decls', 'Decl')),
    3: ('Decls', ('Decl',)),
    4: ('Decl', ('let', 'id', '=', 'Expr', ';')),
    5: ('Decl', ('function', 'id', '(', 'Params', ')', '{', 'Decls', '}')),
    6: ('Decl', ('if', '(', 'Expr', ')', '{', 'Decls', '}', 'else', '{', 'Decls', '}')),
    7: ('Decl', ('while', '(', 'Expr', ')', '{', 'Decls', '}')),
    8: ('Decl', ('id', '=', 'Expr', ';')),
    9: ('Decl', ('return', 'Expr', ';')),
    10: ('Decl', ('Expr', ';')),
    11: ('Params', ('Params', ',', 'id')),
    12: ('Params', ('id',)),
    13: ('Expr', ('Logic',)),
    14: ('Logic', ('Logic', '||', 'And')),
    15: ('Logic', ('And',)),
    16: ('And', ('And', '&&', 'Comp')),
    17: ('And', ('Comp',)),
    18: ('Comp', ('Arit', '==', 'Arit')),
    19: ('Comp', ('Arit', '<', 'Arit')),
    20: ('Comp', ('Arit',)),
    21: ('Arit', ('Arit', '+', 'Mult')),
    22: ('Arit', ('Arit', '-', 'Mult')),
    23: ('Arit', ('Mult',)),
    24: ('Mult', ('Mult', '*', 'App')),
    25: ('Mult', ('App',)),
    26: ('App', ('id', '(', 'Args', ')')),
    27: ('App', ('Unit',)),
    28: ('Args', ('Args', ',', 'Expr')),
    29: ('Args', ('Expr',)),
    30: ('Unit', ('id',)),
    31: ('Unit', ('num',)),
    32: ('Unit', ('str',)),
    33: ('Unit', ('(', 'Expr', ')')),
}


def _lexema(valor) -> str:
    """Extrae el lexema de un terminal almacenado como Token."""
    return valor.lexema


def _crear_identificador(token, tipo: str = 'Identifier') -> NodoAST:
    return NodoAST(tipo, _lexema(token))


def _crear_bloque(declaraciones: list[NodoAST]) -> NodoAST:
    return NodoAST('Block', hijos=list(declaraciones))


def construir_valor_reduccion(numero_regla: int, valores: list):
    """Construye nodos o listas al aplicar una reducción de la gramática."""
    if numero_regla == 1:  # Script -> Decls
        return NodoAST('Script', hijos=list(valores[0]), regla=numero_regla)
    if numero_regla == 2:  # Decls -> Decls Decl
        return list(valores[0]) + [valores[1]]
    if numero_regla == 3:  # Decls -> Decl
        return [valores[0]]
    if numero_regla == 4:  # Decl -> let id = Expr ;
        return NodoAST('VariableDeclaration', f"let {_lexema(valores[1])}", [valores[3]], numero_regla)
    if numero_regla == 5:  # Decl -> function id ( Params ) { Decls }
        parametros = list(valores[3])
        cuerpo = _crear_bloque(valores[6])
        return NodoAST('FunctionDeclaration', _lexema(valores[1]), parametros + [cuerpo], numero_regla)
    if numero_regla == 6:  # Decl -> if ( Expr ) { Decls } else { Decls }
        return NodoAST('IfElseStatement', hijos=[valores[2], _crear_bloque(valores[5]), _crear_bloque(valores[9])], regla=numero_regla)
    if numero_regla == 7:  # Decl -> while ( Expr ) { Decls }
        return NodoAST('While', hijos=[valores[2], _crear_bloque(valores[5])], regla=numero_regla)
    if numero_regla == 8:  # Decl -> id = Expr ;
        return NodoAST('Assignment', _lexema(valores[0]), [valores[2]], numero_regla)
    if numero_regla == 9:  # Decl -> return Expr ;
        return NodoAST('Return', hijos=[valores[1]], regla=numero_regla)
    if numero_regla == 10:  # Decl -> Expr ;
        return NodoAST('ExpressionStatement', hijos=[valores[0]], regla=numero_regla)
    if numero_regla == 11:  # Params -> Params , id
        return list(valores[0]) + [_crear_identificador(valores[2], 'Parameter')]
    if numero_regla == 12:  # Params -> id
        return [_crear_identificador(valores[0], 'Parameter')]
    if numero_regla == 13:  # Expr -> Logic
        return valores[0]
    if numero_regla == 14:  # Logic -> Logic || And
        return NodoAST('BinaryOperation', '||', [valores[0], valores[2]], numero_regla)
    if numero_regla == 15:  # Logic -> And
        return valores[0]
    if numero_regla == 16:  # And -> And && Comp
        return NodoAST('BinaryOperation', '&&', [valores[0], valores[2]], numero_regla)
    if numero_regla == 17:  # And -> Comp
        return valores[0]
    if numero_regla == 18:  # Comp -> Arit == Arit
        return NodoAST('BinaryOperation', '==', [valores[0], valores[2]], numero_regla)
    if numero_regla == 19:  # Comp -> Arit < Arit
        return NodoAST('BinaryOperation', '<', [valores[0], valores[2]], numero_regla)
    if numero_regla == 20:  # Comp -> Arit
        return valores[0]
    if numero_regla == 21:  # Arit -> Arit + Mult
        return NodoAST('BinaryOperation', '+', [valores[0], valores[2]], numero_regla)
    if numero_regla == 22:  # Arit -> Arit - Mult
        return NodoAST('BinaryOperation', '-', [valores[0], valores[2]], numero_regla)
    if numero_regla == 23:  # Arit -> Mult
        return valores[0]
    if numero_regla == 24:  # Mult -> Mult * App
        return NodoAST('BinaryOperation', '*', [valores[0], valores[2]], numero_regla)
    if numero_regla == 25:  # Mult -> App
        return valores[0]
    if numero_regla == 26:  # App -> id ( Args )
        return NodoAST('FunctionCall', _lexema(valores[0]), list(valores[2]), numero_regla)
    if numero_regla == 27:  # App -> Unit
        return valores[0]
    if numero_regla == 28:  # Args -> Args , Expr
        return list(valores[0]) + [valores[2]]
    if numero_regla == 29:  # Args -> Expr
        return [valores[0]]
    if numero_regla == 30:  # Unit -> id
        return _crear_identificador(valores[0])
    if numero_regla == 31:  # Unit -> num
        return NodoAST('Number', _lexema(valores[0]), regla=numero_regla)
    if numero_regla == 32:  # Unit -> str
        return NodoAST('String', _lexema(valores[0]), regla=numero_regla)
    if numero_regla == 33:  # Unit -> ( Expr )
        return valores[1]
    raise ValueError(f"No hay acción semántica para la producción r{numero_regla}.")
