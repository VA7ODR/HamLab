#include "utils.hpp"
#define IMGUI_USE_STL_BINDINGS

#include "json.hpp"

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include <stdio.h>
#include <SDL.h>

#include "PluginLoader.hpp"

#if !SDL_VERSION_ATLEAST(2,0,17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

enum class ActiveTab
{
	Tab1,
	Tab2,
	Tab3
};

void ShowTab(const char* label, ActiveTab& activeTab)
{
	bool isOpen = (activeTab == ActiveTab::Tab1 && strcmp(label, "Tab 1") == 0) ||
			(activeTab == ActiveTab::Tab2 && strcmp(label, "Tab 2") == 0) ||
			(activeTab == ActiveTab::Tab3 && strcmp(label, "Tab 3") == 0);

	if (ImGui::CollapsingHeader(label, isOpen ? ImGuiTreeNodeFlags_DefaultOpen : 0))
	{
		ImGui::Text("This is the content for %s", label);

		if (isOpen)
		{
			ImGui::BulletText("Tab is open!");
			if (ImGui::Button("Close Tab"))
				activeTab = ActiveTab::Tab1; // Close the tab by setting activeTab to a default or closed state
		}
		else
		{
			ImGui::BulletText("Tab is closed!");
			if (ImGui::Button("Open Tab"))
				activeTab = (strcmp(label, "Tab 1") == 0) ? ActiveTab::Tab1 :
															(strcmp(label, "Tab 2") == 0) ? ActiveTab::Tab2 :
																							(strcmp(label, "Tab 3") == 0) ? ActiveTab::Tab3 : ActiveTab::Tab1;
		}
	}
}

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
	SDL_Window* window = SDL_CreateWindow("Dear ImGui SDL2+SDL_Renderer example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1920, 1080, window_flags);
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

	style.FrameBorderSize = 1.0;
	style.FrameRounding = 8.0;
	style.DisplayWindowPadding = {0, 0};

	io.FontDefault = io.Fonts->Fonts[4];

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
	ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
	ImGui_ImplSDLRenderer2_Init(renderer);

	io.Fonts->AddFontDefault();

	HamLab::PluginLoader loader(sPluginFolder, data_share);

	json::document jSettings = data_share.GetData("HamLabMain");

	// Our state
	bool show_demo_window = jSettings["show_demo_window"].boolean();

	ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);
	if (jSettings.exists("clear_color")) {
		auto & jClearColor = jSettings["clear_color"];
		if (jClearColor.size() >= 4) {
			clear_color = ImVec4(jClearColor[0]._float(), jClearColor[1]._float(), jClearColor[2]._float(), jClearColor[3]._float());
		}
	}

	ActiveTab activeTab = ActiveTab::Tab1;
	if (jSettings.exists("activeTab")) {
		activeTab = ActiveTab(jSettings["activeTab"]._int());
	}

	// WSJTXReceiver receiver(szWSJTXSendAddress, wsjtx_send_port, wsjtx_listen_port);

	// receiver.Start();
	// receiver.Replay();

	// Main loop
	bool done = false;

	while (!done)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			ImGui_ImplSDL2_ProcessEvent(&event);
			if (event.type == SDL_QUIT)
				done = true;
			if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
				done = true;
		}

		ImGui_ImplSDLRenderer2_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		int windowWidth, windowHeight;
		SDL_GetWindowSize(window, &windowWidth, &windowHeight);

		float sidebarWidth;
		{
			ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f); // Set background color
			ImVec2 sidebarSize(windowWidth * 0.2f, static_cast<float>(windowHeight));
			ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
			ImGui::SetNextWindowSize(sidebarSize); // Set the width to 1/5th of the window width
			ImGui::Begin("HamLab", nullptr,
						 ImGuiWindowFlags_NoTitleBar |
						 ImGuiWindowFlags_NoMove |
						 ImGuiWindowFlags_NoCollapse |
						 ImGuiWindowFlags_NoSavedSettings |
						 ImGuiWindowFlags_NoResize);

			loader.CallDrawSideBarFunctions();

			if (ImGui::CollapsingHeader("HamLab General Settings", jSettings["general_open"].boolean() ? ImGuiTreeNodeFlags_DefaultOpen : 0)) {
				jSettings["general_open"] = true;
				ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
				ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

				ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
			} else {
				jSettings["general_open"] = false;
			}

			sidebarWidth = ImGui::GetWindowWidth();

			ImGui::End();
		}


		// Main content window
		{
			ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f); // Set background color
			ImGui::SetNextWindowPos(ImVec2(sidebarWidth, 0.0f), ImGuiCond_Always);
			ImGui::SetNextWindowSize(ImVec2(windowWidth - sidebarWidth, static_cast<float>(windowHeight)), ImGuiCond_Always);
			ImGui::Begin("Main Content", nullptr,
						 ImGuiWindowFlags_NoTitleBar |
						 ImGuiWindowFlags_NoMove |
						 ImGuiWindowFlags_NoCollapse |
						 ImGuiWindowFlags_NoSavedSettings |
						 ImGuiWindowFlags_NoResize);

			if (ImGui::BeginTabBar("Tabs")) {
				loader.CallDrawTabFunctions();
				ImGui::EndTabBar();
			}

			ImGui::End();
		}

		if (show_demo_window){
			ImGui::ShowDemoWindow(&show_demo_window);
		}

		// Rendering
		ImGui::Render();
		SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
		SDL_SetRenderDrawColor(renderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
		SDL_RenderClear(renderer);
		ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
		SDL_RenderPresent(renderer);
	}

	// receiver.Stop();

	// Our state
	jSettings["show_demo_window"] = show_demo_window;

	jSettings["clear_color"][0] = clear_color.x;
	jSettings["clear_color"][1] = clear_color.y;
	jSettings["clear_color"][2] = clear_color.z;
	jSettings["clear_color"][3] = clear_color.w;

	jSettings["activeTab"] = (int)activeTab;

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
