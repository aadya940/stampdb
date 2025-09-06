#pragma once

#include <string>
#include <filesystem>
#include <iostream>

// Shadow Copy and Atomic Renames
bool createShadowCopy(const std::string& path);
bool swapShadowAsDb(const std::string& path);
