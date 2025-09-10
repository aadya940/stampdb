from typing import List, Union
from .point import Point
from .csv import CSV
from . import _backend

import os


class StampDB:
    """Python wrapper for the StampDB C++ class.

    A time-series database that stores CSV-like data with efficient
    time-based indexing and CRUD operations.
    """

    def __init__(self, schema: dict, filename: str):
        """Initialize StampDB with a CSV file.

        Args:
            filename: Path to the CSV file to use as database storage.
        """
        self.filename = filename
        self.schema = schema

        self.headers = ["time"] + list(schema.keys())

        if not os.path.exists(filename):
            f = open(self.filename, "w")
            f.write(", ".join(self.headers))
            f.write("\n")
            f.close()

        self._db = _backend.StampDB(filename)

    def read(self, time: float) -> CSV:
        """Read data at a specific time.

        Args:
            time: The time point to read data from.

        Returns:
            CSV object containing the data at the specified time.
        """
        csv_data = self._db.read(time)
        points = [Point(p.time, [row.data for row in p.rows]) for p in csv_data.points]
        return CSV(csv_data.headers, points)

    def read_range(self, start_time: float, end_time: float) -> CSV:
        """Read data within a time range.

        Args:
            start_time: Start of the time range (inclusive).
            end_time: End of the time range (inclusive).

        Returns:
            CSV object containing all data points within the time range.
        """
        csv_data = self._db.read_range(start_time, end_time)
        points = [Point(p.time, [row.data for row in p.rows]) for p in csv_data.points]
        return CSV(csv_data.headers, points)

    def delete_point(self, time: float) -> CSV:
        """Delete a data point at the specified time.

        Args:
            time: The time point to delete.

        Returns:
            CSV object containing the deleted data (if any).
        """
        csv_data = self._db.delete_point(time)
        points = [Point(p.time, [row.data for row in p.rows]) for p in csv_data.points]
        return CSV(csv_data.headers, points)

    def append_point(self, point: Point) -> bool:
        """Append a new data point to the database.

        Args:
            point: Point object to append.

        Returns:
            True if the point was successfully appended.
        """
        return self._db.append_point(point.point)

    def compact(self) -> CSV:
        """Compact the database by removing deleted entries.

        Returns:
            CSV object containing all remaining data after compaction.
        """
        csv_data = self._db.compact()
        points = [Point(p.time, [row.data for row in p.rows]) for p in csv_data.points]
        return CSV(csv_data.headers, points)

    def checkpoint(self) -> bool:
        """Force a checkpoint operation.

        Returns:
            True if checkpoint was successful.
        """
        return self._db.checkpoint()

    def close(self):
        """Close the database connection."""
        self._db.close()

    @property
    def checkpoint_threshold(self) -> int:
        """Get the checkpoint threshold (number of operations before auto-compaction)."""
        return self._db.CHECKPOINT

    @checkpoint_threshold.setter
    def checkpoint_threshold(self, value: int):
        """Set the checkpoint threshold."""
        self._db.CHECKPOINT = value

    def __enter__(self):
        """Context manager entry."""
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        """Context manager exit - automatically close database."""
        self.close()

    def __repr__(self):
        return f"StampDB(filename='{self.filename}')"
