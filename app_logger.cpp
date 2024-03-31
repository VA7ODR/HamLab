#include "app_logger.hpp"
#include <iostream>

int AppLogger::sync()
{
	LogIt(level, sFile, sFunction, iLine, sMessage);
	sMessage.clear();
	return 0;
}

int AppLogger::overflow(int c)
{
	if (c != EOF) {
		sMessage.push_back(static_cast<char>(c));
	}
	return c;
}

std::streamsize AppLogger::xsputn(const char_type *s, std::streamsize n)
{
	sMessage.append(s, n);
	return n;
}

AppLogger::~AppLogger()
{
	if (sMessage.size() > 0) {
		LogIt(level, sFile, sFunction, iLine, sMessage);
	}
}

std::deque<AppLogger::Entry> AppLogger::GetLogs(LogLevel level, size_t iStart, size_t iEnd)
{
	std::lock_guard<std::mutex> lock(Mutex());

	std::deque<Entry> vRet;
	for (size_t i = iStart; i < Logs().size() && i < iEnd; ++i) {
		auto & entry = Logs()[i];
		if (level == ALL || entry.level <= level) {
			vRet.push_back(entry);
		}
	}
	return vRet;
}

void AppLogger::CloneToCout(bool bClone)
{
	CloneToCout() = bClone;
}

std::mutex &AppLogger::Mutex()
{
	static std::mutex mMutex;
	return mMutex;
}

std::deque<AppLogger::Entry> &AppLogger::Logs()
{
	static std::deque<Entry> vLogs;
	return vLogs;
}

bool &AppLogger::CloneToCout()
{
	static bool bRet = false;
	return bRet;
}

AppLogger::AppLogger(LogLevel levelIn, const std::string &sFileIn, const std::string sFunctionIn, int iLineIn) :
	level(levelIn),
	sFile(sFileIn),
	sFunction(sFunctionIn),
	iLine(iLineIn),
	stream(this)
{
	ASSERT(level >= ERROR && level < ALL);
}

void AppLogger::LogIt(LogLevel level, const std::string &sFile, const std::string sFunction, int iLine, const std::string &sMessage)
{
	ASSERT(level >= ERROR && level < ALL);
	std::lock_guard<std::mutex> lock(Mutex());
	Logs().emplace_back(level, sFile, sFunction, iLine, sMessage);
	if (CloneToCout()) {
		if (level > ERROR) {
			std::cout << Logs().back().ToString() << std::flush;
		} else {
			std::cerr << Logs().back().ToString() << std::flush;
		}
	}
}

AppLogger::Entry::Entry(LogLevel levelIn, const std::string &fileIn, const std::string functionIn, int lineIn, const std::string &messageIn) :
	level(levelIn),
	file(fileIn),
	function(functionIn),
	line(lineIn),
	message(messageIn)
{
}

std::string AppLogger::Entry::ToString() const
{
	std::ostringstream oss;
	oss << std::format("{}| {}  {}:{}:{}", static_cast<char>(level), time, function, file, line);
	size_t start = 0;
	size_t p = message.find('\n', start);
	while (p != std::string::npos) {
		oss << "    " << message.substr(start, p - start) << "\n";
		start = p + 1;
		p = message.find('\n', start);
	}
	if (start < message.size()) {
		oss << "    " << message.substr(start);
		if (message.back() != '\n') {
			oss << "\n";
		}
	}
	return oss.str();
}
