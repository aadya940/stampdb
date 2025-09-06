#include "../include/internal/csvparse.hpp"
#include "../include/internal/fileio.hpp"


// Shadow Copy + Atomic Swap.
bool atomicWrite(const std::string& filename, const CSVData& csv, NewAdded& newAdded) {
    createShadowCopy(filename);   // Now a shadow copy of the csv file is created.
    std::string copyFile = filename + ".tmp"; 
    std::ofstream fp = openFileInAppend(copyFile); // Now the file is open in append Mode.

    writeToCSV(csv, fp, newAdded);
    
    swapShadowAsDb(filename);
    
    closeDB(fp);
    return true;
}

void closeDB(std::ofstream& file) {
    file.close();
}


