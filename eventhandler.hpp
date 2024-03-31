#pragma once
#include <chrono>
#include <memory>
#include <vector>

namespace EventHandler
{
	enum event_type
	{
		manual_reset,
		auto_reset
	};

	class EventBase
	{
		public:
			EventBase(const std::string & sNameIn, event_type eTypeIn) : sName(sNameIn), eType(eTypeIn) {}

			void Set();
			void Reset();

			const std::string & Name();

		protected:
			friend class CleanupAfterWait;
			friend class Map;
			friend int Wait(const std::string & sFile, const std::string & sFunc, int iLine, std::vector<std::shared_ptr<EventBase>>, std::chrono::milliseconds);
			void Waiting();
			void AutoReset();
			std::string sName;
			size_t waitCount = 0;
			event_type eType = manual_reset;
			bool bValue = false;
	};

	using Event = std::shared_ptr<EventBase>;

	const std::chrono::milliseconds INFINITE = std::chrono::milliseconds::max();

	int Wait(const std::string & sFile, const std::string & sFunc, int iLine, std::vector<Event> vEvents, std::chrono::milliseconds timeout = std::chrono::milliseconds::max());

	void Set(Event e);

	void Reset(Event e);

	Event CreateEvent(const std::string & sName, event_type eType = manual_reset);

	void ExitAll();

	const int TIMEOUT = -1;
	const int EXIT_ALL = -2;

} // EventHandler

#define EventHandlerWait(...) EventHandler::Wait(SOURCE_FILE, __func__, __LINE__, __VA_ARGS__)
