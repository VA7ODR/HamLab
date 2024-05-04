#pragma once

#include <boost/dll.hpp>
#include <filesystem>
#include <map>
#include <string>

#include "HamLabPlugin.hpp"

namespace HamLab
{
	class PluginLoader
	{
	public:
		typedef PluginBase*(CreatePluginFunc)(DataShareClass &, const std::string&);

		PluginLoader(const std::string & pluginDir, DataShareClass & data_share_in);
		PluginLoader(std::string && pluginDir, DataShareClass & data_share_in);
		~PluginLoader();

		void CallDrawSideBarFunctions();
		void CallDrawTabFunctions();

		[[nodiscard]] DataShareClass & DataShare() const { return dataShare; }

		void LoadPlugin(PluginBase * pPlugin);

	private:
		void LoadPlugins();
		void UnloadPlugins();
		void LoadPlugin(const std::string & Sath);

		DataShareClass &dataShare;
		json::document jLocalData;
		std::string pluginDir;
		std::map<std::string, bool> allPluginsDoLoad;
		std::map<std::string, APIVersion> allPluginsVersions;
		std::map<std::string, boost::dll::shared_library> allPlugins;
		std::map<std::string, PluginBase*> loadedPlugins;
	};
}
