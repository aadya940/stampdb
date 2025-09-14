import numpy as np
from numpy.lib import recfunctions as rfn


class _BaseJoin:
    def __init__(
        self, left: np.ndarray, right: np.ndarray, left_key: str, right_key: str
    ):
        self.left = left
        self.right = right
        self.left_key = left_key
        self.right_key = right_key

        if left_key != right_key:
            # join_by requires matching field names
            self.right = rfn.rename_fields(self.right, {right_key: left_key})
            self.right_key = left_key

    def _join(self, jointype: str):
        return rfn.join_by(
            key=self.left_key,
            r1=self.left,
            r2=self.right,
            jointype=jointype,
            usemask=False,
            asrecarray=False,
        )


class InnerJoin(_BaseJoin):
    """Inner Join"""

    def do(self):
        return self._join("inner")


class OuterJoin(_BaseJoin):
    """Outer Join"""

    def do(self):
        return self._join("outer")


class LeftOuterJoin(_BaseJoin):
    """Left Outer Join"""

    def do(self):
        return self._join("leftouter")
