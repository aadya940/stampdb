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

StampDB is a lightweight time series database designed for seamless compatibility with the PyData ecosystem. The system provides a Python-native interface that handles time series data without the architectural complexity of enterprise-grade database systems. StampDB achieves native compatibility with NumPy [@numpy] and Python's [@python] datetime module [@python-datetime], offering researchers and developers a streamlined solution for time series data management in single-node environments.

## 1. Introduction

### 1.1 Background and Motivation

Time series data processing represents a fundamental requirement across numerous application domains, spanning Internet of Things (IoT) deployments to scientific research initiatives. However, existing database solutions predominantly target enterprise-level architectures, introducing unnecessary complexity for smaller-scale applications. Furthermore, many established systems lack native NumPy [@numpy] compatibility, thereby limiting integration with the broader Python [@python] scientific computing ecosystem. This limitation proves particularly significant given that contemporary data analysis workflows predominantly utilize Python-based libraries including Scikit-learn [@scikit-learn], NumPy [@numpy], and Pandas[ @pandas]. The proposed StampDB addresses these limitations through a C++ backend implementation that provides low-level control while maintaining a Python-centric interface design.

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
