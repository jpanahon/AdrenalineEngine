/*
    tools.h
    Adrenaline Engine

    Holds all the tools for the engine.
*/

#pragma once

#include <iostream>
#include <GLFW/glfw3.h>

#include <vector>
#include <fstream>
#include <set>
#include "types.h"
#include "vk_mem_alloc.h"
#include <regex>

namespace Adren::Tools {
inline std::string formatPath(std::string& path) {
    std::string newPath = std::regex_replace(path, std::regex("\\"), "/");

    return newPath;
}

}