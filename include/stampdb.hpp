#pragma once

#include <string>

#include "internal/fileio.hpp"
#include "internal/csvparse.hpp"

class StampDB {
public:
    // Constructor/Destructor
    explicit StampDB(const std::string& filename);
    ~StampDB() = default;

    // Disable copy/move for now
    StampDB(const StampDB&) = delete;
    StampDB& operator=(const StampDB&) = delete;
    StampDB(StampDB&&) = delete;
    StampDB& operator=(StampDB&&) = delete;

    // CRUD Operations
    CSVData read(double time);
    CSVData read_range(double startTime, double endTime);
    CSVData delete_point(double time);
    bool appendPoint(const Point& point);
    
    // Database Management
    CSVData compact();
    bool checkpoint();
    void close();

    // Configuration
    int CHECKPOINT = 10;  // Number of operations before auto-compaction

private:
    std::string filename;
    std::string shadowFilename;
    CSVData data;
    FullIndex dbIndex;  // std::vector<Index> sorted by time
    NewAdded newAdded;  // Tracks newly added indices
    DeletedIndices deletedIndices;  // Tracks deleted indices
    int operationCount;
};
