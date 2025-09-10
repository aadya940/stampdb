#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <variant>
#include <sstream>

#include "../../include/internal/csvparse.hpp"
#include "../../include/stampdb.hpp"
#include "../../include/internal/converter.hpp"

namespace py = pybind11;

PYBIND11_MODULE(_types, m) {

    // Index
    py::class_<Index>(m, "Index")
        .def(py::init<>())
        .def_readwrite("time", &Index::time)
        .def_readwrite("index", &Index::index)
        .def("__repr__", [](const Index& idx) {
            std::ostringstream oss;
            oss << "Index(time=" << idx.time << ", index=" << idx.index << ")";
            return oss.str();
        });


    // FullIndex
    py::class_<FullIndex>(m, "FullIndex")
        .def(py::init<>())
        .def_readwrite("indices", &FullIndex::indices)
        .def_readwrite("MAX_ROWNUM", &FullIndex::MAX_ROWNUM)
        .def("__repr__", [](const FullIndex& fi) {
            std::ostringstream oss;
            oss << "FullIndex(indices=" << fi.indices.size() << " items"
                << ", MAX_ROWNUM=" << fi.MAX_ROWNUM << ")";
            return oss.str();
        });


    // NewAdded
    py::class_<NewAdded>(m, "NewAdded")
        .def(py::init<>())
        .def_readwrite("indices", &NewAdded::indices)
        .def("__repr__", [](const NewAdded& na) {
            std::ostringstream oss;
            oss << "NewAdded(indices=" << na.indices.size() << " items)";
            return oss.str();
        });


    // DeletedIndices
    py::class_<DeletedIndices>(m, "DeletedIndices")
        .def(py::init<>())
        .def_readwrite("indices", &DeletedIndices::indices)
        .def("__repr__", [](const DeletedIndices& di) {
            std::ostringstream oss;
            oss << "DeletedIndices(indices=" << di.indices.size() << " items)";
            return oss.str();
        });


    // PointRow with variant handling
    py::class_<PointRow>(m, "PointRow")
        .def(py::init<>())
        .def_property("data",
            [](const PointRow& row) -> py::object {
                return std::visit([](const auto& value) -> py::object {
                    return py::cast(value);
                }, row.data);
            },
            [](PointRow& row, py::object obj) {
                if (py::isinstance<py::str>(obj)) {
                    row.data = obj.cast<std::string>();
                } else if (py::isinstance<py::float_>(obj)) {
                    row.data = obj.cast<double>();
                } else if (py::isinstance<py::int_>(obj)) {
                    row.data = obj.cast<int>();
                } else if (py::isinstance<py::bool_>(obj)) {
                    row.data = obj.cast<bool>();
                } else {
                    throw std::runtime_error("Unsupported type for PointRow.data");
                }
            })
        .def("__repr__", [](const PointRow& row) {
            std::ostringstream oss;
            oss << "PointRow(data=";
            std::visit([&oss](const auto& value) {
                oss << value;
            }, row.data);
            oss << ")";
            return oss.str();
        });


    // Point
    py::class_<Point>(m, "_Point")
        .def(py::init<>())
        .def_readwrite("time", &Point::time)
        .def_readwrite("rows", &Point::rows)
        .def("add_row", [](Point& p, const PointRow& row) {
            p.rows.push_back(row);
        }, "Add a row to the point")
        .def("get_rows", [](const Point& p) {
            return p.rows;
        }, "Get all rows")
        .def("__repr__", [](const Point& p) {
            std::ostringstream oss;
            oss << "_Point(time=" << p.time << ", rows=[";
            for (size_t i = 0; i < p.rows.size(); ++i) {
                std::visit([&oss](const auto& value) {
                    oss << value;
                }, p.rows[i].data);
                if (i != p.rows.size() - 1) oss << ", ";
            }
            oss << "])";
            return oss.str();
        });


    // CSVData
    py::class_<CSVData>(m, "CSVData")
        .def(py::init<>())
        .def("append_point", [](CSVData& self, const Point& p) {
            self.points.push_back(p);
        })
        .def_readwrite("headers", &CSVData::headers)
        .def_readwrite("points", &CSVData::points)
        .def("__repr__", [](const CSVData& csv) {
            std::ostringstream oss;
            oss << "CSVData(headers=[";
            for (size_t i = 0; i < csv.headers.size(); ++i) {
                oss << '"' << csv.headers[i] << '"';
                if (i != csv.headers.size() - 1) oss << ", ";
            }
            oss << "], points=" << csv.points.size() << " entries)";
            return oss.str();
        });
    
    py::class_<StampDB>(m, "StampDB")
        .def(py::init<const std::string&>(), "Constructor with filename")
        
        // CRUD Operations
        .def("read", &StampDB::read, "Read data at specific time")
        .def("read_range", &StampDB::read_range, "Read data in time range")
        .def("delete_point", &StampDB::delete_point, "Delete point at specific time")
        .def("append_point", &StampDB::appendPoint, "Append a new point")
        .def("update_point", &StampDB::updatePoint, "Update an existing point")
        
        // Database Management
        .def("compact", &StampDB::compact, "Compact the database")
        .def("checkpoint", &StampDB::checkpoint, "Checkpoint the database")
        .def("close", &StampDB::close, "Close the database")
        
        // Configuration
        .def_readwrite("CHECKPOINT", &StampDB::CHECKPOINT, "Checkpoint threshold")
        .def_static("as_numpy_structured_array", &convertToStructuredArray, "Convert CSVData to NumPy structured array");
}
