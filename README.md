### We are still in ideation phase and not in any way usable

### StampDB

A database inspired by [Bitcask](https://github.com/basho/bitcask), built on top of [`numpy.memmap`](https://numpy.org/devdocs/reference/generated/numpy.memmap.html).
Best suited for time series data.

Written in Python but Fast.

### Design

The database is a folder on the file system having `data` and `index` subdirectories.

```
/mydb/
  ├── data/
  │   ├── segment_0001.dat
  │   ├── segment_0002.dat
  │   └── ...
  ├── index/
      └── hashmap.json
```

If you update the data at a timestamp in the future, the index will point to the new 
file and the offset will point to the new file location, the checksum will also be updated.
The `StampDB` daemon will automatically
delete the old files (files that are not referenced by any index). 

The index is a Python dictionary loaded in-memory for fast
access from a JSON file if it exists, otherwise it is created from scratch.

Here's how it looks:

```python
{
    "timestamp" : [
        filename,
        offset,
        length,
        checksum,
        datatype,
        shape,
    ]
}
```

Everytime the data is loaded, the checksum is recomputed to ensure data integrity.
If the checksum does not match, a `warning` is raised. The checksum is computed via
the CRC32 Algorithm.

The database can be written by a single process and read by multiple processes.

The size of the `.dat` files is `max(array_size, 5MB)`.
When a new array is written, it is appended to the last file if the last
file is not full and can accomodate the complete array.

If the last file is full, a new file is created.

### Query Language

The query language are simple numpy functions.

```python
from stampdb import StampDB, NOW
import numpy as np

db = StampDB("mydb")

def outlier(x):
    return x.mean() > 5

out = db.query(
    NOW() - 1000, # start_timestamp, implicitly converted to Unix-Epoch
    NOW(), # end_timestamp, implicitly converted to Unix-Epoch
    outlier  # A Numpy Expression that evaluates to True or False,
    size=(100, 50) # Size of the each output array or max-size if array is smaller than (100, 50).
)

assert out.shape[1:] == (100, 50) or out.shape[1:] == out[1].shape
assert isinstance(out, np.ndarray) # The output is a numpy memory mapped array, this is a bunch of arrays that evaluate to True, they should ideally be of the same size.
```
