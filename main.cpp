#include "utils.hpp"
#define IMGUI_USE_STL_BINDINGS

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "backends/imgui_impl_opengl3.h"
#include "json.hpp"
#include "logger/logger.hpp"
#include "PluginLoader.hpp"

#include <filesystem>
#include <SDL.h>
#include "SDL_opengl.h"

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

	// Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
	// GL ES 2.0 + GLSL 100
	const char *glsl_version = "#version 100";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
	// GL 3.2 Core + GLSL 150
	const char *glsl_version = "#version 150";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,
						SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
	// GL 3.0 + GLSL 130
	const char *glsl_version = "#version 130";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

	// From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
	SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

	// Create window with graphics context
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_WindowFlags window_flags = (SDL_WindowFlags) (SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
													  | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_SHOWN
													  | SDL_WINDOW_MAXIMIZED);

	// From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
	SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

	// Create window
	SDL_Window* window = SDL_CreateWindow((std::string("HamLab v") + HamLab::CURRENT_API_VERSION.toString()).c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1920, 1080, window_flags);
	if (window == nullptr)
	{
		printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
		return -1;
	}

	SDL_GLContext gl_context = SDL_GL_CreateContext(window);
	SDL_GL_MakeCurrent(window, gl_context);

	HamLab::DataShare data_share{GetAppDataFolder() + "/HamLab.json"};

	std::string sPluginFolder;

	if (std::filesystem::is_directory(GetAppDataFolder() + "/plugins")) {
		sPluginFolder = GetAppDataFolder() + "/plugins";
	} else if (std::filesystem::is_directory("./plugins")) {
		sPluginFolder = "./plugins";
	}

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO & io = ImGui::GetIO();

	io.Fonts->AddFontFromFileTTF("/home/jim/Downloads/hack/Hack-Regular.ttf", 12);
	io.Fonts->AddFontFromFileTTF("/home/jim/Downloads/hack/Hack-Regular.ttf", 16);
	io.Fonts->AddFontFromFileTTF("/home/jim/Downloads/hack/Hack-Regular.ttf", 18);
	io.Fonts->AddFontFromFileTTF("/home/jim/Downloads/hack/Hack-Regular.ttf", 24);
	io.Fonts->AddFontFromFileTTF("/home/jim/Downloads/hack/Hack-Regular.ttf", 36);
	io.Fonts->AddFontFromFileTTF("/home/jim/Downloads/hack/Hack-Regular.ttf", 48);
	io.Fonts->AddFontFromFileTTF("/home/jim/Downloads/hack/Hack-Regular.ttf", 64);
	io.Fonts->AddFontFromFileTTF("/home/jim/Downloads/hack/Hack-Regular.ttf", 72);
	io.Fonts->AddFontFromFileTTF("/home/jim/Downloads/liquid_crystal/LiquidCrystal-Normal.otf", 18);
	io.Fonts->AddFontFromFileTTF("/home/jim/Downloads/liquid_crystal/LiquidCrystal-Normal.otf", 36);
	io.Fonts->AddFontFromFileTTF("/home/jim/Downloads/liquid_crystal/LiquidCrystal-Normal.otf", 72);
	io.Fonts->AddFontFromFileTTF("/home/jim/Downloads/liquid_crystal/LiquidCrystal-Normal.otf", 144);
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;	  // Enable Keyboard Controls
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

	style.WindowBorderSize = 1.0;
	style.ChildBorderSize = 1.0;
	style.PopupBorderSize = 1.0;
	style.FrameBorderSize = 1.0;
	style.TabBorderSize = 1.0;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
	ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
	ImGui_ImplOpenGL3_Init(glsl_version);

	// io.Fonts->AddFontDefault();

	HamLab::PluginLoader loader(sPluginFolder, data_share);

	loader.LoadPlugin(Logger::create(loader.data_share(), "Logger"));

	ojson::document jSettings = data_share.GetData("HamLabMain");

	// Our state
	// bool show_demo_window = jSettings["show_demo_window"].boolean();

	if (!jSettings.exists("vsync")) {
		jSettings["vsync"] = true;
	}

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

		ImGui_ImplOpenGL3_NewFrame();
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
		ImGui::BeginChild("sidebar_child", ImVec2(0, 0), ImGuiChildFlags_AutoResizeY);
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

			ImGui::Checkbox("VSync", jsonTypedRef<bool>(jSettings["vsync"]));
			if (jSettings["vsync"].boolean()) {
				SDL_GL_SetSwapInterval(1);
			} else {
				SDL_GL_SetSwapInterval(0);
			}
			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

		} else {
			jSettings["general_open"] = false;
		}

		ImGui::EndChild();

		ImGui::TableSetColumnIndex(1);

		ImGui::BeginChild("tab_child", ImVec2(0, 0), ImGuiChildFlags_Border | ImGuiChildFlags_FrameStyle);
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
		auto bg_col = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
		glViewport(0, 0, (int) io.DisplaySize.x, (int) io.DisplaySize.y);
		glClearColor(bg_col.x * bg_col.w, bg_col.y * bg_col.w, bg_col.z * bg_col.w, bg_col.w);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Update and Render additional Platform Windows
		// (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
		//  For this specific demo app we could also call SDL_GL_MakeCurrent(window, gl_context) directly)
		// if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		// 	SDL_Window *backup_current_window = SDL_GL_GetCurrentWindow();
		// 	SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
		// 	ImGui::UpdatePlatformWindows();
		// 	ImGui::RenderPlatformWindowsDefault();
		// 	SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
		// }

		SDL_GL_SwapWindow(window);
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
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_GL_DeleteContext(gl_context);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
