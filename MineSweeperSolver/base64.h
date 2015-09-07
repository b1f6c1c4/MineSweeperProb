#pragma once
#include <string>

std::string base64_encode(const std::basic_string<unsigned char> &data);
std::basic_string<unsigned char> base64_decode(std::string const& s);
