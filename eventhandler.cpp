#include "eventhandler.hpp"
#include <condition_variable>
#include <map>

namespace EventHandler
{

	static std::mutex &mtx()
	{
		static std::mutex ret;
		return ret;
	}

	static std::condition_variable & cv()
	{
		static std::condition_variable ret;
		return ret;
	}

	void EventBase::Set()
	{
		bValue = true;
		cv().notify_all();
	}

	void EventBase::Reset()
	{
		if (eType == auto_reset) {
			return;
		}
		std::unique_lock<std::mutex> lck(mtx());
		bValue = false;
	}

	const std::string &EventBase::Name()
	{
		return sName;
	}

	void EventBase::Waiting()
	{
		if (eType == auto_reset) {
			waitCount++;
		}
	}

	void EventBase::AutoReset()
	{
		if (eType == auto_reset) {
			if (waitCount && --waitCount == 0) {
				bValue = false;
			}
		}
	}

	class CleanupAfterWait
	{
		public:
			CleanupAfterWait(std::vector<Event> & vEventsIn) : vEvents(vEventsIn)
			{
				for (auto & event : vEvents) {
					event->Waiting();
				}
			}
			~CleanupAfterWait()
			{
				for (auto & event : vEvents) {
					event->AutoReset();
				}
			}
		private:
			std::vector<Event> vEvents;
	};

	static Event & ExitEvent()
	{
		static Event ret = std::make_shared<EventBase>("ExitEvent", manual_reset);
		return ret;
	}

	class Map
	{
		public:
			Map(const std::string & sFile, const std::string & sFunc, int iLine, std::vector<Event> & vEventsIn) :
				sFile(sFile),
				sFunc(sFunc),
				iLine(iLine),
				vEvents(vEventsIn)
			{
				{
					std::unique_lock<std::mutex> lck(mtx());
					static size_t nextIndex = 0;
					index = nextIndex++;
					map()[sFile][sFunc][iLine][index] = this;
				}
				// std::cout << "Waiting for " << sFile << ":" << iLine << " (" << sFunc << ") " << Map::Status() << std::endl;
			}

			~Map()
			{
				{
					std::unique_lock<std::mutex> lck(mtx());
					map()[sFile][sFunc][iLine].erase(index);
				}
				// std::cout << "Done waiting for " << sFile << ":" << iLine << " (" << sFunc << ") " << Map::Status() << std::endl;
			}

			std::string MyStatus()
			{
				std::string ret("{");
				bool bStart = true;
				for (auto & event : vEvents) {
					if (bStart) {
						bStart = false;
					} else {
						ret += ", ";
					}
					ret += event->Name() + ": " + (event->bValue ? "true" : "false");
				}
				ret.push_back('}');
				return ret;
			}

			static std::string Status();

			static std::mutex & mtx();
			static std::map<std::string, std::map<std::string, std::map<int, std::map<size_t, Map*>>>> & map();

			std::string sFile;
			std::string sFunc;
			int iLine;
			size_t index;
			std::vector<Event> & vEvents;
	};

	std::mutex & Map::mtx()
	{
		static std::mutex ret;
		return ret;
	}

	std::map<std::string, std::map<std::string, std::map<int, std::map<size_t, Map*>>>> & Map::map()
	{
		static std::map<std::string, std::map<std::string, std::map<int, std::map<size_t, Map*>>>> ret;
		return ret;
	}

	std::string Map::Status()
	{
		std::unique_lock<std::mutex> lck(mtx());
		std::string ret;
		bool bStart = true;
		for (auto & file : map()) {
			for (auto & func : file.second) {
				for (auto & line : func.second) {
					bool bStart = true;
					std::string sHeader = file.first + ":" + std::to_string(line.first) + " (" + func.first + ")";
					for (auto & index : line.second) {
						if (bStart) {
							bStart = false;
							ret += sHeader;
						} else {
							ret.append(sHeader.size(), ' ');
						}
						std::string sIndex = std::to_string(index.first);
						ret.append(32 - sIndex.size(), ' ');
						ret += sIndex + " -> " + index.second->MyStatus() + "\n";
					}
				}
			}
		}
		return ret;
	}

	int Wait(const std::string & sFile, const std::string & sFunc, int iLine, std::vector<Event> vEvents, std::chrono::milliseconds timeout)
	{
		Map map(sFile, sFunc, iLine, vEvents);
		std::unique_lock<std::mutex> lck(mtx());
		while (ExitEvent()->bValue == false) {
			CleanupAfterWait cleanup(vEvents);
			size_t i = 0;
			for (auto & e : vEvents) {
				if (e->bValue) {
					return i;
				}
			}
			if (timeout == std::chrono::milliseconds::max()) {
				cv().wait(lck, [&vEvents] { if (ExitEvent()->bValue) return true; for (auto & e : vEvents) { if (e->bValue) { return true; } } return false; });
			} else {
				bool bRet = cv().wait_for(lck, timeout, [&vEvents] { if (ExitEvent()->bValue) return true; for (auto & e : vEvents) { if (e->bValue) { return true; } } return false; });
				if (!bRet) {
					return TIMEOUT;
				}
			}
			if (ExitEvent()->bValue) {
				return EXIT_ALL;
			}
			for (auto & e : vEvents) {
				if (e->bValue) {
					return i;
				}
			}
		}
		return EXIT_ALL;
	}

	void Set(Event e)
	{
		e->Set();
	}

	void Reset(Event e)
	{
		e->Reset();
	}

	Event CreateEvent(const std::string &sName, event_type eType)
	{
		return std::make_shared<EventBase>(sName, eType);
	}

	void ExitAll()
	{
		ExitEvent()->Set();
	}
} // EventHandler
