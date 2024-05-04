#include "PluginLoader.hpp"

#include <ranges>

#include "imgui.h"

namespace HamLab
{
	PluginLoader::PluginLoader(const std::string & pluginDir, DataShareClass & data_share_in)
				: dataShare(data_share_in),
				  pluginDir(pluginDir)
	{
		jLocalData = dataShare.GetData("PluginLoader");
		LoadPlugins();
	}

	PluginLoader::PluginLoader(std::string && pluginDir, DataShareClass & data_share_in)
			: dataShare(data_share_in),
			  pluginDir(std::move(pluginDir))
	{
		jLocalData = dataShare.GetData("PluginLoader");
		LoadPlugins();
	}

	PluginLoader::~PluginLoader()
	{
		UnloadPlugins();
		dataShare.SetData("PluginLoader", jLocalData);
	}

	void PluginLoader::CallDrawSideBarFunctions()
	{
		for (auto &[name, plugin] : loadedPlugins)
		{
			if (plugin) {
				if (plugin->ShowSideBar()) {
					bool bOpen = plugin->DrawSideBar(jLocalData["plugin"][name]["sidebar_open"].boolean());
					jLocalData["plugin"][name]["sidebar_open"] = bOpen;
					if (bOpen) {
						ImGui::Text(" ");
					}
				}
			}
		}

		if (ImGui::CollapsingHeader("Plugins", jLocalData["plugins_open"].boolean() ? ImGuiTreeNodeFlags_DefaultOpen : 0)) {

			jLocalData["plugins_open"] = true;

			for (auto & do_plugin : allPluginsDoLoad) {
				if (allPlugins.contains(do_plugin.first) && ImGui::Checkbox(do_plugin.first.c_str(), &do_plugin.second)) {
					auto & plugin = loadedPlugins[do_plugin.first];
					if (do_plugin.second) {
						auto & lib = allPlugins[do_plugin.first];
						if (!plugin) {
							std::function<CreatePluginFunc> createPlugin = lib.get_alias<CreatePluginFunc>("CreatePlugin");

							if (!createPlugin)
							{
								std::cerr << "Failed to find CreatePlugin function in plugin: " << do_plugin.first << std::endl;
								do_plugin.second = false;
								continue;
							}

							plugin = createPlugin(dataShare, do_plugin.first);
						}
					} else {
						delete plugin;
						plugin = nullptr;
					}
					jLocalData["loaded_plugins"][do_plugin.first] = (plugin != nullptr);
				}
			}
			ImGui::Text(" ");
		} else {
			jLocalData["plugins_open"] = false;
		}
	}

	void PluginLoader::CallDrawTabFunctions()
	{
		for (auto &plugin : loadedPlugins | std::views::values)
		{
			if (plugin) {
				if (plugin->ShowTab()) {
					plugin->DrawTab();
				}
			}
		}
	}

	void PluginLoader::LoadPlugin(PluginBase * pPlugin)
	{
		loadedPlugins[pPlugin->Name()] = pPlugin;
		auto version = *HamLab::PluginBase::Version();
		allPluginsVersions.insert({pPlugin->Name(), version});
	}

	void PluginLoader::LoadPlugins()
	{
		std::filesystem::path pluginPath(pluginDir);

		if (!std::filesystem::exists(pluginPath) || !std::filesystem::is_directory(pluginPath))
		{
			std::cerr << "Invalid plugin directory: " << pluginDir << std::endl;
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

	void PluginLoader::UnloadPlugins()
	{
		while (!loadedPlugins.empty()) {
			auto it = loadedPlugins.begin();

			delete it->second;

			loadedPlugins.erase(it);
		}
		allPluginsVersions.clear();
		allPluginsDoLoad.clear();
		allPlugins.clear();
	}

	void PluginLoader::LoadPlugin(const std::string & sPath)
	{
		boost::dll::shared_library lib(sPath, boost::dll::load_mode::append_decorations);

		if (!lib.is_loaded())
		{
			std::cerr << "Failed to load plugin: " << sPath << std::endl;
			return;
		}

		typedef const char *(PluginNameFunc)();
		typedef const APIVersion *(PluginVersionFunc)();

		std::function<CreatePluginFunc> createPlugin = lib.get_alias<CreatePluginFunc>("CreatePlugin");

		if (!lib.has("CreatePlugin")) {
			std::cerr << "Failed to find CreatePlugin function in plugin: " << sPath << std::endl;
			return;
		}

		std::function<PluginNameFunc> pluginName = lib.get_alias<PluginNameFunc>("Name");
		if (!pluginName)
		{
			std::cerr << "Failed to find Name function in plugin: " << sPath << std::endl;
			return;
		}

		std::function<PluginVersionFunc> pluginVersion = lib.get_alias<PluginVersionFunc>("Version");

		if (!pluginVersion)
		{
			std::cerr << "Failed to find Version function in plugin: " << sPath << std::endl;
			return;
		}

		std::string name = pluginName();
		APIVersion version = *pluginVersion();

		std::cout << "Discovered plugin: " << name << ": API Version: " << version << std::endl;
		bool bLoad = jLocalData["loaded_plugins"][name].boolean();
		allPlugins[name] = lib;
		allPluginsVersions.insert({name, version});
		allPluginsDoLoad[name] = bLoad;
		if (bLoad) {
			loadedPlugins[name] = createPlugin(dataShare, name);
		} else {
			loadedPlugins[name] = nullptr;
		}
	}
} // HamLab