#include "utils.hpp"

std::string Pretty(const std::vector<unsigned char> &in)
{
	std::string sRet;
	sRet.reserve(in.size() * 8);
	for (auto & c : in) {
		if (c < ' ' || c >= 127) {
			char szHex[24];
			sprintf(szHex, "[%.2X]", c);
			sRet.append(szHex);
		} else {
			sRet.push_back((char)c);
		}
	}
	return sRet;
}

