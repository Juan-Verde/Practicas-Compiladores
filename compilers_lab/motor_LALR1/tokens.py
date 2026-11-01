"""Modelo de los tokens que recibe el analizador sintáctico LALR(1)."""
from dataclasses import dataclass


@dataclass(frozen=True)
class Token:
    tipo: str
    lexema: str
    linea: int
    columna: int

    def __str__(self) -> str:
        return f"{self.tipo}({self.lexema!r}) en {self.linea}:{self.columna}"
