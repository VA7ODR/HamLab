#include "utils.hpp"
#define IMGUI_USE_STL_BINDINGS

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include "json.hpp"
#include "logger/logger.hpp"
#include "PluginLoader.hpp"

#include <filesystem>
#include <SDL.h>
#include <stdio.h>

#if !SDL_VERSION_ATLEAST(2,0,17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

// Main code
int main(int, char**)
{
	// Setup SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
	{
		printf("Error: %s\n", SDL_GetError());
		return -1;
	}

	HamLab::DataShare data_share{GetAppDataFolder() + "/HamLab.json"};

	std::string sPluginFolder;

	if (std::filesystem::is_directory(GetAppDataFolder() + "/plugins")) {
		sPluginFolder = GetAppDataFolder() + "/plugins";
	} else if (std::filesystem::is_directory("./plugins")) {
		sPluginFolder = "./plugins";
	}

	// From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
	SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

	// Create window with SDL_Renderer graphics context
	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_MAXIMIZED);
	SDL_Window* window = SDL_CreateWindow((std::string("HamLab v") + HamLab::CURRENT_API_VERSION.toString()).c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1920, 1080, window_flags);
	if (window == nullptr)
	{
		printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
		return -1;
	}
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
	if (renderer == nullptr)
	{
		SDL_Log("Error creating SDL_Renderer!");
		return 0;
	}

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.Fonts->AddFontFromFileTTF("/home/jim/Downloads/hack/Hack-Regular.ttf", 12);
	io.Fonts->AddFontFromFileTTF("/home/jim/Downloads/hack/Hack-Regular.ttf", 16);
	io.Fonts->AddFontFromFileTTF("/home/jim/Downloads/hack/Hack-Regular.ttf", 18);
	io.Fonts->AddFontFromFileTTF("/home/jim/Downloads/hack/Hack-Regular.ttf", 24);
	io.Fonts->AddFontFromFileTTF("/home/jim/Downloads/hack/Hack-Regular.ttf", 36);
	io.Fonts->AddFontFromFileTTF("/home/jim/Downloads/hack/Hack-Regular.ttf", 48);
	io.Fonts->AddFontFromFileTTF("/home/jim/Downloads/hack/Hack-Regular.ttf", 64);
	io.Fonts->AddFontFromFileTTF("/home/jim/Downloads/hack/Hack-Regular.ttf", 72);
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	auto & style = ImGui::GetStyle();

	style.DisplayWindowPadding = {0, 0};

	style.WindowRounding = 8.0;
	style.ChildRounding = 8.0;
	style.FrameRounding = 8.0;
	style.PopupRounding = 8.0;
	style.ScrollbarRounding = 8.0;
	style.GrabRounding = 8.0;
	style.TabRounding = 6.0;

	style.WindowBorderSize = 0.0;
	style.ChildBorderSize = 1.0;
	style.PopupBorderSize = 1.0;
	style.FrameBorderSize = 1.0;
	style.TabBorderSize = 1.0;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
	ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
	ImGui_ImplSDLRenderer2_Init(renderer);

	io.Fonts->AddFontDefault();

	HamLab::PluginLoader loader(sPluginFolder, data_share);

	loader.LoadPlugin(Logger::create(loader.data_share(), "Logger"));

	ojson::document jSettings = data_share.GetData("HamLabMain");

	// Our state
	// bool show_demo_window = jSettings["show_demo_window"].boolean();

	size_t iFontIndex = 3;
	if (jSettings.exists("default_font")) {
		iFontIndex = jSettings["default_font"]._size_t();
	}

	io.FontDefault = io.Fonts->Fonts[iFontIndex];

	if (jSettings.exists("chosen_colours")) {
		ImVec4 * colors = ImGui::GetStyle().Colors;
		for (int i = 0; i < ImGuiCol_COUNT; ++i) {
			colors[i] = JSONArrayToImVec4(jSettings["chosen_colours"][i]);
		}
	}

	bool bShowSettingsJSON = jSettings["jsow_settings_json"].boolean();

	// Main loop
	bool done = false;

	while (!done) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			ImGui_ImplSDL2_ProcessEvent(&event);
			if (event.type == SDL_QUIT) {
				done = true;
			}
			if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window)) {
				done = true;
			}
		}

		ImGui_ImplSDLRenderer2_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		int windowWidth, windowHeight;
		SDL_GetWindowSize(window, &windowWidth, &windowHeight);

		ImVec2 sidebarSize(windowWidth, static_cast<float>(windowHeight));
		ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
		ImGui::SetNextWindowSize(sidebarSize); // Set the width to 1/5th of the window width

		ImGui::Begin("HamLab", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDecoration);

		ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetStyleColorVec4(ImGuiCol_Border));
		style.Colors[ImGuiCol_Border] = style.Colors[ImGuiCol_BorderShadow] = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
		ImGui::BeginTable("Main", 2, ImGuiTableFlags_Resizable);

		ImGui::TableSetupColumn("SideBar", ImGuiTableColumnFlags_WidthStretch, windowWidth * 0.2);
		ImGui::TableSetupColumn("Tabs", ImGuiTableColumnFlags_WidthStretch, windowWidth * 0.8);

		ImGui::TableNextRow();
		ImGui::PopStyleColor();
		ImGui::TableSetColumnIndex(0);
		loader.CallDrawSideBarFunctions();

		if (ImGui::CollapsingHeader("HamLab General Settings", jSettings["general_open"].boolean() ? ImGuiTreeNodeFlags_DefaultOpen : 0)) {
			jSettings["general_open"] = true;

			ImFont * font_current = ImGui::GetFont();
			if (ImGui::BeginCombo("Font", font_current->GetDebugName())) {
				size_t iIndex = 0;
				for (ImFont * font : io.Fonts->Fonts) {
					ImGui::PushID((void *)font);
					if (ImGui::Selectable(font->GetDebugName(), font == font_current)) {
						io.FontDefault = font;
						iFontIndex = iIndex;
					}
					ImGui::PopID();
					++iIndex;
				}
				ImGui::EndCombo();
			}

			ImGui::ShowStyleSelector("Select Style");

			ImGui::Checkbox("Demo Window", jsonTypedRef<bool>(jSettings["show_demo_window"])); // Edit bools storing our window open/close state
			ImGui::Checkbox("Show Settings", &bShowSettingsJSON);

			if (bShowSettingsJSON) {
				ShowJsonWindow("HamLab Settings", jSettings, bShowSettingsJSON);
			}

			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

		} else {
			jSettings["general_open"] = false;
		}

		ImGui::TableSetColumnIndex(1);

		ImGui::BeginChild("child", ImVec2(0, 0), ImGuiChildFlags_Border | ImGuiChildFlags_FrameStyle);
		if (ImGui::BeginTabBar("Tabs")) {
			loader.CallDrawTabFunctions();
			ImGui::EndTabBar();
		}
		ImGui::EndChild();

		ImGui::EndTable();

		ImGui::End();

		if (jSettings["show_demo_window"].boolean()) {
			ImGui::ShowDemoWindow(jsonTypedRef<bool>(jSettings["show_demo_window"]));
		}

		// Rendering
		ImGui::Render();
		SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 1);
		SDL_RenderClear(renderer);
		ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
		SDL_RenderPresent(renderer);
	}

	// receiver.Stop();

	// Our state
	// jSettings["show_demo_window"] = show_demo_window;
	jSettings["default_font"] = iFontIndex;

	{
		ImVec4 * colors = ImGui::GetStyle().Colors;
		for (int i = 0; i < ImGuiCol_COUNT; ++i) {
			jSettings["chosen_colours"][i] = ImVec4ToJSONArray(colors[i]);
		}
	}

	jSettings["jsow_settings_json"] = bShowSettingsJSON;

	data_share.SetData("HamLabMain", jSettings);

	// Cleanup
	ImGui_ImplSDLRenderer2_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
