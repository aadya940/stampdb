#include "../include/stampdb.hpp"


StampDB::StampDB(const std::string& filename) : filename(filename), shadowFilename(filename + ".tmp"), operationCount(0) {
    // Load data using existing parseCSV function and initialize index
    this->data = parseCSV(filename, this->dbIndex);
    
    // Create shadow copy using existing file I/O functions
    createShadowCopy(filename);
}

CSVData StampDB::read(double time) {
    CSVData result;
    result.headers = this->data.headers;
    
    // Use findInTimeRange with a zero-width range to find exact time match
    auto range = findInTimeRange(this->dbIndex, time, time);
    if (!range.empty()) {
        result.points.push_back(this->data.points[range[0].index]);
    }
    
    return result;
}

CSVData StampDB::read_range(double startTime, double endTime) {
    CSVData result;
    result.headers = this->data.headers;
    
    auto range = findInTimeRange(this->dbIndex, startTime, endTime);
    for (const auto& idx : range) {
        result.points.push_back(this->data.points[idx.index]);
    }
    
    return result;
}

CSVData StampDB::delete_point(double time) {
    // Find the exact time match
    auto range = findInTimeRange(this->dbIndex, time, time);
    if (!range.empty()) {
        // Delete the point using the found index
        this->data = deletePointwithIndex(this->data, range[0].index, time, this->dbIndex, this->deletedIndices);
    }
    return this->data;
}

bool StampDB::checkpoint() {
    if (this->newAdded.indices.empty()) {
        return true;  // Nothing to checkpoint
    }
    
    // Ensure shadow file exists and is up to date
    if (!std::filesystem::exists(shadowFilename)) {
        if (!createShadowCopy(filename)) {
            throw std::runtime_error("Failed to create initial shadow copy");
        }
    }
    
    std::cout << "Opening File in Append Mode ..." << std::endl;
    std::ofstream file = openFileInAppend(shadowFilename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open shadow file for checkpointing");
    }
    
    std::cout << "Writing to CSV ..." << std::endl;
    bool success = writeToCSV(this->data, file, this->newAdded);
    file.close();
    
    std::cout << "Closing File in Append Mode ..." << std::endl;

    if (!success) {
        throw std::runtime_error("Failed to write checkpoint data to shadow file");
    }
    
    // Atomically swap the shadow file with the main database file
    if (!swapShadowAsDb(this->filename)) {
        throw std::runtime_error("Failed to swap shadow file after checkpoint");
    }
    
    return true;
}

bool StampDB::appendPoint(const Point& point) {
    // Add the new point to our in-memory data
    this->data = appendRow(this->data, point, this->dbIndex, this->newAdded);
    
    // Check if we need to perform a checkpoint
    if (++operationCount >= CHECKPOINT) {
        checkpoint();
        operationCount = 0;
    }
    
    return true;
}

CSVData StampDB::compact() {
    // First, perform a checkpoint if there are pending writes
    checkpoint();
    
    // If there are deletions, perform compaction by creating a fresh shadow copy
    if (!deletedIndices.indices.empty()) {
        // Create a new shadow file from the current database state
        if (!createShadowCopy(filename)) {
            throw std::runtime_error("Failed to create new shadow copy for compaction");
        }
        
        // Write the compacted data to the new shadow file
        writeCSV(shadowFilename, this->data);
        
        // Atomically swap the compacted shadow file with the main database
        if (!swapShadowAsDb(this->filename)) {
            throw std::runtime_error("Failed to swap compacted shadow file");
        }
        
        // Clear the deleted indices after successful compaction
        this->deletedIndices.indices.clear();
    }
    
    return this->data;
}

void StampDB::close() {
    // Note: Compaction is now user-controlled, so we don't perform it automatically on close
    // The user should explicitly call compact() if they want to persist changes
    compact();
    
    // Clear all data structures
    this->data = {};
    this->dbIndex.indices.clear();
    this->dbIndex.MAX_ROWNUM = 0;
    this->newAdded.indices.clear();
    this->deletedIndices.indices.clear();
}
