#pragma once

#include <map>
#include <string>
#include <filesystem>
#include <iostream>
#include <boost/dll.hpp>

#include "HamLabPlugin.hpp"
#include "imgui.h"

namespace HamLab
{
	class PluginLoader
	{
	public:
		typedef PluginBase*(CreatePluginFunc)(DataShare &, const std::string&);
		PluginLoader(const std::string &pluginDir, DataShare & data_share_in)
			: pluginDir_(pluginDir),
			  data_share_(data_share_in)
		{
			jLocalData = data_share_.GetData("PluginLoader");
			LoadPlugins();
		}

		~PluginLoader()
		{
			UnloadPlugins();
			data_share_.SetData("PluginLoader", jLocalData);
		}

		void CallDrawSideBarFunctions()
		{
			for (auto &[name, plugin] : loaded_plugins_)
			{
				if (plugin) {
					if (plugin->ShowSideBar()) {
						jLocalData["plugin"][name]["sidebar_open"] = plugin->DrawSideBar(jLocalData["plugin"][name]["sidebar_open"].boolean());
					}
				}
			}

			if (ImGui::CollapsingHeader("Plugins", jLocalData["plugins_open"].boolean() ? ImGuiTreeNodeFlags_DefaultOpen : 0)) {

				jLocalData["plugins_open"] = true;

				for (auto & do_plugin : all_plugins_do_load_) {
					auto & plugin = loaded_plugins_[do_plugin.first];
					if (ImGui::Checkbox(do_plugin.first.c_str(), &do_plugin.second)) {
						if (do_plugin.second) {
							auto & lib = all_plugins_[do_plugin.first];
							if (!plugin) {
								std::function<CreatePluginFunc> createPlugin = lib.get_alias<CreatePluginFunc>("CreatePlugin");

								if (!createPlugin)
								{
									std::cerr << "Failed to find CreatePlugin function in plugin: " << do_plugin.first << std::endl;
									do_plugin.second = false;
									continue;
								}

								plugin = createPlugin(data_share_, do_plugin.first);
							}
						} else {
							delete plugin;
							plugin = nullptr;
						}
						jLocalData["loaded_plugins"][do_plugin.first] = (plugin != nullptr);
					}
				}
			} else {
				jLocalData["plugins_open"] = false;
			}
		}

		void CallDrawTabFunctions()
		{
			for (auto &[name, plugin] : loaded_plugins_)
			{
				if (plugin) {
					if (plugin->ShowTab()) {
						plugin->DrawTab();
					}
				}
			}
		}

	private:
		void LoadPlugins()
		{
			std::filesystem::path pluginPath(pluginDir_);

			if (!std::filesystem::exists(pluginPath) || !std::filesystem::is_directory(pluginPath))
			{
				std::cerr << "Invalid plugin directory: " << pluginDir_ << std::endl;
				return;
			}

			for (const auto &entry : std::filesystem::directory_iterator(pluginPath))
			{
				if (entry.is_regular_file() && entry.path().extension() == ".so")
				{
					LoadPlugin(entry.path());
				}
			}
		}

		void UnloadPlugins()
		{
			while (loaded_plugins_.size()) {
				auto it = loaded_plugins_.begin();
				if (it->second) {
					delete it->second;
				}
				loaded_plugins_.erase(it);
			}
			all_plugins_versions_.clear();
			all_plugins_do_load_.clear();
			all_plugins_.clear();
		}

		void LoadPlugin(const std::string &path)
		{
			boost::dll::shared_library lib(path, boost::dll::load_mode::append_decorations);

			if (!lib.is_loaded())
			{
				std::cerr << "Failed to load plugin: " << path << std::endl;
				return;
			}

			typedef const char *(PluginNameFunc)(void);
			typedef const APIVersion *(PluginVersionFunc)(void);

			std::function<CreatePluginFunc> createPlugin = lib.get_alias<CreatePluginFunc>("CreatePlugin");

			if (!lib.has("CreatePlugin")) {
				std::cerr << "Failed to find CreatePlugin function in plugin: " << path << std::endl;
				return;
			}

			std::function<PluginNameFunc> pluginName = lib.get_alias<PluginNameFunc>("Name");
			if (!pluginName)
			{
				std::cerr << "Failed to find Name function in plugin: " << path << std::endl;
				return;
			}

			std::function<PluginVersionFunc> pluginVersion = lib.get_alias<PluginVersionFunc>("Version");

			if (!pluginVersion)
			{
				std::cerr << "Failed to find Version function in plugin: " << path << std::endl;
				return;
			}

			std::string name = pluginName();
			APIVersion version = *pluginVersion();

			std::cout << "Discovered plugin: " << name << ": API Version: " << version << std::endl;
			bool bLoad = jLocalData["loaded_plugins"][name].boolean();
			all_plugins_[name] = lib;
			all_plugins_versions_.insert({name, version});
			all_plugins_do_load_[name] = bLoad;
			if (bLoad) {
				loaded_plugins_[name] = createPlugin(data_share_, name);
			} else {
				loaded_plugins_[name] = nullptr;
			}
		}

	private:
		DataShare &data_share_;
		json::document jLocalData;
		std::string pluginDir_;
		std::map<std::string, bool> all_plugins_do_load_;
		std::map<std::string, APIVersion> all_plugins_versions_;
		std::map<std::string, boost::dll::shared_library> all_plugins_;
		std::map<std::string, PluginBase*> loaded_plugins_;
	};
}
