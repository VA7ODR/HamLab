#pragma once

#include "json.hpp"
#include "shared_recursive_mutex.hpp"

#include <string>

namespace HamLab
{
	struct APIVersion {
		constexpr APIVersion(int majorIn, int minorIn, int patchIn) : major(majorIn), minor(minorIn), patch(patchIn) {}

		int major;
		int minor;
		int patch;

		[[nodiscard]] const char* toString() const;
	};

	constexpr APIVersion CURRENT_API_VERSION{0, 0, 1};

	std::ostream & operator<<(std::ostream & os, const APIVersion & version);

	class DataShare
	{
		public:
			explicit DataShare(const std::string & sFileNameIn);
			explicit DataShare(std::string && sFileNameIn);
			~DataShare();

			json::value GetData(const std::string & sKey, const json::value & jDefault);
			json::value GetData(const std::string & sKey);
			void SetData(const std::string & sKey, const json::value & jValue);

		private:
			SharedRecursiveMutex mtx;
			std::string sFileName;
			json::document jData;
	};

	class PluginBase
	{
		public:
			PluginBase(DataShare & data_share_in, const std::string & name_in);
			PluginBase(DataShare & data_share_in, std::string && name_in);
			virtual ~PluginBase();

			constexpr const std::string & Name() { return name; }

			static const APIVersion * Version() { return &CURRENT_API_VERSION; }

			virtual void DrawTab() = 0;
			virtual bool DrawSideBar(bool) = 0;

			virtual bool ShowTab() { return true; }
			virtual bool ShowSideBar() { return true; }

		protected:
			DataShare & data_share_;
			std::string name;
			json::document jLocalData;
	};
} //HamLab
