#pragma once

#include <deque>
#include <mutex>
#include <streambuf>
#include <string>

#include "utils.hpp"

class AppLogger  : public std::streambuf
{
	public:
		enum LogLevel : char
		{
			ERROR = '0',
			WARNING,
			INFO,
			DEBUG,
			TRACE,
			ALL, // Don't use for logging, this is for asking for logs in return functions.
		};

		enum LogEntry : size_t
		{
			START = 0,
			END = static_cast<size_t>(-1),
		};

		struct Entry
		{
				LogLevel level = ERROR;
				std::string file;
				std::string function;
				std::string time = PrettyDateTime(SystemNow());
				int line = 0;
				std::string message;

				Entry() = default;
				Entry(LogLevel levelIn, const std::string & fileIn, const std::string functionIn, int lineIn, const std::string & messageIn);

				std::string ToString() const;
		};

		AppLogger(LogLevel levelIn, const std::string & sFileIn, const std::string sFunctionIn, int iLineIn);
		~AppLogger();

		int sync() override;
		int overflow(int c) override;
		std::streamsize xsputn(const char_type* s, std::streamsize n) override;

		template <typename T>
		std::ostream& operator<<(const T& value) {
			stream << value;
			return stream;
		}

		static std::deque<Entry> GetLogs(LogLevel level = ALL, size_t iStart = START, size_t iEnd = END);

		static void CloneToCout(bool bClone);
		static bool & CloneToCout();

	protected:
		static void LogIt(LogLevel level, const std::string & sFile, const std::string sFunction, int iLine, const std::string & sMessage);

		LogLevel level;
		std::string sFile;
		std::string sFunction;
		int iLine;
		std::string sMessage;
		std::ostream stream;
		static std::mutex & Mutex();
		static std::deque<Entry> & Logs();
};

#define Log(L) AppLogger(L, SOURCE_FILE, __FUNCTION__, __LINE__)
