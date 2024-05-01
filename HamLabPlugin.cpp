#include "HamLabPlugin.hpp"

namespace HamLab
{
	[[nodiscard]] const char* APIVersion::toString() const
	{
		const static std::string sVersion{std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch)};
		return sVersion.c_str();
	}

	std::ostream & operator<<(std::ostream & os, const APIVersion & version)
	{
		os << version.toString();
		return os;
	}

	DataShareClass::DataShareClass(const std::string & sFileNameIn) : sFileName(sFileNameIn)
	{
		RecursiveExclusiveLock lock(mtx);
		jData.parseFile(sFileName);
	}

	DataShareClass::DataShareClass(std::string && sFileNameIn) : sFileName(std::move(sFileNameIn))
	{
		RecursiveExclusiveLock lock(mtx);
		jData.parseFile(sFileName);
	}

	DataShareClass::~DataShareClass()
	{
		RecursiveExclusiveLock lock(mtx);
		jData.writeFile(sFileName, true);
	}

	json::value DataShareClass::GetData(const std::string & sKey, const json::value & jDefault)
	{
		RecursiveExclusiveLock lock(mtx);
		auto & jRet = jData[sKey];
		if (jRet.IsVoid()) {
			jRet = jDefault;
		}
		return jRet;
	}

	json::value DataShareClass::GetData(const std::string & sKey)
	{
		RecursiveExclusiveLock lock(mtx);
		return jData[sKey];
	}

	void DataShareClass::SetData(const std::string & sKey, const json::value & jValue)
	{
		RecursiveExclusiveLock lock(mtx);
		jData[sKey] = jValue;
	}

	PluginBase::PluginBase(DataShareClass & dataShareIn, const std::string & sNameIn) : dataShare(dataShareIn), sName(sNameIn)
	{
		jLocalData = dataShare.GetData(Name());
	}

	PluginBase::PluginBase(DataShareClass & dataShareIn, std::string && sNameIn) : dataShare(dataShareIn), sName(std::move(sNameIn))
	{
		jLocalData = dataShare.GetData(Name());
	}

	PluginBase::~PluginBase()
	{
		dataShare.SetData(Name(), jLocalData);
	}
} // HamLab