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
		typedef PluginBase*(CreatePluginFunc)(DataShare &, const std::string&);

		PluginLoader(const std::string & pluginDir, DataShare & data_share_in);
		PluginLoader(std::string && pluginDir, DataShare & data_share_in);
		~PluginLoader();

		void CallDrawSideBarFunctions();
		void CallDrawTabFunctions();

		[[nodiscard]] DataShare & data_share() const { return data_share_; }

		void LoadPlugin(PluginBase * pPlugin);

	private:
		void LoadPlugins();
		void UnloadPlugins();
		void LoadPlugin(const std::string &path);

		DataShare &data_share_;
		json::document jLocalData;
		std::string pluginDir_;
		std::map<std::string, bool> all_plugins_do_load_;
		std::map<std::string, APIVersion> all_plugins_versions_;
		std::map<std::string, boost::dll::shared_library> all_plugins_;
		std::map<std::string, PluginBase*> loaded_plugins_;
	};
}
