# StampDB

**⚠️ Currently in ideation phase - not ready for production use**

A high-performance time series database inspired by [Bitcask](https://github.com/basho/bitcask), built on [`numpy.memmap`](https://numpy.org/devdocs/reference/generated/numpy.memmap.html). Written in Python with a focus on speed and efficiency for time series data.

## Architecture

StampDB organizes data as a simple filesystem structure with separate data and indexing layers:

```
/mydb/
  ├── data/
  │   ├── segment_0001.dat
  │   ├── segment_0002.dat
  │   └── ...
  ├── index/
      └── hashmap.json
```

### Data Storage

Data files are segmented with a maximum size of `max(array_size, 5MB)`. When writing new arrays:
- If the current segment can accommodate the complete array, data is appended
- If the segment is full, a new segment file is created
- The database supports single-writer, multi-reader access patterns

### Index Management

The index maintains an in-memory Python dictionary for fast lookups, persisted as JSON. Each timestamp maps to metadata containing file location and integrity information:

```python
{
    "timestamp": [
        filename,   # segment file containing the data
        offset,     # byte offset within the file
        length,     # data length in bytes
        checksum,   # CRC32 hash for integrity verification
        datatype,   # numpy data type
        shape,      # array dimensions
    ]
}
```

### Data Integrity

Every data access triggers checksum verification using CRC32. Mismatched checksums raise warnings to alert of potential corruption. When data is updated at existing timestamps, the index points to the new location, checksum is updated and orphaned arrays are automatically cleaned up by the StampDB daemon clearing space in existing files and new data can be added there if it is smaller than 5MB and the new array can fit in.

## Query Interface

StampDB provides a numpy-based query language for filtering and retrieving time series data:

```python
from stampdb import StampDB, NOW
import numpy as np

db = StampDB("mydb")

def outlier(x):
    return x.mean() > 5

result = db.query(
    NOW() - 1000,  # start timestamp (auto-converted to Unix epoch)
    NOW(),         # end timestamp (auto-converted to Unix epoch)
    outlier,       # numpy predicate function returning boolean
    size=(100, 50) # output array dimensions or max size constraint
)

# Results are memory-mapped numpy arrays
assert result.shape[1:] == (100, 50) or result.shape[1:] == result[1].shape
assert isinstance(result, np.ndarray)
```

The query system returns numpy memory-mapped arrays containing all data points that satisfy the predicate function, maintaining consistent dimensions across results.


### How's it different from SQL?

Traditional SQL databases use B-trees because they need to support complex queries, joins, and range scans efficiently. But for time series data like Stocks, EEG etc., you're usually just writing new data points (arrays) and reading back ranges by timestamp (I think much simpler access patterns).
StampDB uses a hash index (like Bitcask) which gives O(1) lookups by timestamp, plus the append-only log structure is perfect for time series since you're constantly writing new data chronologically. The memory-mapped files let you treat the data like it's in RAM even when it's on disk, which is great for numerical computations.
