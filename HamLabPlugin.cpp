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

	DataShare::DataShare(const std::string & sFileNameIn) : sFileName(sFileNameIn)
	{
		RecursiveExclusiveLock lock(mtx);
		jData.parseFile(sFileName);
	}

	DataShare::DataShare(std::string && sFileNameIn) : sFileName(std::move(sFileNameIn))
	{
		RecursiveExclusiveLock lock(mtx);
		jData.parseFile(sFileName);
	}

	DataShare::~DataShare()
	{
		RecursiveExclusiveLock lock(mtx);
		jData.writeFile(sFileName, true);
	}

	json::value DataShare::GetData(const std::string & sKey, const json::value & jDefault)
	{
		RecursiveExclusiveLock lock(mtx);
		auto & jRet = jData[sKey];
		if (jRet.IsVoid()) {
			jRet = jDefault;
		}
		return jRet;
	}
	json::value DataShare::GetData(const std::string & sKey)
	{
		RecursiveExclusiveLock lock(mtx);
		return jData[sKey];
	}

	void DataShare::SetData(const std::string & sKey, const json::value & jValue)
	{
		RecursiveExclusiveLock lock(mtx);
		jData[sKey] = jValue;
	}

	PluginBase::PluginBase(DataShare & data_share_in, const std::string & name_in) : data_share_(data_share_in), name(name_in)
	{
		jLocalData = data_share_.GetData(Name());
	}

	PluginBase::PluginBase(DataShare & data_share_in, std::string && name_in) : data_share_(data_share_in), name(std::move(name_in))
	{
		jLocalData = data_share_.GetData(Name());
	}

	PluginBase::~PluginBase()
	{
		data_share_.SetData(Name(), jLocalData);
	}
} // HamLab