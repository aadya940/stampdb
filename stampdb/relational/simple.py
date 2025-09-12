"""Implementation of simple relational operations like Selection, Projection, Summation, etc."""

import numpy as np
from typing import List


class Selection:
    """Selects rows based on a condition."""

    def __init__(self, condition: str, data: np.ndarray):
        self.condition = condition
        self.data = data

    def do(self) -> np.ndarray:
        try:
            local_vars = {"data": self.data, "np": np}
            for name in self.data.dtype.names:
                local_vars[name] = self.data[name]
            mask = eval(self.condition, {"__builtins__": None}, local_vars)
            return self.data[mask]
        except Exception as e:
            raise ValueError(f"Error evaluating condition '{self.condition}': {str(e)}")


class Projection:
    """Selects specific columns from the data."""

    def __init__(self, columns: List[str], data: np.ndarray):
        self.columns = columns
        self.data = data

    def do(self) -> np.ndarray:
        return self.data[self.columns]


class Summation:
    """Calculates the sum of a column."""

    def __init__(self, column: str, data: np.ndarray):
        self.column = column
        self.data = data

    def do(self) -> float:
        return np.sum(self.data[self.column])


class OrderBy:
    """Sorts the data by specified columns using numpy's sort."""

    def __init__(self, columns: List[str], data: np.ndarray):
        self.columns = columns
        self.data = data

    def do(self) -> np.ndarray:
        if not self.columns:
            return self.data

        return np.sort(self.data, order=self.columns)
