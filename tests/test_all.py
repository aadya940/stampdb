import sys
import os
import random
import numpy as np
import json
import tempfile
import pytest

sys.path.append("../")

from stampdb import StampDB, Point
from stampdb.schema import SchemaValidation
from stampdb.relational import (
    Summation,
    Projection,
    Selection,
    OrderBy,
    InnerJoin,
    OuterJoin,
    LeftOuterJoin,
)

# Before running, make sure no "test.csv" file exists in the current directory.


def test_db():
    db = StampDB("test.csv", schema={"temp": "float", "humidity": "string"})
    assert os.path.exists("test.csv") and os.path.getsize("test.csv") > 0
    assert os.path.exists("test.csv.tmp") and os.path.getsize("test.csv.tmp") > 0

    out = db.read_range(0, 10)
    assert out.size == 0

    db.close()

    assert not os.path.exists("test.csv.tmp")
    os.remove("test.csv")


def test_points():
    db = StampDB("test.csv", schema={"temp": "float", "humidity": "string"})

    # Test with numeric data only for now
    p1 = Point(time=0, data=[23.5, "moderate"])  # Using float instead of string
    db.append_point(p1)

    out = db.read_range(0, 10)
    assert out.size == 1
    assert out["temp"][0] == 23.5

    # Check that we can read the humidity value
    # Note: String comparison is disabled due to C++ backend issues
    humidity_value = out["humidity"][0]
    assert humidity_value == "moderate"

    # Check that all expected columns are present
    assert "time" in out.dtype.names
    assert "temp" in out.dtype.names
    assert "humidity" in out.dtype.names

    db.close()
    os.remove("test.csv")


def test_checkpoint():
    db = StampDB("test.csv", schema={"temp": "float", "humidity": "string"})

    db.checkpoint_threshold = 1
    p1 = Point(time=0, data=[23.5, "moderate"])
    db.append_point(p1)

    assert os.path.exists("test.csv") and os.path.getsize("test.csv") > len(
        "temp" + "humidity" + "time" + ",,\n"
    )

    db.close()
    os.remove("test.csv")


def test_delete():
    db = StampDB("test.csv", schema={"temp": "float", "humidity": "string"})
    p1 = Point(time=0, data=[23.5, "moderate"])

    db.append_point(p1)
    db.compact()

    out = db.read_range(0, 10)
    assert out.size == 1

    db.delete_point(0)
    db.compact()

    out = db.read_range(0, 10)
    assert out.size == 0

    db.close()


def test_relational():
    db = StampDB("test.csv", schema={"temp": "float", "humidity": "string"})
    p1 = Point(time=0, data=[23.5, "moderate"])
    p2 = Point(time=1, data=[24.5, "high"])
    db.append_point(p1)
    db.append_point(p2)
    db.compact()

    out = db.read_range(0, 10)
    assert isinstance(out, np.ndarray)

    s = Selection("temp > 24", out)
    assert s.do().size == 1

    p = Projection(["temp"], out)
    assert p.do().size == 2

    plus = Summation("temp", out)
    assert plus.do() == 48

    orderby = OrderBy(["temp"], out)
    assert orderby.do().size == 2
    assert orderby.do()["temp"][0] == 23.5

    db.close()
    os.remove("test.csv")


def test_joins():
    db = StampDB("test.csv", schema={"temp": "float", "humidity": "string"})
    for i in range(100):
        time = i
        temp = random.randint(0, 50)
        humidity = random.choice(["low", "moderate", "high"])
        p = Point(time=time, data=[temp, humidity])
        db.append_point(p)

    # Written to disk.
    db.compact()

    db2 = StampDB("test2.csv", schema={"weather": "string", "temp": "float"})
    for i in range(100):
        time = i
        temp = random.randint(0, 50)
        weather = random.choice(["sunny", "rainy", "cloudy"])
        p = Point(time=time, data=[weather, temp])
        db2.append_point(p)

    # Written to disk.
    db2.compact()

    data = db.read_range(0, 100)
    assert data.size == 100

    ij = InnerJoin(data, db2.read_range(0, 100), "temp", "temp")
    assert ij.do().size > 0

    oj = OuterJoin(data, db2.read_range(0, 100), "temp", "temp")
    assert oj.do().size > 0

    loj = LeftOuterJoin(data, db2.read_range(0, 100), "temp", "temp")
    assert loj.do().size > 0

    db.close()
    db2.close()
    os.remove("test.csv")
    os.remove("test2.csv")
    os.remove("test.csv.schema")
    os.remove("test2.csv.schema")


def test_schema_with_stampdb():
    # Test schema validation through StampDB
    schema = {"temperature": "float", "status": "string", "active": "bool"}
    
    # Test valid data
    db = StampDB("test_schema.csv", schema=schema)
    valid_point = Point(time=0, data=[23.5, "online", True])
    assert db.append_point(valid_point) is True
    
    # Test invalid data
    invalid_point = Point(time=1, data=["not a float", "online", True])
    with pytest.raises(ValueError):
        db.append_point(invalid_point)
    
    # Verify only valid point was added
    data = db.read_range(0, 10)
    assert len(data) == 1
    assert data["temperature"][0] == 23.5
    
    # Test update with valid data
    updated_point = Point(time=0, data=[25.0, "online", False])
    assert db.update_point(updated_point) is True
    
    # Test update with invalid data
    invalid_update = Point(time=0, data=[25.0, 123, False])
    with pytest.raises(ValueError):
        db.update_point(invalid_update)
    
    db.close()
    os.remove("test_schema.csv")
    os.remove("test_schema.csv.schema")
