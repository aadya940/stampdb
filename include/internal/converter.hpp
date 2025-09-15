#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <Python.h>
#include <cstring>

#ifdef _MSC_VER
    #include <BaseTsd.h>
    #ifndef ssize_t
        typedef SSIZE_T ssize_t;
    #endif
#else
    #include <cstddef>
#endif


namespace py = pybind11;

py::array convertToStructuredArray(const CSVData& csv) {
    const auto& headers = csv.headers;
    const auto& points = csv.points;

    if (points.empty()) {
        return py::array();
    }

    size_t num_rows = points.size();
    const auto& first_point = points[0];
    std::vector<std::pair<std::string, py::dtype>> fields;

    // Create field for time (assuming it's always the first column)
    fields.emplace_back(headers[0], py::dtype::of<double>());

    // Create fields for the rest of the columns based on the data types
    for (size_t i = 0; i < first_point.rows.size(); ++i) {
        const auto& variant = first_point.rows[i].data;
        size_t header_idx = i + 1;
        
        if (header_idx >= headers.size()) {
            throw std::runtime_error("Mismatch between headers and data columns");
        }

        if (std::holds_alternative<int>(variant)) {
            fields.emplace_back(headers[header_idx], py::dtype::of<int32_t>());
        } else if (std::holds_alternative<double>(variant)) {
            fields.emplace_back(headers[header_idx], py::dtype::of<double>());
        } else if (std::holds_alternative<std::string>(variant)) {
            // Use a reasonable fixed-size string - 256 characters should handle most cases
            fields.emplace_back(headers[header_idx], py::dtype("U256"));
        } else if (std::holds_alternative<bool>(variant)) {
            fields.emplace_back(headers[header_idx], py::dtype::of<bool>());
        } else {
            throw std::runtime_error("Unsupported data type in first row");
        }
    }

    // Create the structured dtype and array
    py::list field_list;
    for (const auto& [name, dt] : fields) {
        field_list.append(py::make_tuple(name, dt));
    }
    
    py::dtype dtype = py::dtype::from_args(field_list);
    std::vector<ssize_t> shape = {static_cast<ssize_t>(num_rows)};
    py::array result(dtype, shape);
    char* base_ptr = static_cast<char*>(result.mutable_data());

    // Calculate field offsets
    std::vector<ssize_t> offsets;
    ssize_t offset = 0;
    for (const auto& field : fields) {
        offsets.push_back(offset);
        offset += field.second.itemsize();
    }

    // Populate the array
    for (size_t row = 0; row < num_rows; ++row) {
        const auto& point = points[row];
        char* row_ptr = base_ptr + row * dtype.itemsize();

        // Set the time field (first field)
        *reinterpret_cast<double*>(row_ptr + offsets[0]) = point.time;

        // Set the remaining fields
        size_t point_rows_size = point.rows.size();
        size_t fields_size_minus_one = fields.size() - 1;
        size_t max_cols = (point_rows_size < fields_size_minus_one) ? point_rows_size : fields_size_minus_one;
        for (size_t col = 0; col < max_cols; ++col) {
            char* field_ptr = row_ptr + offsets[col + 1];
            const auto& variant = point.rows[col].data;

            if (std::holds_alternative<int>(variant)) {
                *reinterpret_cast<int32_t*>(field_ptr) = std::get<int>(variant);
            } else if (std::holds_alternative<double>(variant)) {
                *reinterpret_cast<double*>(field_ptr) = std::get<double>(variant);
            } else if (std::holds_alternative<std::string>(variant)) {
                const std::string& str = std::get<std::string>(variant);
                size_t field_size = fields[col + 1].second.itemsize();
                
                // Zero the field
                std::memset(field_ptr, 0, field_size);
                
                // Simple UTF-32 conversion for ASCII strings
                char32_t* u32_ptr = reinterpret_cast<char32_t*>(field_ptr);
                size_t max_chars = (str.length() < (field_size / sizeof(char32_t) - 1)) ? 
                                   str.length() : (field_size / sizeof(char32_t) - 1);
                
                for (size_t i = 0; i < max_chars; ++i) {
                    u32_ptr[i] = static_cast<char32_t>(static_cast<unsigned char>(str[i]));
                }
            } else if (std::holds_alternative<bool>(variant)) {
                *reinterpret_cast<bool*>(field_ptr) = std::get<bool>(variant);
            }
        }
    }

    return result;
}
