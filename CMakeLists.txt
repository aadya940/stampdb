cmake_minimum_required(VERSION 3.10)
project(DatabaseTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the csv2 library include directory
include_directories(${PROJECT_SOURCE_DIR}/libs/csv2/include)

# Add header definitions
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/internal)


# Add the example executable
add_executable(test 
    src/csvparse.cpp
    src/appendonly.cpp
    src/atomicity.cpp
    src/algorithms.cpp
    src/fileio.cpp
    src/stampdb.cpp
    test.cpp
)
