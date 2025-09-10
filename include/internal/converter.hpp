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

    for (size_t i = 0; i < first_point.rows.size(); ++i) {
        const auto& variant = first_point.rows[i].data;

        if (std::holds_alternative<int>(variant)) {
            fields.emplace_back(headers[i], py::dtype::of<int32_t>());
        } else if (std::holds_alternative<double>(variant)) {
            fields.emplace_back(headers[i], py::dtype::of<double>());
        } else if (std::holds_alternative<std::string>(variant)) {
            fields.emplace_back(headers[i], py::dtype("U"));
        } else if (std::holds_alternative<bool>(variant)) {
            fields.emplace_back(headers[i], py::dtype::of<bool>());
        } else {
            throw std::runtime_error("Unsupported data type in first row");
        }
    }

    std::vector<std::string> names;
    std::vector<py::dtype> dtypes;
    std::vector<ssize_t> offsets;
    ssize_t offset = 0;

    for (const auto& [name, dt] : fields) {
        names.push_back(name);
        dtypes.push_back(dt);
        offsets.push_back(offset);
        offset += dt.itemsize();
    }

    py::list field_list;
    for (size_t i = 0; i < fields.size(); ++i) {
        field_list.append(py::make_tuple(names[i], dtypes[i]));
    }
    
    py::dtype dtype = py::dtype::from_args(field_list);
    std::vector<ssize_t> shape = {static_cast<ssize_t>(num_rows)};
    py::array result(dtype, shape);
    char* base_ptr = static_cast<char*>(result.mutable_data());

    for (size_t row = 0; row < num_rows; ++row) {
        const auto& point = points[row];
        const auto& row_data = point.rows;
        char* row_ptr = base_ptr + row * dtype.itemsize();

        *reinterpret_cast<double*>(row_ptr + offsets[0]) = point.time;

        for (size_t col = 0; col < row_data.size() && col < fields.size() - 1; ++col) {
            char* field_ptr = row_ptr + offsets[col + 1];
            const auto& variant = row_data[col].data;

            if (std::holds_alternative<int>(variant)) {
                *reinterpret_cast<int32_t*>(field_ptr) = std::get<int>(variant);
            } else if (std::holds_alternative<double>(variant)) {
                *reinterpret_cast<double*>(field_ptr) = std::get<double>(variant);
            } else if (std::holds_alternative<std::string>(variant)) {
                std::string str = std::get<std::string>(variant);
                auto py_str = py::str(str).attr("ljust")(64).cast<py::str>();
                std::memcpy(field_ptr, py_str.cast<std::string>().c_str(), 64 * sizeof(char));
            } else if (std::holds_alternative<bool>(variant)) {
                *reinterpret_cast<bool*>(field_ptr) = std::get<bool>(variant);
            }
        }
    }

    return result;
}
