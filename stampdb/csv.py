from typing import List
from .point import Point

from . import _backend
from ._backend._types import CSVData


class CSV:
    """A CSV file with a list of points."""

    def __init__(self, headers: List[str], points: List[Point]):
        self.csv = _backend.CSVData()
        self.csv.headers = headers
        self.csv.points = [p.point for p in points]

    def __repr__(self):
        return f"CSV(headers={self.csv.headers}, points={self.csv.points[:min(5, len(self.csv.points))]}...)"

    def __str__(self):
        return self.__repr__()

    def __len__(self):
        return len(self.csv.points)

    def append_point(self, point: Point):
        self.csv.append_point(point.point)
