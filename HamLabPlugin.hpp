#pragma once

#include "json.hpp"
#include "shared_recursive_mutex.hpp"
#include <string>

namespace HamLab
{

	struct APIVersion {
		constexpr APIVersion(int major, int minor, int patch) : major(major), minor(minor), patch(patch) {}

		int major;
		int minor;
		int patch;

		const char* toString() const {
			const static std::string sVersion{std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch)};
			return sVersion.c_str();
		}
	};

	constexpr APIVersion CURRENT_API_VERSION{1, 2, 3};

	std::ostream &operator<<(std::ostream &os, const APIVersion &version)
	{
		os << version.toString();
		return os;
	}


class DataShare
{
	public:
		DataShare(const std::string & sFileNameIn) : sFileName(sFileNameIn)
		{
			RecursiveExclusiveLock lock(mtx);
			jData.parseFile(sFileName);
		}

		~DataShare()
		{
			jData.writeFile(sFileName, true);
		}

		json::value GetData(const std::string & sKey, json::value jDefault = json::value())
		{
			RecursiveExclusiveLock lock(mtx);
			auto & jRet = jData[sKey];
			if (jRet.IsVoid()) {
				jRet = jDefault;
			}
			return jRet;
		}

		void SetData(const std::string & sKey, json::value & jValue)
		{
			RecursiveExclusiveLock lock(mtx);
			jData[sKey] = jValue;
		}

	private:
		SharedRecursiveMutex mtx;
		std::string sFileName;
		json::document jData;
};

class PluginBase
{
	public:
		PluginBase(DataShare & data_share_in, const std::string & name_in) : data_share_(data_share_in), name(name_in)
		{
			jLocalData = data_share_.GetData(Name());
		}

		virtual ~PluginBase()
		{
			data_share_.SetData(Name(), jLocalData);
		}

		constexpr const std::string & Name() { return name; }

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
