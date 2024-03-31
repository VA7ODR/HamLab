#pragma once

#include "app_logger.hpp"
#include <map>
#include <set>
#include <thread>
#include <memory>

#if defined _WINDOWS

typedef HANDLE thread_id_t;
#define get_thread_id GetCurrentThreadId;

#else

#include <unistd.h>

typedef __pid_t thread_id_t;
#define get_thread_id gettid

#endif

class Thread : public std::enable_shared_from_this<Thread>
{
	public:
		Thread(const std::string & sFileIn, const std::string & sFunctionIn, int iLineIn, const std::string & sNameIn) :
			sFile(sFileIn),
			sFunction(sFunctionIn),
			iLine(iLineIn),
			sName(sNameIn)
		{

		}

		std::shared_ptr<Thread> start(auto __f)
		{
			parent_id = get_thread_id();
			auto self = shared_from_this();
			auto runner = [self](std::stop_token stoken, auto __f) {
				{
					std::lock_guard<std::mutex> lock(m_mutex());
					self->id = get_thread_id();
					m_threads()[self->id] = self.get();
					if (m_threads().find(self->parent_id) != m_threads().end()) {
						m_threads()[self->parent_id]->children.insert(self->id);
					} else {
						main_thread_children().insert(self->id);
					}
				}
				__f();
				{
					std::lock_guard<std::mutex> lock(m_mutex());
					m_threads().erase(self->id);
					if (m_threads().find(self->parent_id) != m_threads().end()) {
						m_threads()[self->parent_id]->children.erase(self->id);
					} else {
						main_thread_children().erase(self->id);
					}
				}
			};
			m_thread = std::jthread(runner, __f);
			return self;
		}

		std::jthread & get_thread();

		static thread_id_t main_thread_id();

		struct MapItem
		{
			std::string sFile;
			std::string sFunction;
			int iLine = 0;
			std::string sName;
			thread_id_t id = 0;
			thread_id_t parent_id = 0;
			std::set<MapItem> children;

			MapItem();

			MapItem(const Thread & thread);

			bool operator<(const MapItem & other) const;
		};

		static MapItem map();
		static void log_map(AppLogger::LogLevel levelIn, const std::string &sFileIn, const std::string sFunctionIn, int iLineIn);

	private:
		std::string sName;
		std::string sFile;
		std::string sFunction;
		int iLine = 0;
		thread_id_t id = 0;
		thread_id_t parent_id = 0;
		std::set<thread_id_t> children;
		std::jthread m_thread;

		static std::mutex & m_mutex();

		static std::set<thread_id_t> & main_thread_children();

		static std::map<thread_id_t, Thread*> & m_threads();
};

#define THREAD(sName, func, ...) std::make_shared<Thread>(SOURCE_FILE, __FUNCTION__, __LINE__, sName)->start(std::bind(func, __VA_ARGS__))
#define LOG_THREAD_MAP(level) Thread::log_map(level, SOURCE_FILE, __FUNCTION__, __LINE__)

