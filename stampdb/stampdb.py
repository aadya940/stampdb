from .point import Point
from . import _backend

import numpy as np
import os
from datetime import datetime, timezone
from typing import Union, Tuple

from .schema import SchemaValidation


class StampDB:
    """Python wrapper for the StampDB C++ class.

    A time-series database that stores CSV-like data with efficient
    time-based indexing and CRUD operations.
    """

    def __init__(self, filename: str, schema: dict = None):
        """Initialize StampDB with a CSV file.

        Args:
            filename: str
                Path to the CSV file to use as database storage.
            schema: Optional[dict]
                Optional dictionary mapping column names to data types.
        """
        self.filename = filename
        self.schema = list(schema.values())

        self.schema_file = filename + ".schema"

        if os.path.exists(self.schema_file):
            self.schema = SchemaValidation(schema=None, filename=self.schema_file)
        else:
            self.schema = SchemaValidation(
                schema=self.schema, filename=self.schema_file
            )

        self.headers = ["time"] + list(schema.keys())

        if not os.path.exists(filename):
            if schema is None:
                raise ValueError(
                    "Schema must be provided if Database File does not exist."
                )

            f = open(self.filename, "w")
            f.write(", ".join(self.headers))
            f.write("\n")
            f.close()

        self._db = _backend.StampDB(filename)

    def _convert_to_timestamp(self, time: Union[float, datetime]) -> float:
        """Convert datetime object to timestamp if needed.

        Args:
            time: Either a float timestamp or datetime object

        Returns:
            float: Timestamp in seconds since epoch
        """
        if isinstance(time, datetime):
            if time.tzinfo is None:
                time = time.replace(tzinfo=timezone.utc)
            return time.timestamp()
        return time

    def read(self, time: Union[float, datetime]) -> np.ndarray:
        """Read data at a specific time.

        Args:
            time: Union[float, datetime]
                The time point to read data from. Can be a Unix timestamp (float) or datetime object.

        Returns:
            NumPy structured array containing the data at the specified time.
        """
        timestamp = self._convert_to_timestamp(time)
        csv_data = self._db.read(timestamp)

        csv_data.headers = [h.strip() for h in csv_data.headers if h.strip()]

        return self._db.as_numpy_structured_array(csv_data)

    def read_range(
        self, start_time: Union[float, datetime], end_time: Union[float, datetime]
    ) -> np.ndarray:
        """Read data within a time range.

        Args:
            start_time: Union[float, datetime]
                Start of the time range (inclusive). Can be Unix timestamp or datetime object.
            end_time: Union[float, datetime]
                End of the time range (inclusive). Can be Unix timestamp or datetime object.

        Returns:
            NumPy structured array containing all data points within the time range.
        """
        start = self._convert_to_timestamp(start_time)
        end = self._convert_to_timestamp(end_time)
        csv_data = self._db.read_range(start, end)

        csv_data.headers = [h.strip() for h in csv_data.headers if h.strip()]

        return self._db.as_numpy_structured_array(csv_data)

    def delete_point(self, time: Union[float, datetime]) -> np.ndarray:
        """Delete a data point at the specified time.

        Args:
            time: Union[float, datetime]
                The time point to delete. Can be Unix timestamp or datetime object.

        Returns:
            NumPy structured array containing the deleted data (if any).
        """
        timestamp = self._convert_to_timestamp(time)
        csv_data = self._db.delete_point(timestamp)

        csv_data.headers = [h.strip() for h in csv_data.headers if h.strip()]

        return self._db.as_numpy_structured_array(csv_data)

    def append_point(self, point: Point) -> bool:
        """Append a new data point to the database.

        Args:
            point: Point
                Point object to append.

        Returns:
            True if the point was successfully appended.
        """
        self.schema.validate(point)
        return self._db.append_point(point.point)

    def update_point(self, point: Point) -> bool:
        """Update an existing data point in the database.

        Args:
            point: Point
                Point object to update.

        Returns:
            True if the point was successfully updated.
        """
        self.schema.validate(point)
        return self._db.update_point(point.point)

    def compact(self) -> np.ndarray:
        """Compact the database by removing deleted entries.

        Returns:
            NumPy structured array containing all remaining data after compaction.
        """
        csv_data = self._db.compact()
        csv_data.headers = [h.strip() for h in csv_data.headers if h.strip()]
        return self._db.as_numpy_structured_array(csv_data)

    def get_timestamps(self) -> Tuple[datetime, datetime]:
        """Get the first and last timestamps in the database.

        Returns:
            Tuple[datetime, datetime]: (first_timestamp, last_timestamp) as datetime objects

        Raises:
            ValueError: If the database is empty
        """
        # Read a small range to check if database has any data
        data = self.read_range(0, float("inf"))
        if len(data) == 0:
            raise ValueError("Database is empty")

        # Get the first and last timestamps
        first_ts = data[0]["time"]
        last_ts = data[-1]["time"]

        # Convert timestamps to datetime objects
        return (
            datetime.fromtimestamp(first_ts, tz=timezone.utc),
            datetime.fromtimestamp(last_ts, tz=timezone.utc),
        )

    def checkpoint(self) -> bool:
        """Force a checkpoint operation.

        Returns:
            True if checkpoint was successful.
        """
        return self._db.checkpoint()

    def close(self):
        """Close the database connection."""
        if not os.path.exists(self.schema_file):
            self.schema._save_schema_to_file()
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
