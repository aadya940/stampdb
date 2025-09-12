#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

#ifdef _MSC_VER
    #include <BaseTsd.h>
    typedef SSIZE_T ssize_t;
#else
    #include <cstddef>
#endif

namespace py = pybind11;

py::array convertToStructuredArray(const CSVData& csv) {
    const auto& headers = csv.headers;
    const auto& points = csv.points;

    if (points.empty()) {
        throw std::runtime_error("CSV data is empty.");
    }

    size_t num_rows = points.size();
    size_t num_cols = headers.size();
    
    const auto& first_point = points[0];
    std::vector<std::pair<std::string, py::dtype>> fields;

    // Create field for time (assuming it's always the first column)
    fields.emplace_back(headers[0], py::dtype::of<double>());

    // Create fields for the rest of the columns based on the data types
    for (size_t i = 0; i < first_point.rows.size(); ++i) {
        const auto& variant = first_point.rows[i].data;
        
        // Note: i+1 because headers[0] is time, headers[i+1] corresponds to rows[i]
        size_t header_idx = i + 1;
        if (header_idx >= headers.size()) {
            throw std::runtime_error("Mismatch between headers and data columns");
        }

        if (std::holds_alternative<int>(variant)) {
            fields.emplace_back(headers[header_idx], py::dtype::of<int32_t>());
        } else if (std::holds_alternative<double>(variant)) {
            fields.emplace_back(headers[header_idx], py::dtype::of<double>());
        } else if (std::holds_alternative<std::string>(variant)) {
            fields.emplace_back(headers[header_idx], py::dtype("U64")); // Fixed string length
        } else if (std::holds_alternative<bool>(variant)) {
            fields.emplace_back(headers[header_idx], py::dtype::of<bool>());
        } else {
            throw std::runtime_error("Unsupported data type in first row");
        }
    }

    // Create the structured dtype
    py::list field_list;
    for (const auto& [name, dt] : fields) {
        field_list.append(py::make_tuple(name, dt));
    }
    
    py::dtype dtype = py::dtype::from_args(field_list);
    std::vector<ssize_t> shape = {static_cast<ssize_t>(num_rows)};
    py::array result(dtype, shape);
    char* base_ptr = static_cast<char*>(result.mutable_data());

    // Calculate offsets properly
    std::vector<ssize_t> offsets;
    ssize_t offset = 0;
    for (const auto& [name, dt] : fields) {
        offsets.push_back(offset);
        offset += dt.itemsize();
    }

    // Populate the array
    for (size_t row = 0; row < num_rows; ++row) {
        const auto& point = points[row];
        const auto& row_data = point.rows;
        char* row_ptr = base_ptr + row * dtype.itemsize();

        // Set the time field (first field)
        *reinterpret_cast<double*>(row_ptr + offsets[0]) = point.time;

        // Set the remaining fields
        for (size_t col = 0; col < row_data.size() && col + 1 < fields.size(); ++col) {
            char* field_ptr = row_ptr + offsets[col + 1];
            const auto& variant = row_data[col].data;

            if (std::holds_alternative<int>(variant)) {
                *reinterpret_cast<int32_t*>(field_ptr) = std::get<int>(variant);
            } else if (std::holds_alternative<double>(variant)) {
                *reinterpret_cast<double*>(field_ptr) = std::get<double>(variant);
            } else if (std::holds_alternative<std::string>(variant)) {
                std::string str = std::get<std::string>(variant);
                // Ensure string fits in the allocated space (64 chars)
                if (str.length() >= 64) {
                    str = str.substr(0, 63); // Leave room for null terminator
                }
                std::memcpy(field_ptr, str.c_str(), str.length() + 1);
                // Pad remaining space with zeros
                std::memset(field_ptr + str.length() + 1, 0, 64 - str.length() - 1);
            } else if (std::holds_alternative<bool>(variant)) {
                *reinterpret_cast<bool*>(field_ptr) = std::get<bool>(variant);
            }
        }
    }

    return result;
}