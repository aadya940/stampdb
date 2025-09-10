from stampdb import StampDB, Point  # Replace with your actual module name

def main():
    # Use the database

    schema = {
        "temp": "float",
        "humidity": "float",
    }

    with StampDB(schema, "data.csv") as db:

        db.append_point(Point(1.0, [23.5, 45.2]))
        db.append_point(Point(2.0, [24.1, 46.8]))
        db.append_point(Point(3.0, [25.7, 48.3]))

        db.checkpoint()

        data = db.read_range(0.0, 10.0)
        print(data[data['time'] == 2.0])

        new_point = Point(3.0, [25.7, 48.3])
        db.append_point(new_point)

        point_data = db.read(2.0)
        print("Point data:", point_data)

        db.delete_point(1.0)
        final_data = db.read_range(0.0, 10.0)
        print(f"Final count: {len(final_data)} points")


if __name__ == "__main__":
    main()
