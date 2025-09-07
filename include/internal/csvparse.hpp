#pragma once

#include <string>
#include <iostream>
#include <vector>
#include <variant>  
#include <fstream>
#include <sstream>

#include <csv2/reader.hpp>
#include <csv2/writer.hpp>

#include <ciso646>


struct PointRow {
    std::variant<std::string, double, int, bool> data;
};


struct Point {
    double time;
    std::vector<PointRow> rows;
};


struct CSVData {
    std::vector<std::string> headers;
    std::vector<Point> points;
};


// Maps time to index for efficient deletions.
struct Index {
    double time;
    int index;
};


// Full Index
struct FullIndex {
    std::vector<Index> indices;
    int MAX_ROWNUM;
};


// Tracks new Indices added before disk write. 
// Only write these Indices.
struct NewAdded {
    std::vector<Index> indices;
};


// Tracks deleted indices.
// This is written On-disk to keep it fault tolerant.
struct DeletedIndices {
    std::vector<Index> indices;
};


// Function to parse CSV file and return CSVData structure
CSVData parseCSV(const std::string& filename, FullIndex& dbIndex);

// Function to append a row to CSV data
CSVData appendRow(CSVData& csv, const Point& point, FullIndex& dbIndex, NewAdded& newAdded);

// Function to delete a row from CSV data
CSVData deletePointwithIndex(CSVData& data, int index, double time, FullIndex& dbIndex, DeletedIndices& deletedIndices);

// Point to vector.
std::vector<std::string> pointToVector(const Point& point);

// Function to write CSV data to a file ( Complete file rewritten )
void writeCSV(const std::string& filename, const CSVData& csv);

// Function to print CSV data to console
void printCSVData(const CSVData& data);

// File I/O functions
std::ofstream openFileInAppend(const std::string& filename);
bool writeToCSV(const CSVData& csv, std::ofstream& file, NewAdded& newAdded);

// Binary search and time range query functions
void insertIndexSorted(FullIndex& fullIndex, const Index& newIndex);
std::vector<Index>::const_iterator findFirstAfterOrEqualTime(
    const FullIndex& fullIndex, double targetTime);
std::vector<Index> findInTimeRange(
    const FullIndex& fullIndex, double startTime, double endTime);
int getNextRowNumber(const FullIndex& fullIndex);

// Append Only DB Functions
bool writeToCSV(const CSVData& csv, std::ofstream& file, NewAdded& newAdded);

// Main DB Functions.
bool atomicWrite(const std::string& filename, const CSVData& csv, NewAdded& newAdded);
void closeDB(std::ofstream& file);


// Algorithms
std::vector<Index>::const_iterator findFirstAfterOrEqualTime(
    const FullIndex& fullIndex, double targetTime);
std::vector<Index> findInTimeRange(
    const FullIndex& fullIndex, double startTime, double endTime);
int getNextRowNumber(const FullIndex& fullIndex);
