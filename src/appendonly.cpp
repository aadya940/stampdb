#include "../include/internal/csvparse.hpp"

// Actually append only DB Implementation.
// This will be used along with a shadow copy.
// The functions will be checkpointed after a few writes.
// At this point a new Shadow copy will be created.

// Opens file in Append mode.
std::ofstream openFileInAppend(const std::string& filename){
    std::ofstream file(filename, std::ios::app);
    return file;
}


// Writes Newly added Points in Append mode.
// Should be used in conjunction with `openFileInAppend`. 
// File can be closed later.
bool writeToCSV(const CSVData& csv, std::ofstream& file, NewAdded& newAdded) {    
    if (!file.is_open()) {
        return false;
    }

    std::vector<std::vector<std::string>> rows;
    for (const auto& point : csv.points) {
        rows.push_back(pointToVector(point));
    }

    if (!rows.empty()) {
        csv2::Writer<csv2::delimiter<','>> writer(file);
        writer.write_rows(rows);
        return file.good();
    }
    
    return true;  // No rows to write is not an error
}

// For deletions we would have to periodically rewrite the complete file and replace atomically.
// This can be done in a seperately and update indices succeeding it.
