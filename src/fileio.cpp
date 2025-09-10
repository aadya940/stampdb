#include "../include/internal/fileio.hpp"

namespace fs = std::filesystem;

bool createShadowCopy(const std::string& path) {
    std::string shadowStr = path + ".tmp";
    fs::path original(path);
    fs::path shadow(shadowStr);
    
    try {
        // Copy with overwrite if exists
        fs::copy_file(original, shadow, fs::copy_options::overwrite_existing);
        return true;
    } catch (const fs::filesystem_error& e) {
        return false;
    }
}

// This will be called during the checkpointing process (after say writing 50 rows or make it configurable?).
bool swapShadowAsDb(const std::string& path) {
    std::string shadowStr = path + ".tmp";
    fs::path original(path);
    fs::path shadow(shadowStr);
    try {
        fs::rename(shadow, original);
        return true;
    } catch (const fs::filesystem_error& e) {
        return false;
    }
}
