"""Nodos y presentación en texto del AST construido durante las reducciones."""
from dataclasses import dataclass, field


@dataclass
class NodoAST:
    """Nodo del árbol sintáctico abstracto (AST)."""

    tipo: str
    valor: str | None = None
    hijos: list['NodoAST'] = field(default_factory=list)
    regla: int | None = None

    def etiqueta(self) -> str:
        if self.valor is not None:
            return f"{self.tipo} ({self.valor})"
        return self.tipo

    def imprimir(self, nivel: int = 0) -> str:
        """Devuelve el árbol en una representación jerárquica de texto."""
        lineas = [f"{'  ' * nivel}{self.etiqueta()}"]
        for hijo in self.hijos:
            lineas.append(hijo.imprimir(nivel + 1))
        return '\n'.join(lineas)

    def ast_diccionario(self) -> dict:
        """Devuelve una versión serializable del AST."""
        resultado = {'tipo': self.tipo}
        if self.valor is not None:
            resultado['valor'] = self.valor
        resultado['hijos'] = [hijo.ast_diccionario() for hijo in self.hijos]
        if self.regla is not None:
            resultado['regla'] = self.regla
        return resultado
