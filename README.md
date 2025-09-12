# StampDB

<img src="logo.png" alt="drawing" align="center" width="350" height="350" />

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
