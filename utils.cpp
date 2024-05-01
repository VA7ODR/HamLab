#include "utils.hpp"

#include <functional>

std::string Pretty(const std::vector<unsigned char> &in)
{
	std::string sRet;
	sRet.reserve(in.size() * 8);
	for (auto & c : in) {
		if (c < ' ' || c >= 127) {
			char szHex[24];
			sprintf(szHex, "[%.2X]", c);
			sRet.append(szHex);
		} else {
			sRet.push_back((char)c);
		}
	}
	return sRet;
}

std::string GetAppDataFolder()
{
#if defined(_WIN32)
	const char *appDataEnvVar = std::getenv("APPDATA");
	if (appDataEnvVar)
		return appDataEnvVar;
	else
		return ""; // Handle error on Windows
#elif defined(__APPLE__)
	const char *homeDir = std::getenv("HOME");
	if (homeDir)
		return std::string(homeDir) + "/Library/Application Support";
	else
		return ""; // Handle error on macOS
#else
	const char *homeDir = std::getenv("HOME");
	if (homeDir)
		return std::string(homeDir) + "/.local/share";
	else
		return ""; // Handle error on Linux
#endif
}

void ShowJsonWindow(const std::string & sTitle, json::value & jData, bool & bShow)
{
	ImGui::Begin(sTitle.c_str(), &bShow);
	// if (ImGui::TreeNode("Tree view")) {
	static ImGuiTableFlags flags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody;

	static ImGuiTreeNodeFlags tree_node_flags = ImGuiTreeNodeFlags_SpanAllColumns;
	// 	ImGui::CheckboxFlags("ImGuiTreeNodeFlags_SpanFullWidth", &tree_node_flags, ImGuiTreeNodeFlags_SpanFullWidth);
	// 	ImGui::CheckboxFlags("ImGuiTreeNodeFlags_SpanAllColumns", &tree_node_flags, ImGuiTreeNodeFlags_SpanAllColumns);
	ImGui::Text("Right-click field name or value to copy it to the clipboard.");
	if (ImGui::BeginTable("3ways", 3, flags)) {
		// The first column will use the default _WidthStretch when ScrollX is Off and _WidthFixed when ScrollX is On
		ImGui::TableSetupColumn("Index", ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableHeadersRow();

		std::function<void(const std::string &, json::value &)> recursiveLambda = [&recursiveLambda](const std::string & sName, json::value & jValue) -> void
		{
			switch (jValue.isA()) {
				case json::JSON_VOID:
					return;

				case json::JSON_NULL:
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					ImGui::TreeNodeEx(sName.c_str(), tree_node_flags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen);
					if (ImGui::IsItemClicked(1)) {
						ImGui::SetClipboardText(sName.c_str());
					}
					ImGui::TableNextColumn();
					ImGui::Text("NULL");
					if (ImGui::IsItemClicked(1)) {
						ImGui::SetClipboardText("NULL");
					}
					ImGui::TableNextColumn();
					ImGui::Text("null");
					break;

				case json::JSON_BOOLEAN:
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					ImGui::TreeNodeEx(sName.c_str(), tree_node_flags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen);
					if (ImGui::IsItemClicked(1)) {
						ImGui::SetClipboardText(sName.c_str());
					}
					ImGui::TableNextColumn();
					ImGui::Text(jValue.boolean() ? "true" : "false");
					if (ImGui::IsItemClicked(1)) {
						ImGui::SetClipboardText(jValue.boolean() ? "true" : "false");
					}
					ImGui::TableNextColumn();
					ImGui::Text("bool");
					break;

				case json::JSON_NUMBER:
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					ImGui::TreeNodeEx(sName.c_str(), tree_node_flags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen);
					if (ImGui::IsItemClicked(1)) {
						ImGui::SetClipboardText(sName.c_str());
					}
					ImGui::TableNextColumn();
					ImGui::Text("%s", jValue.c_str());
					if (ImGui::IsItemClicked(1)) {
						ImGui::SetClipboardText(jValue.c_str());
					}
					ImGui::TableNextColumn();
					ImGui::Text("number");
					break;

				case json::JSON_STRING:
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					ImGui::TreeNodeEx(sName.c_str(), tree_node_flags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen);
					if (ImGui::IsItemClicked(1)) {
						ImGui::SetClipboardText(sName.c_str());
					}
					ImGui::TableNextColumn();
					ImGui::Text("%s", jValue.c_str());
					if (ImGui::IsItemClicked(1)) {
						ImGui::SetClipboardText(jValue.c_str());
					}
					ImGui::TableNextColumn();
					ImGui::Text("string");
					break;

				case json::JSON_OBJECT:
				{
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					bool open = ImGui::TreeNodeEx(sName.c_str(), tree_node_flags | ImGuiTreeNodeFlags_DefaultOpen);
					if (ImGui::IsItemClicked(1)) {
						ImGui::SetClipboardText(sName.c_str());
					}
					ImGui::TableNextColumn();
					ImGui::TextDisabled("--");
					ImGui::TableNextColumn();
					ImGui::Text("object");
					if (open) {
						for (auto & sub : jValue) {
							recursiveLambda(sub.key(), sub);
						}
						ImGui::TreePop();
					}
					break;
				}

				case json::JSON_ARRAY:
				{
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					bool open = ImGui::TreeNodeEx(sName.c_str(), tree_node_flags | ImGuiTreeNodeFlags_DefaultOpen);
					if (ImGui::IsItemClicked(1)) {
						ImGui::SetClipboardText(sName.c_str());
					}
					ImGui::TableNextColumn();
					ImGui::TextDisabled("--");
					ImGui::TableNextColumn();
					ImGui::Text("array");
					if (open) {
						size_t iIndex = 0;
						for (auto & sub : jValue) {
							recursiveLambda(std::to_string(iIndex++), sub);
						}
						ImGui::TreePop();
					}
					break;
				}
			}
		};

		recursiveLambda(sTitle.c_str(), jData);
		ImGui::EndTable();
	}
	// 	ImGui::TreePop();
	// }
	ImGui::End();
}

json::value ImVec4ToJSONArray(const ImVec4 & in)
{
	json::value ret;
	ret[0] = in.x;
	ret[1] = in.y;
	ret[2] = in.z;
	ret[3] = in.w;
	return ret;
}

ImVec4 JSONArrayToImVec4(json::value & jData)
{
	ImVec4 ret {0, 0, 0, 0};

	if (jData.IsArray()) {
		ret.x = jData[0]._float();
		ret.y = jData[1]._float();
		ret.z = jData[2]._float();
		ret.w = jData[3]._float();
	}
	return ret;
}
