#pragma once
#ifdef PE_BUILD_DLL
	#define PE_API __declspec(dllexport)
#else
	#define PE_API __declspec(dllimport)
#endif
#include <fwd.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <vec3.hpp>

// Variadic template function
template <typename... Args>
std::string Combine(const Args&... args) {
	std::ostringstream oss;
	// Using a fold expression to append each argument to the stream
	((oss << args), ...);
	return oss.str();
}

inline std::string PositionString(glm::vec3 vector3)
{
	return "x: " + std::to_string(vector3.x) + " y:" + std::to_string(vector3.y) + " z:" + std::to_string(vector3.z);
}


