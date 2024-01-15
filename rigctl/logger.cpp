#include "logger.hpp"

// #include "utils.hpp"
#include "json.hpp"
#include "utils.hpp"

Logger::Logger(HamLab::DataShare & pDataShareIn, const std::string & name_in) : HamLab::PluginBase(pDataShareIn, name_in) {}

Logger::~Logger() {}

bool Logger::DrawSideBar(bool bOpen)
{
	if (ImGui::CollapsingHeader("Logger", bOpen ? ImGuiTreeNodeFlags_DefaultOpen : 0)) {
		auto jLoggerData = data_share_.GetData("logger_data");
		// ImGui::Lab
		ImGui::InputFloat("Freq (MHz)", jsonTypedRef<float>(jLoggerData["freq"]), 0, 0, "%.6f");
		ImGui::InputText("DE", jsonTypedRef<char *>(jLoggerData["call"]), 8, ImGuiInputTextFlags_CharsUppercase);
		ImGui::InputText("DE Grid", jsonTypedRef<char *>(jLoggerData["de_grid"]), 6);
		ImGui::InputText("DX", jsonTypedRef<char *>(jLoggerData["dx_call"]), 8, ImGuiInputTextFlags_CharsUppercase);
		ImGui::InputText("DX Grid", jsonTypedRef<char *>(jLoggerData["dx_grid"]), 6);
		data_share_.SetData("logger_data", jLoggerData);

		return true;
	}
	return false;
}

HamLab::PluginBase * Logger::create(HamLab::DataShare & data_share_, const std::string & name) { return new Logger(data_share_, name); }
