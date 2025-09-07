#include <pybind11/pybind11.h>
#include <pybind11/stl.h>


#include "../../include/internal/csvparse.hpp"


namespace py = pybind11;


PYBIND11_MODULE(_backend, m) {

    // Index
    py::class_<Index>(m, "Index")
        .def(py::init<>())
        .def_readwrite("time", &Index::time)
        .def_readwrite("index", &Index::index);


    // FullIndex
    py::class_<FullIndex>(m, "FullIndex")
        .def(py::init<>())
        .def_readwrite("indices", &FullIndex::indices)
        .def_readwrite("MAX_ROWNUM", &FullIndex::MAX_ROWNUM);


    // NewAdded
    py::class_<NewAdded>(m, "NewAdded")
        .def(py::init<>())
        .def_readwrite("indices", &NewAdded::indices);


    // DeletedIndices
    py::class_<DeletedIndices>(m, "DeletedIndices")
        .def(py::init<>())
        .def_readwrite("indices", &DeletedIndices::indices);


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
            }
        );


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
        }, "Get all rows");


    // CSVData
    py::class_<CSVData>(m, "CSVData")
        .def(py::init<>())
        .def_readwrite("headers", &CSVData::headers)
        .def_readwrite("points", &CSVData::points);
}
