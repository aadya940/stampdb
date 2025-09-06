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

    for (const auto& index : newAdded.indices) {
        if (index.index >= 0 && index.index < static_cast<int>(csv.points.size())) {
            rows.push_back(pointToVector(csv.points.at(index.index)));
        }
    }

    if (!rows.empty()) {
        csv2::Writer<csv2::delimiter<','>> writer(file);
        writer.write_rows(rows);
        
        // Clear the indices if everything was written successfully
        newAdded.indices.clear();
        return true;
    }
    
    return false;
}

// For deletions we would have to periodically rewrite the complete file and replace atomically.
// This can be done in a seperately and update indices succeeding it.
