// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include "framework.h"

// Our stuff

#include <Windows.h>
#include <iostream>
#include <iomanip>
#include <string>       // for std::string, std::stoi
#include <sstream>      // for std::stringstream, std::getline
#include <vector>       // for std::vector
#include <algorithm>    // for std::search, std::equal
#include <psapi.h>      // for MODULEINFO, GetModuleInformation
#include <intrin.h>		// editing registries
#include <functional>	// IAT hooking

#endif //PCH_H
