# StampDB

<p align="center">
  <img src="logo.png" alt="drawing" width="350" height="350">
</p>


**StampDB** is a performant time series database inspired by [tinyflux](https://github.com/citrusvanilla/tinyflux), with a focus on maximizing compatibility with the PyData ecosystem.
It is designed to work natively with NumPy and Pythons datetime module.

## Key Features

### C++ Core
-  Efficient CSV Parsing (`csv2` based).
-  In-Memory Indexing for fast lookups.
-  Append-Only Writes for data integrity.
-  Simple and fast Range Queries.
-  Atmoic Writes.

### Python Frontend
-  Seamless conversion from C++ CSV objects to NumPy structured arrays.
-  Relational algebra operations like joins, summations, and more using NumPy on structured arrays.
-  Use Native Datetime objects for I/O.

**You should not use StampDB if you need advanced database features like:**

- Access from multiple processes or threads
- An HTTP server
- Management of relationships between tables
- Access control and users
- ACID guarantees
- High performance as the size of your dataset grows

The Python API is in Development phase currently.

### Use Cases

- IOT and Sensor Data
- Scientific and Research Data Acquisition
- Single Node Data Processing
- Private Data Storage

## Installation
```
pip install -r requirements.txt
python setup.py build_ext --inplace
pip install .
```

## Quick Start

I/O using StampDB.

```python
from stampdb import *

# This will create a csv store with time, temp, humidity columns.
db = StampDB("test.csv", schema={"temp": "float", "humidity": "float"})

# Appending a point.
p = Point(time=1, data=[22.5, "moderate"])
db.append_point(p)

# Doing append only writes to the disk.
db.checkpoint()

# Doing in memory deletion.
db.delete_point(time=1)

# Forcing actual disk deletion.
db.compact() # If not done explicitly, it happens on close.

# Closing the database.
db.close()
```

Relational Algebra using StampDB.

```python
from stampdb.relational import *

# Given the db is loaded and running using the `Quick Start` section.

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

```

Joins using StampDB.

```python
from stampdb.relational import *

db = StampDB("test.csv", schema={"temp": "float", "humidity": "float"})
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
```
