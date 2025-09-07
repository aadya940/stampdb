from . import _backend

from typing import List, Union

class Point:
    def __init__(self, time: float, data: List[Union[int, float, str, bool]]):
        self.data = data

        self.point = _backend._Point()
        self.point.time = time

        for value in data:
            p = _backend.PointRow()
            p.data = value
            self.point.add_row(p)
    
    def __repr__(self):
        return f"Point(time={self.point.time}, data={self.data})"
    
    def __len__(self):
        return len(self.data)
    
    def __str__(self):
        return self.__repr__()
