from stampdb import StampDB, Point  # Replace with your actual module name


def main():
    # Use the database

    schema = {
        "temp": "float",
        "humidity": "float",
    }

    with StampDB(schema, "data.csv") as db:

        # Read existing data
        db.append_point(Point(1.0, [23.5, 45.2]))
        db.append_point(Point(2.0, [24.1, 46.8]))
        db.append_point(Point(3.0, [25.7, 48.3]))

        db.checkpoint()  # Does Append Only writes. (Much faster than compact)

        data = db.read_range(0.0, 10.0)
        print(f"Found {len(data)} points")

        # Add new point
        new_point = Point(3.0, [25.7, 48.3])
        db.append_point(new_point)

        # Read specific time
        point_data = db.read(2.0)
        if len(point_data) > 0:
            temp, humidity = [row.data for row in point_data.csv.points[0].rows]
            print(f"Time 2.0: {temp}°C, {humidity}%")

        # Delete and check final count
        db.delete_point(1.0)
        final_data = db.read_range(0.0, 10.0)
        print(f"Final count: {len(final_data)} points")


if __name__ == "__main__":
    main()
