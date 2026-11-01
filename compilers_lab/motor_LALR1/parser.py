"""Motor de ejecución del analizador sintáctico LALR(1).

Interpreta las tablas ACCION y GOTO hardcoded en tablas_lalr.py
"""
from dataclasses import dataclass
from typing import Any

from gramatica import PRODUCCIONES, construir_valor_reduccion
from nodos_ast import NodoAST
from tablas_lalr import ACCION, GOTO
from tokens import Token


class ErrorSintactico(ValueError):
    """Error sintáctico con estado y ubicación del token inesperado."""

    def __init__(self, mensaje: str, token: Token, estado: int, esperados: list[str]):
        self.token = token
        self.estado = estado
        self.esperados = esperados
        texto_esperados = ', '.join(esperados) if esperados else '(ningún terminal)'
        super().__init__(
            f"{mensaje} en línea {token.linea}, columna {token.columna}: "
            f"se encontró {token.lexema!r} ({token.tipo}), estado {estado}. "
            f"Terminales esperados: {texto_esperados}."
        )


@dataclass
class ResultadoAnalisis:
    aceptada: bool
    ast: NodoAST
    reducciones: list[int]
    pasos: int


class AnalizadorLALR:
    """Analizador ascendente LALR(1) que interpreta ACCION y GOTO hardcoded."""

    def __init__(self, mostrar_traza: bool = False):
        self.mostrar_traza = mostrar_traza

    def analizar(self, tokens: list[Token]) -> ResultadoAnalisis:
        if not isinstance(tokens, list) or any(not isinstance(token, Token) for token in tokens):
            raise TypeError('analizar() espera una lista de objetos Token construidos previamente.')

        if tokens and tokens[-1].tipo == '$':
            tokens_entrada = list(tokens)
        else:
            if tokens:
                ultimo_token = tokens[-1]
                fin_entrada = Token(
                    '$', '$', ultimo_token.linea,
                    ultimo_token.columna + len(ultimo_token.lexema)
                )
            else:
                fin_entrada = Token('$', '$', 1, 1)
            tokens_entrada = list(tokens) + [fin_entrada]

        # La pila de estados utiliza los identificadores LALR compactos 0..9.
        pila_estados: list[int] = [0]
        # Cada elemento guarda (símbolo gramatical, valor semántico).
        pila_simbolos: list[tuple[str, Any]] = []
        reducciones: list[int] = []
        indice_entrada = 0
        pasos = 0
        max_pasos = max(10000, len(tokens_entrada) * 1000)

        while pasos < max_pasos:
            pasos += 1
            estado_actual = pila_estados[-1]
            token_anticipado = tokens_entrada[indice_entrada]
            accion = ACCION.get(estado_actual, {}).get(token_anticipado.tipo)

            if accion is None:
                esperados = sorted(
                    terminal for terminal in ACCION.get(estado_actual, {}) if terminal != '$'
                )
                raise ErrorSintactico('Error sintáctico', token_anticipado, estado_actual, esperados)

            if self.mostrar_traza:
                estados = ' '.join(map(str, pila_estados))
                simbolos = ' '.join(simbolo for simbolo, _ in pila_simbolos)
                entrada_restante = ' '.join(
                    token.lexema for token in tokens_entrada[indice_entrada:]
                )
                print(
                    f"[{pasos:04}] estados=[{estados}] símbolos=[{simbolos}] "
                    f"entrada=[{entrada_restante}] acción={accion}"
                )

            if accion == 'acc':
                if indice_entrada != len(tokens_entrada) - 1 or token_anticipado.tipo != '$':
                    raise ErrorSintactico(
                        'Aceptación inesperada antes del fin de entrada',
                        token_anticipado, estado_actual, ['$']
                    )
                if not pila_simbolos or not isinstance(pila_simbolos[-1][1], NodoAST):
                    raise RuntimeError('La tabla aceptó la entrada, pero no se construyó un AST raíz.')
                return ResultadoAnalisis(True, pila_simbolos[-1][1], reducciones, pasos)

            if accion.startswith('s'):
                estado_destino = int(accion[1:])
                pila_simbolos.append((token_anticipado.tipo, token_anticipado))
                pila_estados.append(estado_destino)
                indice_entrada += 1
                continue

            if accion.startswith('r'):
                numero_regla = int(accion[1:])
                lado_izquierdo, lado_derecho = PRODUCCIONES[numero_regla]
                cantidad_simbolos = len(lado_derecho)

                if cantidad_simbolos:
                    if (
                        len(pila_simbolos) < cantidad_simbolos
                        or len(pila_estados) <= cantidad_simbolos
                    ):
                        raise RuntimeError(f'Pila insuficiente al aplicar r{numero_regla}.')
                    simbolos_extraidos = pila_simbolos[-cantidad_simbolos:]
                    del pila_simbolos[-cantidad_simbolos:]
                    del pila_estados[-cantidad_simbolos:]
                    valores = [valor for _, valor in simbolos_extraidos]
                else:
                    valores = []

                valor_semantico = construir_valor_reduccion(numero_regla, valores)
                estado_anterior = pila_estados[-1]
                estado_destino = GOTO.get(estado_anterior, {}).get(lado_izquierdo)
                if estado_destino is None:
                    raise RuntimeError(
                        f'Falta GOTO[{estado_anterior}][{lado_izquierdo!r}] '
                        f'tras aplicar r{numero_regla}. Revisa las tablas LALR(1).'
                    )

                pila_simbolos.append((lado_izquierdo, valor_semantico))
                pila_estados.append(estado_destino)
                reducciones.append(numero_regla)

                if self.mostrar_traza:
                    produccion = ' '.join(lado_derecho) if lado_derecho else 'ε'
                    print(
                        f"      reducir r{numero_regla}: {lado_izquierdo} -> {produccion} | "
                        f"GOTO[{estado_anterior}, {lado_izquierdo}] = {estado_destino}"
                    )
                continue

            raise RuntimeError(f'Acción desconocida {accion!r} en ACCION[{estado_actual}].')

        raise RuntimeError(
            f'El análisis excedió el límite de {max_pasos} pasos, revisa las tablas.'
        )
