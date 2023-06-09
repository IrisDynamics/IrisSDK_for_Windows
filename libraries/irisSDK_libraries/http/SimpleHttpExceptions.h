#pragma once

#include <winhttp.h>
#include <windows.h>

void freak_out() {
	DWORD word_bro = GetLastError();
	std::cout << "Error: " << word_bro << std::endl;
	throw std::runtime_error("Error occurred: " + word_bro);
}