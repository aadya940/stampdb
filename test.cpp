#include <iostream>
#include <iomanip>
#include <string>
#include <filesystem>
#include <cassert>

#include "include/stampdb.hpp"

using namespace std;
namespace fs = std::filesystem;

int main() {
    try {
        // Clean up any existing test files
        fs::remove("test_db.csv");
        fs::remove("test_db.csv.tmp");
        
        // Create a test CSV file
        {
            ofstream testFile("test_db.csv");
            testFile << "time, id, name, value, active\n";
            testFile << "1.0, 1001, Item 1, 10.5, true\n";
            testFile << "2.0, 1002, Item 2, 20.5, false\n";
            testFile << "3.0, 1003, Item 3, 30.5, true\n";
        }
        
        cout << "=== Testing StampDB ===\n";
        
        // Test 1: Initialize database
        cout << "\n[Test 1] Initializing database...\n";
        StampDB db("test_db.csv");
        cout << "Database initialized successfully.\n";
        
        // Test 2: Read existing data
        cout << "\n[Test 2] Reading existing data...\n";
        auto allData = db.read_range(0, 10);
        cout << "Found " << allData.points.size() << " data points.\n";
        printCSVData(allData);
        
        // Test 3: Read single point
        cout << "[Test 3] Reading single point (time=2.0)...\n";
        auto point = db.read(2.0);

        if (!point.points.empty()) {
            const PointRow& row = point.points.at(0).rows.at(0);
            cout << "Found point at time 2.0:\n";
            
            std::cout << "Row of the Point: ";
            std::visit([](const auto& value){
                std::cout << value;
            }, row.data);
            std::cout << std::endl;

        } else {
            cout << "No point found at time 2.0\n";
        }
        
        // Test 4: Add new point
        cout << "\n[Test 4] Adding new point...\n";
        Point newPoint;
        newPoint.time = 4.0;
        
        // Add each field as a separate PointRow
        newPoint.rows.push_back({1004});    // id
        newPoint.rows.push_back({"Item 4"}); // name
        newPoint.rows.push_back({40.5});    // value
        newPoint.rows.push_back({true});    // active
        
        db.appendPoint(newPoint);
        cout << "\n New Point Appended. Now starting compaction." << endl;
        db.compact();

        cout << "New point added. Current data:\n";
        printCSVData(db.read_range(1, 4));
        
        // Test 5: Delete point
        cout << "\n[Test 5] Deleting point at time=2.0...\n";
        db.delete_point(2.0);
        db.compact();

        cout << "After deletion. Current data:\n";
        printCSVData(db.read_range(0, 10));
        
        // Test 6: Force a checkpoint
        cout << "\n[Test 6] Forcing checkpoint...\n";
        db.compact();
        cout << "Checkpoint completed.\n";
        
        // Test 7: Test persistence by creating a new instance
        cout << "\n[Test 7] Testing persistence...\n";
        {
            StampDB db2("test_db.csv");
            cout << "Data after reloading from disk:\n";
            printCSVData(db2.read_range(0, 10));
        }
        
        // Test 8: Compact the database
        cout << "\n[Test 8] Compacting database...\n";
        db.compact();
        cout << "Compaction completed.\n";
        
        // Clean up
        db.close();
        
        cout << "\n=== All tests completed successfully! ===\n";
        
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}
