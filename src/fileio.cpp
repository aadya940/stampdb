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

bool swapShadowAsDb(const std::string& path, int maxRetries) {
    fs::path original(path);
    fs::path shadow(path + ".tmp");

    for (int attempt = 1; attempt <= maxRetries; ++attempt) {
        try {
            fs::rename(shadow, original);
            return true;
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Attempt " << attempt << " failed: " << e.what() << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    std::cerr << "Failed to swap shadow as db after " << maxRetries << " attempts.\n";
    return false;
}

