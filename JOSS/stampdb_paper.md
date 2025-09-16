---
title: 'StampDB: A C++ Time Series Database library designed for compatibility with the PyData Ecosystem.'
tags:
    - Time Series
    - Database Systems
    - Time Series Databases
    - Python
    - PyData
    - NumPy
authors:
    - name: Aadya A. Chinubhai
      affiliation: 1
affiliations:
    - name: Santa Clara University, Santa Clara, California, United States
      index: 1
bibliography: references.bib
---

## Abstract
StampDB is a lightweight time series database designed for compatibility with the PyData Ecosystem. It provides a simple, Python-native system that handles time series data without the complexity of full-fledged database systems. We try to be
natively compatible with NumPy [@numpy] and pythons [@python] datetime [@python-datetime] module.

## 1. Introduction

### 1.1 Background and Motivation
Time Series is fundamental to many applications, from IoT to Scientific Research. However, most of the databases are
heavy-weight and are focused on solving enterprise-level problems. Also, some of them not being natively compatible
with NumPy [@numpy] miss out on most of the other libraries that natively support NumPy. This is important science
most of the data analysis work is being done in Python [@python] using libraries like Scikit-learn [@scikit-learn],
Numpy [@numpy], Pandas [@pandas], etc. All of this is done while having a C++ [@cpp] backend which provides low-level control.

### 1.2 Project Goals
StampDB was designed to:
- Provide a straightforward API for time series data management
- Maintain a minimal codebase focused on core functionality
- Be natively compatible with NumPy [@numpy] and pythons [@python] datetime [@python-datetime] module.

### 1.3 Target Use Cases
- Sensor Data
- Scientific and Research Data Acquisition
- Single Node Data Processing
- Private Data Storage

## 2. System Architecture

### 2.1 Core Components

At its heart, StampDB is a CSV [@rfc4180] file with a schema. The schema is stored in a separate file and is used to validate the data as it is written to the CSV [@rfc4180] file.

StampDB comprises of two things:
- A C++ library that provides the core functionality.
- A Python wrapper that provides a simple NumPy [@numpy] native API for the C++ library.

C++ Core:
- CSV Parsing and Reads/Writes using csv2 [@csv2].
- In-Memory time based indexing.
- Atomic Writes via shadow copies.
- Append only disk writes.
- A converter to convert CSV C++ objects to a NumPy [@numpy] structured array.

Python Wrapper:
- A simple NumPy [@numpy] native API for the C++ library.
- A simple query language for NumPy structured arrays.

## 3. You should not use StampDB if

Access from multiple processes or threads
- An HTTP server
- Management of relationships between tables
- Access control and users
- ACID guarantees
- High performance as the size of your dataset grows

## 4. API Reference

### 4.1 StampDB class

I/O using the StampDB class.

```python
class StampDB(path: str, schema: dict) — Opens or creates a CSV-backed database.
Example: db = StampDB("test.csv", schema={"temp": "float", "humidity": "float"})
    - append_point(point: Point) -> None — Append a new record to the database.
    - checkpoint() -> None — Flush in-memory append-only writes to disk.
    - delete_point(time: int | float) -> None — Mark a record deleted (in-memory).
    - compact() -> None — Physically remove deleted records from disk (auto on close).
    - read_range(start: int, end: int) -> np.ndarray — Read data between two time values.
    - close() -> None — Cleanly close the database.
```

The Point class.

```python
class Point - Represents a single time-stamped record.
Example: p = Point(time=1, data=[22.5, "moderate"])
    time: int | float
    values: dict[str, Any]
```

The Relational Algebra basic classes.
```python
class Selection(condition: str, data: np.ndarray) — Filters rows based on a condition string; do() -> np.ndarray.
class Projection(columns: list[str], data: np.ndarray) — Select specific columns; do() -> np.ndarray.
class Summation(column: str, data: np.ndarray) — Compute sum of a column; do() -> float.
class OrderBy(columns: list[str], data: np.ndarray) — Sort rows by one or more columns; do() -> np.ndarray.
```

The Relational Algebra join classes.

```python
class InnerJoin(left: np.ndarray, right: np.ndarray, left_key: str, right_key: str) — Inner join; do() -> np.ndarray.
class OuterJoin(left: np.ndarray, right: np.ndarray, left_key: str, right_key: str) — Full outer join; do() -> np.ndarray.
class LeftOuterJoin(left: np.ndarray, right: np.ndarray, left_key: str, right_key: str) — Left outer join; do() -> np.ndarray.
```

## 5. Conclusion
StampDB provides a minimal, easy-to-use solution for basic time series data storage in Python. Its simplicity makes it suitable for educational purposes and small-scale applications where a full-fledged database would be unnecessary. The implementation focuses on core functionality while maintaining a clean and maintainable codebase.


## License

We distribute under the permissive Apache License 2.0. 

## References
