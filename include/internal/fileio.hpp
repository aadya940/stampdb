#pragma once

#include <string>
#include <filesystem>
#include <iostream>
#include <thread>
#include <chrono>


// Shadow Copy and Atomic Renames
bool createShadowCopy(const std::string& path);
bool swapShadowAsDb(const std::string& path, int maxRetries = 5);
