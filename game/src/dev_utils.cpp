#include <vector>
#include <string>
#include <sstream>
#include "raylib.h"

double clamp(double d, double min, double max) {
	const double t = d < min ? min : d;
	return t > max ? max : t;
}

std::vector<std::string> SplitString(const std::string& str, int splitLength)
{
	int NumSubstrings = str.length() / splitLength;
	std::vector<std::string> ret;

	for (auto i = 0; i < NumSubstrings; i++)
	{
		ret.push_back(str.substr(i * splitLength, splitLength));
	}

	// If there are leftover characters, create a shorter item at the end.
	if (str.length() % splitLength != 0)
	{
		ret.push_back(str.substr(splitLength * NumSubstrings));
	}


	return ret;
}

unsigned char HexToChar(const std::string& hex) {
	int returnValue;
	std::stringstream ss;
	ss << std::hex << hex;
	ss >> returnValue;
	return returnValue;
}

Color HexCode(std::string hexCode) {
	hexCode.erase(hexCode.begin());
	std::vector splitString = SplitString(hexCode, 2);
	if (splitString.size() == 3) {
		return {HexToChar(splitString[0]), HexToChar(splitString[1]), HexToChar(splitString[2]), 0xff};
	}
	else if (splitString.size() == 4) {
		return { HexToChar(splitString[0]), HexToChar(splitString[1]), HexToChar(splitString[2]), HexToChar(splitString[3])};
	}
	else {
		return { 0x00, 0x00, 0x00, 0x00 };
	}
}