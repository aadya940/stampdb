from stampdb import StampDB, Point
from stampdb.relational import Summation, Projection, Selection

import numpy as np


def main():
    # Use the database
    schema = {
        "temp": "float",
        "humidity": "float",
    }

    with StampDB("data.csv", schema) as db:
        points = [
            Point(1.0, [23.5, 45.2]),
            Point(2.0, [24.1, 46.8]),
            Point(3.0, [25.7, 48.3])
        ]
        for point in points:
            db.append_point(point)

        db.checkpoint()
        
        data = db.read_range(0.0, 10.0)
        print(data)

        new_point = Point(4.0, [25.7, 48.3])
        db.append_point(new_point)

        point_data = db.read(2.0)
        print("Point data:", point_data)

        db.delete_point(1.0)
        final_data = db.read_range(0.0, 10.0)
        print(f"Final count: {len(final_data)} points")

        print("Column Names: \n")
        print(final_data.dtype.names)


if __name__ == "__main__":
    main()
