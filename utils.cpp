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


std::string GetAppDataFolder()
{
#if defined(_WIN32)
	const char *appDataEnvVar = std::getenv("APPDATA");
	if (appDataEnvVar)
		return appDataEnvVar;
	else
		return ""; // Handle error on Windows
#elif defined(__APPLE__)
	const char *homeDir = std::getenv("HOME");
	if (homeDir)
		return std::string(homeDir) + "/Library/Application Support";
	else
		return ""; // Handle error on macOS
#else
	const char *homeDir = std::getenv("HOME");
	if (homeDir)
		return std::string(homeDir) + "/.local/share";
	else
		return ""; // Handle error on Linux
#endif
}
