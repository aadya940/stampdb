#include "../include/internal/csvparse.hpp"


bool __parse_bool(const std::string& value);

// CSV Management.
// All functions here work on complete CSV files.
// Entire CSV Files are loaded and entire CSV Files are written.

CSVData parseCSV(const std::string& filename, FullIndex& dbIndex) {
    CSVData csv;

    csv2::Reader<csv2::delimiter<','>,
                 csv2::quote_character<'"'>,
                 csv2::first_row_is_header<true>,
                 csv2::trim_policy::trim_whitespace> reader;

    if (reader.mmap(filename)) {
        // headers
        for (const auto& header : reader.header()) {
            std::string value;
            header.read_value(value);
            csv.headers.push_back(value);
        }

        int iter = 0;

        // returns `size_t` number of rows.
        dbIndex.MAX_ROWNUM = (int)reader.rows();

        // rows
        for (const auto& row : reader) {

            if (row.length() < csv.headers.size()) {
                // Skip malformed or empty rows
                dbIndex.MAX_ROWNUM--;
                continue;
            }        

            Point point;
            bool first = true;

            Index thisIndex;
            thisIndex.index = iter;
            iter++;

            for (const auto& cell : row) {
                std::string value;
                cell.read_value(value);

                if (first) {
                    // treat first col as time
                    point.time = std::stod(value);
                    first = false;

                    thisIndex.time = point.time;

                    // Insert the index in sorted order by time
                    insertIndexSorted(dbIndex, thisIndex);

                    continue;
                }

                // try bool
                try {
                    point.rows.push_back({__parse_bool(value)});
                    continue;
                
                } catch (...) {}

                // try double
                try {
                    point.rows.push_back({std::stod(value)});
                    continue;
                } catch (...) {}

                // try int
                try {
                    point.rows.push_back({std::stoi(value)});
                    continue;
                } catch (...) {}

                // fallback string
                point.rows.push_back({value});
            }

            csv.points.push_back(point);
        }
    }

    return csv;
}


CSVData appendRow(CSVData& csv, const Point& point, FullIndex& dbIndex, NewAdded& newAdded) {
    // Store the current size as the index for the new point
    int newIndex = static_cast<int>(csv.points.size());
    csv.points.push_back(point);

    // Update indices
    Index thisIndex;
    thisIndex.index = newIndex;
    thisIndex.time = point.time;
        
    // Update the main index
    dbIndex.MAX_ROWNUM++;
    insertIndexSorted(dbIndex, thisIndex);

    // Add to newAdded for checkpointing
    newAdded.indices.push_back(thisIndex);


    return csv;
}


std::string variantToString(const std::variant<std::string, double, int, bool>& v) {
    return std::visit([](auto&& arg) -> std::string {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::string>) {
            return arg;
        } else if constexpr (std::is_same_v<T, bool>) {
            return arg ? "true" : "false";
        } else {
            std::ostringstream oss;
            oss << arg;
            return oss.str();
        }
    }, v);
}


std::vector<std::string> pointToVector(const Point& point) {
    std::vector<std::string> vec;
    
    // Add time as the first column
    vec.push_back(std::to_string(point.time));
    
    // Add the rest of the row data
    for (const auto& row : point.rows) {
        vec.push_back(variantToString(row.data));
    }

    return vec;
}



// Rewrites the complete CSV.
void writeCSV(const std::string& filename, const CSVData& csv) {
    std::ofstream file(filename);
    
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file");
    }

    csv2::Writer<csv2::delimiter<','>> writer(file);
    std::vector<std::vector<std::string>> rows;

    // headers
    rows.push_back(csv.headers);

    // rows
    for (const auto& point : csv.points) {
        rows.push_back(pointToVector(point));
    }

    writer.write_rows(rows);
    file.close();
}



// Function to print CSV data - kept for debugging purposes but not used in production
void printCSVData(const CSVData& data) {
    // Implementation removed for production use
}


CSVData deletePointwithIndex(CSVData& data, int index, double time, FullIndex& dbIndex, DeletedIndices& deletedIndices) {
    if (index >= 0 && index < data.points.size()) {
        // Record this deletion in the global deletedIndices
        Index thisDeletedIndex;
        thisDeletedIndex.index = index;
        thisDeletedIndex.time = time;
        deletedIndices.indices.push_back(thisDeletedIndex);

        // Remove the point from the data
        data.points.erase(data.points.begin() + index);

        // Find and remove the corresponding index from dbIndex
        auto it = std::find_if(dbIndex.indices.begin(), dbIndex.indices.end(),
            [index](const Index& idx) { return idx.index == index; });
            
        if (it != dbIndex.indices.end()) {
            dbIndex.indices.erase(it);
        }

        // Adjust indices in dbIndex that are greater than the deleted one
        for (auto& idx : dbIndex.indices) {
            if (idx.index > index) {
                idx.index--;
            }
        }

        // Update MAX_ROWNUM if needed
        if (dbIndex.MAX_ROWNUM > 0) {
            dbIndex.MAX_ROWNUM--;
        }
    }

    return data;
}


// Private utility function.
bool __parse_bool(const std::string& value) {
    std::string val = value;
    // convert to lowercase for case-insensitive comparison
    std::transform(val.begin(), val.end(), val.begin(), ::tolower);
    if (val == "true") {
        return true;
    } else if (val == "false") {
        return false;
    }

    throw std::invalid_argument("Not a bool");
}
