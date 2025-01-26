#pragma once
#include <regex>
#include <string>
#include <vector>
#include "systems/TransformSystem.h"

// Function to parse the file and return a list of pairs of name and Transform
std::vector<std::pair<std::string, Transform>> parseLevelFile(const std::string& filePath);

