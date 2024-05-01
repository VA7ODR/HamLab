#include "PluginLoader.hpp"

#include "imgui.h"

namespace HamLab
{
	PluginLoader::PluginLoader(const std::string & pluginDir, DataShare & data_share_in)
				: data_share_(data_share_in),
				  pluginDir_(pluginDir)
	{
		jLocalData = data_share_.GetData("PluginLoader");
		LoadPlugins();
	}

	PluginLoader::PluginLoader(std::string && pluginDir, DataShare & data_share_in)
			: data_share_(data_share_in),
			  pluginDir_(std::move(pluginDir))
	{
		jLocalData = data_share_.GetData("PluginLoader");
		LoadPlugins();
	}

	PluginLoader::~PluginLoader()
	{
		UnloadPlugins();
		data_share_.SetData("PluginLoader", jLocalData);
	}

	void PluginLoader::CallDrawSideBarFunctions()
	{
		for (auto &[name, plugin] : loaded_plugins_)
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

			for (auto & do_plugin : all_plugins_do_load_) {
				if (all_plugins_.contains(do_plugin.first) && ImGui::Checkbox(do_plugin.first.c_str(), &do_plugin.second)) {
					auto & plugin = loaded_plugins_[do_plugin.first];
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
			ImGui::Text(" ");
		} else {
			jLocalData["plugins_open"] = false;
		}
	}

	void PluginLoader::CallDrawTabFunctions()
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

	void PluginLoader::LoadPlugin(PluginBase * pPlugin)
	{
		loaded_plugins_[pPlugin->Name()] = pPlugin;
		auto version = *HamLab::PluginBase::Version();
		all_plugins_versions_.insert({pPlugin->Name(), version});
	}

	void PluginLoader::LoadPlugins()
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

	void PluginLoader::UnloadPlugins()
	{
		while (!loaded_plugins_.empty()) {
			auto it = loaded_plugins_.begin();

			delete it->second;

			loaded_plugins_.erase(it);
		}
		all_plugins_versions_.clear();
		all_plugins_do_load_.clear();
		all_plugins_.clear();
	}

	void PluginLoader::LoadPlugin(const std::string &path)
	{
		boost::dll::shared_library lib(path, boost::dll::load_mode::append_decorations);

		if (!lib.is_loaded())
		{
			std::cerr << "Failed to load plugin: " << path << std::endl;
			return;
		}

		typedef const char *(PluginNameFunc)();
		typedef const APIVersion *(PluginVersionFunc)();

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
} // HamLab