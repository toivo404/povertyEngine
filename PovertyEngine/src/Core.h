#pragma once
#ifdef PE_BUILD_DLL
	#define PE_API __declspec(dllexport)
#else
	#define PE_API __declspec(dllimport)
#endif
#include <sstream>
#include <string>

// Variadic template function
template <typename... Args>
std::string Combine(const Args&... args) {
	std::ostringstream oss;
	// Using a fold expression to append each argument to the stream
	((oss << args), ...);
	return oss.str();
}

