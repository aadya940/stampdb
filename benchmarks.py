import timeit
import random
from datetime import datetime, timedelta, timezone
import numpy as np

# TinyFlux imports
from tinyflux import TinyFlux, Point, TimeQuery, FieldQuery

# StampDB imports
from stampdb import StampDB, Point as StampPoint

NUM_POINTS = 10_000
RANGE_START = 0
RANGE_END = NUM_POINTS - 1

def generate_data():
    base_time = datetime.now(tz=timezone.utc)
    times = [base_time + timedelta(seconds=i) for i in range(NUM_POINTS)]
    values = [random.uniform(15, 35) for _ in range(NUM_POINTS)]
    return times, values

def benchmark_tinyflux(times, values):
    db = TinyFlux("tinyflux.csv")

    def append():
        for t, v in zip(times, values):
            p = Point(time=t, tags={"room": "bedroom"}, fields={"temp": v})
            db.insert(p, compact_key_prefixes=True)
    write_time = timeit.timeit(append, number=1)

    t0 = times[RANGE_START]
    t1 = times[RANGE_END]

    def range_query():
        Time = TimeQuery()
        results = db.search((Time >= t0) & (Time <= t1))
        return results
    query_time = timeit.timeit(range_query, number=1)

    def aggregate():
        Field = FieldQuery()
        results = db.search(Field.temp > 0)
        temps = [p.fields["temp"] for p in results]
        return sum(temps) / len(temps) if temps else None
    agg_time = timeit.timeit(aggregate, number=1)

    db.close()
    return write_time, query_time, agg_time

def benchmark_stampdb(times, values):
    db = StampDB("stampdb.csv", schema={"temp": "float"})

    def append():
        for t, v in zip(times, values):
            ts = int(t.timestamp())
            db.append_point(StampPoint(time=ts, data=[v]))
        db.compact()
    write_time = timeit.timeit(append, number=1)

    t0 = int(times[RANGE_START].timestamp())
    t1 = int(times[RANGE_END].timestamp())

    def range_query():
        return db.read_range(t0, t1)
    query_time = timeit.timeit(range_query, number=1)

    def aggregate():
        data = db.read_range(t0, t1)
        return np.mean(data["temp"]) if data.size > 0 else None
    agg_time = timeit.timeit(aggregate, number=1)

    db.close()
    return write_time, query_time, agg_time

def run_benchmarks():
    times, values = generate_data()
    print("Benchmarking TinyFlux...")
    tf_write, tf_query, tf_agg = benchmark_tinyflux(times, values)
    print(f"TinyFlux: Append={tf_write:.3f}s, Query={tf_query:.3f}s, Aggregate={tf_agg:.3f}s")

    print("Benchmarking StampDB...")
    stamp_write, stamp_query, stamp_agg = benchmark_stampdb(times, values)
    print(f"StampDB: Append={stamp_write:.3f}s, Query={stamp_query:.3f}s, Aggregate={stamp_agg:.3f}s")

    # Cleanup files
    import os
    os.remove("tinyflux.csv")
    os.remove("stampdb.csv")
    os.remove("stampdb.csv.schema")
    
    return tf_write, tf_query, tf_agg, stamp_write, stamp_query, stamp_agg

if __name__ == "__main__":
    tf_write, tf_query, tf_agg, stamp_write, stamp_query, stamp_agg = run_benchmarks()

    avg_write_speedup = []
    avg_query_speedup = []
    avg_agg_speedup = []

    for i in range(100):
        tf_write, tf_query, tf_agg, stamp_write, stamp_query, stamp_agg = run_benchmarks()
        avg_write_speedup.append(tf_write / stamp_write)
        avg_query_speedup.append(tf_query / stamp_query)
        avg_agg_speedup.append(tf_agg / stamp_agg)

    print("StampDB Speedups as compared to tinyflux:")
    print("Writes: ",  np.mean(avg_write_speedup), "x")
    print("Queries: ", np.mean(avg_query_speedup), "x")
    print("Aggregates: ", np.mean(avg_agg_speedup), "x")
