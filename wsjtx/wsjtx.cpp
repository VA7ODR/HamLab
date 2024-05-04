#include "wsjtx.hpp"
#include "utils.hpp"
#include <boost/dll/alias.hpp>

WSJTXReceiver::WSJTXReceiver(HamLab::DataShareClass & pDataShareIn, const std::string & name_in) :
	HamLab::PluginBase(pDataShareIn, name_in)
{
	std::cout << "WSJTXReceiver(" << sName << ")" << std::endl;

	if (!jLocalData.exists("send_address")) {
		jLocalData["send_address"] = "127.0.0.1";
	}

	if (!jLocalData.exists("send_port")) {
		jLocalData["send_port"] = 2237;
	}

	if (!jLocalData.exists("listen_port")) {
		jLocalData["listen_port"] = 2237;
	}

	// Open the socket and join the multicast group
	boost::asio::ip::udp::endpoint endpoint_(boost::asio::ip::address::from_string("0.0.0.0"), jLocalData["listen_port"]._int());
	socket_.open(endpoint_.protocol());
	socket_.set_option(boost::asio::ip::udp::socket::reuse_address(true));
	socket_.bind(endpoint_);

	Start();
	Replay();
}

WSJTXReceiver::~WSJTXReceiver()
{
	Stop();
	// jLocalData["send_address"] = szWSJTXSendAddress;
	// jLocalData["send_port"] = send_port;
	// jLocalData["listen_port"] = listen_port;
	// jLocalData["show_json"] = bShowJSON;
	std::cout << "~WSJTXReceiver()" << std::endl;
}

bool WSJTXReceiver::DrawSideBar(bool bOpen)
{
	if (ImGui::CollapsingHeader("WSJT-X Settings", bOpen ? ImGuiTreeNodeFlags_DefaultOpen : 0)) {
		// Adjust the layout of the main content area for Section 1
		ImGui::InputText("Send Address", jsonTypedRef<char *>(jLocalData["send_address"]), 256);
		ImGui::InputInt("Send Port", jsonTypedRef<int>(jLocalData["send_port"]));
		if (ImGui::InputInt("Listen Port", jsonTypedRef<int>(jLocalData["listen_port"]))) {
			Restart();
		}
		ImGui::Checkbox("Show JSON", jsonTypedRef<bool>(jLocalData["show_json"]));
		if (jLocalData["show_json"].boolean()) {
			auto jLocalHistory = history();
			ShowJsonWindow("WSJT-X History", jLocalHistory, *jsonTypedRef<bool>(jLocalData["show_json"]));
		}
		return true;
	}
	return false;
}

void WSJTXReceiver::DrawTab()
{
	if (ImGui::BeginTabItem(sName.c_str())) {
		json::document jLocalHistory = history();
		int            id            = 0;
		if (jLocalHistory["Status"]["tx_enabled"].boolean()) {
			ImGui::PushID(id++);
			ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4) ImColor(0.6f, 0.3f, 0.1f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4) ImColor(0.6f, 0.4f, 0.2f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4) ImColor(0.6f, 0.5f, 0.3f));
			ImGui::Button("TX Enabled");
			ImGui::PopStyleColor(3);
			ImGui::PopID();
		} else {
			ImGui::Button("TX Enabled");
		}

		ImGui::SameLine();
		if (jLocalHistory["Status"]["transmitting"].boolean()) {
			ImGui::PushID(id++);
			ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4) ImColor(0.6f, 0.1f, 0.1f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4) ImColor(0.6f, 0.2f, 0.2f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4) ImColor(0.6f, 0.3f, 0.3f));
			ImGui::Button("Transmitting");
			ImGui::PopStyleColor(3);
			ImGui::PopID();
		} else {
			ImGui::Button("Transmitting");
		}

		ImGui::SameLine();
		if (jLocalHistory["Status"]["decoding"].boolean()) {
			ImGui::PushID(id++);
			ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4) ImColor(0.1f, 0.6f, 0.1f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4) ImColor(0.2f, 0.6f, 0.2f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4) ImColor(0.3f, 0.6f, 0.3f));
			ImGui::Button("Decoding");
			ImGui::PopStyleColor(3);
			ImGui::PopID();
		} else {
			ImGui::Button("Decoding");
		}

		ImGui::SameLine();
		if (jLocalHistory["Status"]["watchdog_timeout"].boolean()) {
			ImGui::PushID(id++);
			ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4) ImColor(0.1f, 0.1f, 0.6f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4) ImColor(0.2f, 0.2f, 0.6f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4) ImColor(0.3f, 0.3f, 0.6f));
			ImGui::Button("Watchdog");
			ImGui::PopStyleColor(3);
			ImGui::PopID();
		} else {
			ImGui::Button("Watchdog");
		}

		jLocalHistory["Decode"].sort(
									 [](json::value & a, json::value & b)
									 {
										 bool bTimeEqual = a["time"] == b["time"];
										 if (bTimeEqual) {
											 return a["snr"] > b["snr"];
										 } else {
											 return a["time"] > b["time"];
										 }
									 });
		if (jLocalHistory["Decode"].size() && jLocalHistory["Decode"].front().size() &&
			ImGui::BeginTable("Table", jLocalHistory["Decode"].front().size(), ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_BordersOuterV)) {
			// Set style for borders
			ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4, 2)); // Adjust cell padding if needed
			// ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(255, 255, 255, 255)); // Set border color

			std::deque<std::string> headers;
			std::deque<float>       sizes;

			sizes.resize(jLocalHistory["Decode"].size());
			int iCol = 0;

			for (auto & entry : jLocalHistory["Decode"].front()) {
				ImVec2 textSize = ImGui::CalcTextSize(entry.key().c_str());
				sizes[iCol]     = std::max(sizes[iCol], textSize.x + ImGui::GetStyle().CellPadding.x * 2.0f);
				headers.push_back(entry.key());
				++iCol;
			}

			for (auto & row : jLocalHistory["Decode"]) {
				iCol = 0;
				for (auto & col : row) {
					col.toString(1);
					ImVec2 textSize = ImGui::CalcTextSize(col.c_str());
					sizes[iCol]     = std::max(sizes[iCol], textSize.x + ImGui::GetStyle().CellPadding.x * 2.0f);
					++iCol;
				}
			}

			// Headers
			int colIndex = 0;
			for (auto & entry : headers) {
				// ImGui::TableSetColumnIndex(colIndex);
				ImGui::TableSetupColumn(entry.c_str(), ImGuiTableColumnFlags_WidthFixed, sizes[colIndex]);
				++colIndex;

				// std::cout << entry.c_str() << ": " << sizes[colIndex] << std::endl;
			}

			ImGui::TableHeadersRow();

			// Data
			size_t rowIndex = 0;
			for (const auto & row : jLocalHistory["Decode"]) {
				ImGui::TableNextRow();

				int iCol = 0;
				for (auto & entry : row) {
					entry.toString(1);
					ImGui::TableSetColumnIndex(static_cast<int>(iCol++));

					ImGui::Text("%s", entry.c_str());
				}
			}

			ImGui::PopStyleVar();

			ImGui::EndTable();
		}
		ImGui::EndTabItem();
	}
}

HamLab::PluginBase * WSJTXReceiver::create(HamLab::DataShareClass & data_share_, const std::string & name)
{
	return new WSJTXReceiver(data_share_, name);
}

void WSJTXReceiver::Start()
{
	StartReceive();
	if (!ioThread.joinable()) {
		ioThread = std::thread([this] { ioc.run(); });
	}
}

void WSJTXReceiver::Restart()
{
	Stop();

	if (socket_.is_open()) {
		socket_.close();
	}
	boost::asio::ip::udp::endpoint endpoint_(boost::asio::ip::address::from_string("0.0.0.0"), jLocalData["listen_port"]._int());
	socket_.open(endpoint_.protocol());
	socket_.set_option(boost::asio::ip::udp::socket::reuse_address(true));
	socket_.bind(endpoint_);

	Start();
}

void WSJTXReceiver::Stop()
{
	ioc.stop();
	if (ioThread.joinable()) {
		ioThread.join();
	}
	ioc.restart();
}

json::document WSJTXReceiver::history()
{
	std::lock_guard<std::recursive_mutex> lk(mtx);
	return jHistory;
}

void WSJTXReceiver::SendReply(json::document & jData)
{
	BinarySerializer binarySerializer;
	binarySerializer.Num2Bin(static_cast<uint32_t>(2914831322L));
	binarySerializer.Num2Bin(static_cast<uint32_t>(2));
	binarySerializer.Num2Bin(static_cast<uint32_t>(WSJT_X_Msg_Reply));
	binarySerializer.Str2Bin(jData["target_id"].string());
	binarySerializer.Num2Bin(jData["time"]._uint32());
	binarySerializer.Num2Bin(jData["snr"]._uint32());
	binarySerializer.Num2Bin(jData["delta_time"]._double());
	binarySerializer.Num2Bin(jData["delta_frequency"]._uint32());
	binarySerializer.Str2Bin(jData["mode"].string());
	binarySerializer.Str2Bin(jData["message_text"].string());
	binarySerializer.Num2Bin(jData["low_confidence"].boolean());
	binarySerializer.Num2Bin(static_cast<u_int8_t>(0));

	Send(*binarySerializer);
}

void WSJTXReceiver::Replay()
{
	BinarySerializer binarySerializer;
	binarySerializer.Num2Bin(static_cast<uint32_t>(2914831322L));
	binarySerializer.Num2Bin(static_cast<uint32_t>(2));
	binarySerializer.Num2Bin(static_cast<uint32_t>(WSJT_X_Msg_Replay));
	binarySerializer.Str2Bin("HamLab");
	Send(*binarySerializer);
}

void WSJTXReceiver::Send(const std::vector<unsigned char> & data)
{
	std::cout << "WSJTXReceiver::Send: " << jLocalData["send_address"].string() << ":" << jLocalData["send_port"]._int() << std::endl;
	boost::asio::ip::udp::resolver               resolver(ioc);
	boost::asio::ip::udp::resolver::results_type endpoints    = resolver.resolve(jLocalData["send_address"].string(), jLocalData["send_port"].string());
	boost::asio::ip::udp::endpoint               sendEndpoint = *endpoints.begin();
	boost::asio::ip::udp::socket                 socket(ioc);
	socket.open(boost::asio::ip::udp::v4());
	socket.set_option(boost::asio::ip::udp::socket::reuse_address(true));
	boost::system::error_code ec;
	// if (std::string(szWSJTXSendAddress).ends_with(".255")) {
	socket.set_option(boost::asio::socket_base::broadcast(true));
	// }
	socket.send_to(boost::asio::buffer(data), sendEndpoint, 0, ec);
	if (ec) {
		std::cerr << "WSJTXReceiver::Send failed (" << jLocalData["send_address"].string() << "): " << ec.what() << std::endl;
	}
}

void WSJTXReceiver::StartReceive()
{
	std::cout << sName << ": StartReceive." << std::endl;
	socket_.async_receive_from(boost::asio::buffer(data), senderEndpoint, [this](const boost::system::error_code & error, std::size_t bytesReceived)
	{
		if (!error) {
			ProcessMessage(bytesReceived);
			StartReceive(); // Continue to listen for more messages
		} else {
			std::cerr << "Error receiving data: " << error.message() << std::endl;
		}
	});
}

void WSJTXReceiver::ProcessMessage(std::size_t bytesReceived)
{
	std::vector<unsigned char> message(data.begin(), data.begin() + bytesReceived);

	// Decode the received WSJTX message
	auto jData = DecodeMessage(message);

	// std::cout << jData << std::endl;

	std::lock_guard<std::recursive_mutex> lk(mtx);
	std::string                           sType = jData["type_name"].string();
	if (sType == "Status") {
		auto jLoggerData       = dataShare.GetData("logger_data");
		jLoggerData["call"]    = jData["call"];
		jLoggerData["de_grid"] = jData["de_grid"];
		jLoggerData["dx_call"] = jData["dx_call"];
		jLoggerData["dx_grid"] = jData["dx_grid"];
		jLoggerData["freq"]    = jData["freq"] / 1000000.0;
		dataShare.SetData("logger_data", jLoggerData);
		jHistory[sType] = jData;
	} else {
		auto & jTypeHistory = jHistory[sType];
		jTypeHistory.toArray();
		jTypeHistory.push_front(jData);

		while (jTypeHistory.size() > 50) {
			jTypeHistory.pop_back();
		}
	}

	// jHistory.writeFile("jHistory.json", true);
	// // Print the received message
	std::cout << "Received message size: " << message.size() << " bytes" << std::endl;
}

json::document WSJTXReceiver::DecodeMessage(const std::vector<unsigned char> & vMessage)
{
	json::document jRet;

	jRet["type_name"] = "Unknown";

	if (vMessage.size() < 16) {
		jRet["error"]         = true;
		jRet["error_message"] = "WSJT-X: Datagram Too Short.";
		return jRet;
	}

	std::cout << PrettyHex(vMessage) << std::endl;

	BinaryConverter binaryConverter(vMessage.begin(), vMessage.end());

	try {
		binaryConverter.Bin2Num<uint32_t>(jRet["magic"]);
		binaryConverter.Bin2Num<uint32_t>(jRet["schema"]);

		binaryConverter.Bin2Num<uint32_t>(jRet["type"]);
		WSJT_X_Msg_Type type = static_cast<WSJT_X_Msg_Type>(jRet["type"]._uint32());

		binaryConverter.Bin2Str(jRet["id"]);

		switch (type) {
			case WSJT_X_Msg_Heartbeat:
			{
				jRet["type_name"] = "Heartbeat";
				binaryConverter.Bin2Num<uint32_t>(jRet["max_schema"]);
				binaryConverter.Bin2Str(jRet["version"]);
				binaryConverter.Bin2Str(jRet["revision"]);
				break;
			}

			case WSJT_X_Msg_Status:
			{
				jRet["type_name"]           = "Status";
				jRet["freq"]                = binaryConverter.Bin2Num<uint64_t>();
				jRet["mode"]                = binaryConverter.Bin2Str();
				jRet["dx_call"]             = binaryConverter.Bin2Str();
				jRet["report"]              = binaryConverter.Bin2Str();
				jRet["tx_mode"]             = binaryConverter.Bin2Str();
				jRet["tx_enabled"]          = binaryConverter.Bin2Num<bool>();
				jRet["transmitting"]        = binaryConverter.Bin2Num<bool>();
				jRet["decoding"]            = binaryConverter.Bin2Num<bool>();
				jRet["rx_df"]               = binaryConverter.Bin2Num<uint32_t>();
				jRet["tx_df"]               = binaryConverter.Bin2Num<uint32_t>();
				jRet["call"]                = binaryConverter.Bin2Str();
				jRet["de_grid"]             = binaryConverter.Bin2Str();
				jRet["dx_grid"]             = binaryConverter.Bin2Str();
				jRet["watchdog_timeout"]    = binaryConverter.Bin2Num<bool>();
				jRet["sub_mode"]            = binaryConverter.Bin2Str();
				jRet["fast_mode"]           = binaryConverter.Bin2Num<bool>();
				jRet["special_op_mode"]     = binaryConverter.Bin2Num<uint8_t>();
				jRet["frequency_tolerance"] = binaryConverter.Bin2Num<uint32_t>();
				jRet["tr_period"]           = binaryConverter.Bin2Num<uint32_t>();
				jRet["configuration_name"]  = binaryConverter.Bin2Str();
				jRet["tx_message"]          = binaryConverter.Bin2Str();

				break;
			}

			case WSJT_X_Msg_Decode:
			{
				jRet["type_name"]       = "Decode";
				jRet["is_new"]          = binaryConverter.Bin2Num<bool>();
				jRet["time"]            = binaryConverter.Bin2Num<uint32_t>();
				jRet["snr"]             = binaryConverter.Bin2Num<int32_t>();
				jRet["delta_time"]      = binaryConverter.Bin2Num<double>();
				jRet["delta_frequency"] = binaryConverter.Bin2Num<uint32_t>();
				jRet["mode"]            = binaryConverter.Bin2Str();
				jRet["message_text"]    = binaryConverter.Bin2Str();
				jRet["low_confidence"]  = binaryConverter.Bin2Num<bool>();
				jRet["off_air"]         = binaryConverter.Bin2Num<bool>();

				break;
			}

			case WSJT_X_Msg_Clear:
			{
				jRet["type_name"] = "Clear";
				jRet["window"]    = binaryConverter.Bin2Num<uint8_t>();

				break;
			}

			case WSJT_X_Msg_Reply:
			{
				jRet["type_name"]       = "Reply";
				jRet["time"]            = binaryConverter.Bin2Num<uint32_t>();
				jRet["snr"]             = binaryConverter.Bin2Num<int32_t>();
				jRet["delta_time"]      = binaryConverter.Bin2Num<double>();
				jRet["delta_frequency"] = binaryConverter.Bin2Num<uint32_t>();
				jRet["mode"]            = binaryConverter.Bin2Str();
				jRet["message_text"]    = binaryConverter.Bin2Str();
				jRet["low_confidence"]  = binaryConverter.Bin2Num<bool>();
				jRet["modifiers"]       = binaryConverter.Bin2Num<uint8_t>();

				break;
			}

			case WSJT_X_Msg_Replay:
			{
				jRet["type_name"] = "Replay";
				break;
			}
			/*      QDateTime:
 *           QDate      qint64    Julian day number
 *           QTime      quint32   Milli-seconds since midnight
 *           timespec   quint8    0=local, 1=UTC, 2=Offset from UTC
 *                                                 (seconds)
 *                                3=time zone
 *           offset     qint32    only present if timespec=2
 *           timezone   several-fields only present if timespec=3
 */
			case WSJT_X_Msg_QSOLogged:
			{
				jRet["type_name"] = "QSOLogged";
				// std::cout << "QSOLogged: " << std::endl;
				jRet["date_off"]     = binaryConverter.Bin2Num<uint64_t>();
				jRet["time_off"]     = binaryConverter.Bin2Num<uint32_t>();
				jRet["timespec_off"] = binaryConverter.Bin2Num<uint8_t>();
				// jRet["offset_off"] = binaryConverter.Bin2Num<uint32_t>();

				// std::cout << Pretty({binaryConverter.begin(), binaryConverter.end()}) << std::endl;
				// std::cout << jRet.write(true) << std::endl;

				jRet["dx_call"]       = binaryConverter.Bin2Str();
				jRet["dx_grid"]       = binaryConverter.Bin2Str();
				jRet["tx_freq"]       = binaryConverter.Bin2Num<uint64_t>();
				jRet["mode"]          = binaryConverter.Bin2Str();
				jRet["report_sent"]   = binaryConverter.Bin2Str();
				jRet["report_rcvd"]   = binaryConverter.Bin2Str();
				jRet["tx_pwr"]        = binaryConverter.Bin2Str();
				jRet["comments"]      = binaryConverter.Bin2Str();
				jRet["name"]          = binaryConverter.Bin2Str();
				jRet["date_on"]       = binaryConverter.Bin2Num<uint64_t>();
				jRet["time_on"]       = binaryConverter.Bin2Num<uint32_t>();
				jRet["timespec_on"]   = binaryConverter.Bin2Num<uint8_t>();
				jRet["offset_on"]     = binaryConverter.Bin2Num<uint32_t>();
				jRet["operator_call"] = binaryConverter.Bin2Str();
				jRet["my_call"]       = binaryConverter.Bin2Str();
				jRet["my_grid"]       = binaryConverter.Bin2Str();
				jRet["exch_sent"]     = binaryConverter.Bin2Str();
				jRet["exch_rcvd"]     = binaryConverter.Bin2Str();
				jRet["adif_prop"]     = binaryConverter.Bin2Str();

				break;
			}

			case WSJT_X_Msg_WSPRDecode:
			{
				jRet["type_name"]  = "WSPRDecode";
				jRet["is_new"]     = binaryConverter.Bin2Num<bool>();
				jRet["time"]       = binaryConverter.Bin2Num<uint32_t>();
				jRet["snr"]        = binaryConverter.Bin2Num<int32_t>();
				jRet["delta_time"] = binaryConverter.Bin2Num<double>();
				jRet["frequency"]  = binaryConverter.Bin2Num<uint64_t>();
				jRet["drift"]      = binaryConverter.Bin2Num<uint32_t>();
				jRet["callsign"]   = binaryConverter.Bin2Str();
				jRet["grid"]       = binaryConverter.Bin2Str();
				jRet["power"]      = binaryConverter.Bin2Num<uint32_t>();
				jRet["off_air"]    = binaryConverter.Bin2Num<bool>();
			}

			case WSJT_X_Msg_LoggedADIF:
			{
				jRet["type_name"] = "LoggedADIF";
				jRet["adif_text"] = binaryConverter.Bin2Str();

				break;
			}

			default:
				jRet["type_name"] = "Unknown";
				jRet["error"]         = true;
				jRet["error_message"] = "WSJT-X: Unhandled message type.";
				break;
		}

		if (binaryConverter.Current() != binaryConverter.End()) {
			jRet["remaining"] = PrettyHex({binaryConverter.Begin(), binaryConverter.End()});
		}
	} catch (const HamLabException & e) {
		std::cerr << "WSJTXReceiver Error decoding " << jRet["type_name"].string() << "message:" << std::endl;
		std::cerr << "    What: " << e.what() << std::endl;
		std::cerr << "    Done: " << PrettyHex({binaryConverter.Begin(), binaryConverter.Current()}) << std::endl;
		std::cerr << "    Left: " << PrettyHex({binaryConverter.Current(), binaryConverter.End()}) << std::endl;
		std::cerr << "    JSON:\n" << jRet.write(true) << std::endl;
		jRet["error"] = "WSJTXReceiver Error decoding " + jRet["type_name"].string();
		jRet["what"]  = e.what();
		jRet["done"]  = PrettyHex({binaryConverter.Begin(), binaryConverter.Current()});
		jRet["left"]  = PrettyHex({binaryConverter.Current(), binaryConverter.End()});
	}

	return jRet;
}

BOOST_DLL_ALIAS(
				WSJTXReceiver::create, // <-- this function is exported with...
				CreatePlugin           // <-- ...this alias name
			   )

const char * MyName()
{
	return "WSJT-X";
}

BOOST_DLL_ALIAS(
				MyName, // <-- this function is exported with...
				Name    // <-- ...this alias name
			   )

const HamLab::APIVersion * MyVersion()
{
	return &HamLab::CURRENT_API_VERSION;
}

BOOST_DLL_ALIAS(
				MyVersion, // <-- this function is exported with...
				Version    // <-- ...this alias name
			   )
