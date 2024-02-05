#pragma once

#include <vector>
#include <string>

double clamp(double, double, double);

std::vector<std::string> SplitString(const std::string&, int);

unsigned char HexToChar(const std::string&);
Color HexCode(std::string);