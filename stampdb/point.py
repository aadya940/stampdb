from . import _backend
from ._backend._types import _Point

from typing import List, Union


class Point:
    """A point in time with a list of values.
    To be used as an append primitive in `StampDB`.
    """

    def __init__(self, time: float, data: List[Union[int, float, str, bool]]):
        """Initialize a point in time with a list of values.

        Args:
            time: float
                The time of the point.
            data: List[Union[int, float, str, bool]]
                The list of values.
        """
        self.data = data

        self.point = _Point()
        self.point.time = time

        for value in data:
            p = _backend.PointRow()
            p.data = value
            self.point.add_row(
                p
            )  # Add row is used to append values inside a timepoint list.

    def __repr__(self):
        """Return a string representation of the point."""
        return f"Point(time={self.point.time}, data={self.data})"

    def __len__(self):
        """Return the number of values in the point."""
        return len(self.data)

    def __str__(self):
        """Return the string representation of the point."""
        return self.__repr__()
