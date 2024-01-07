#include "wsjtxreceiver.hpp"
#include "utils.hpp"

json::document WSJTXReceiver::history()
{
	std::lock_guard<std::recursive_mutex> lk(mtx);
	return jHistory;
}

void WSJTXReceiver::SendReply(json::document &jData)
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

void WSJTXReceiver::ProcessMessage(std::size_t bytesReceived) {
	std::vector<unsigned char> message(data_.begin(), data_.begin() + bytesReceived);

	// Decode the received WSJTX message
	auto jData = DecodeMessage(message);

	// std::cout << jData << std::endl;

	std::lock_guard<std::recursive_mutex> lk(mtx);
	std::string sType = jData["type_name"].string();
	auto & jTypeHistory = jHistory[sType];
	jTypeHistory.toArray();
	jTypeHistory.push_front(jData);



	while (jTypeHistory.size() > 50) {
		jTypeHistory.pop_back();
	}

	jHistory.writeFile("jHistory.json", true);
	// // Print the received message
	// std::cout << "Received message size: " << message.size() << " bytes" << std::endl;
}

json::document WSJTXReceiver::DecodeMessage(const std::vector<unsigned char> &vMessage)
{
	json::document jRet;

	jRet["type_name"] = "Unknown";

	if (vMessage.size() < 16) {
		jRet["error"] = true;
		jRet["error_message"] = "WSJT-X: Datagram Too Short.";
		return jRet;
	}

	std::cout << Pretty(vMessage) << std::endl;

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
				jRet["type_name"] = "Status";
				jRet["freq"] = binaryConverter.Bin2Num<uint64_t>();
				jRet["mode"] = binaryConverter.Bin2Str();
				jRet["dx_call"] = binaryConverter.Bin2Str();
				jRet["report"] = binaryConverter.Bin2Str();
				jRet["tx_mode"] = binaryConverter.Bin2Str();
				jRet["tx_enabled"] = binaryConverter.Bin2Num<bool>();
				jRet["transmitting"] = binaryConverter.Bin2Num<bool>();
				jRet["decoding"] = binaryConverter.Bin2Num<bool>();
				jRet["rx_df"] = binaryConverter.Bin2Num<uint32_t>();
				jRet["tx_df"] = binaryConverter.Bin2Num<uint32_t>();
				jRet["call"] = binaryConverter.Bin2Str();
				jRet["de_grid"] = binaryConverter.Bin2Str();
				jRet["dx_grid"] = binaryConverter.Bin2Str();
				jRet["watchdog_timeout"] = binaryConverter.Bin2Num<bool>();
				jRet["sub_mode"] = binaryConverter.Bin2Str();
				jRet["fast_mode"] = binaryConverter.Bin2Num<bool>();
				jRet["special_op_mode"] = binaryConverter.Bin2Num<uint8_t>();
				jRet["frequency_tolerance"] = binaryConverter.Bin2Num<uint32_t>();
				jRet["tr_period"] = binaryConverter.Bin2Num<uint32_t>();
				jRet["configuration_name"] = binaryConverter.Bin2Str();
				jRet["tx_message"] = binaryConverter.Bin2Str();

				break;
			}

			case WSJT_X_Msg_Decode:
			{
				jRet["type_name"] = "Decode";
				jRet["is_new"] = binaryConverter.Bin2Num<bool>();
				jRet["time"] = binaryConverter.Bin2Num<uint32_t>();
				jRet["snr"] = binaryConverter.Bin2Num<int32_t>();
				jRet["delta_time"] = binaryConverter.Bin2Num<double>();
				jRet["delta_frequency"] = binaryConverter.Bin2Num<uint32_t>();
				jRet["mode"] = binaryConverter.Bin2Str();
				jRet["message_text"] = binaryConverter.Bin2Str();
				jRet["low_confidence"] = binaryConverter.Bin2Num<bool>();
				jRet["off_air"] = binaryConverter.Bin2Num<bool>();

				break;
			}

			case WSJT_X_Msg_Clear:
			{
				jRet["type_name"] = "Clear";
				jRet["window"] = binaryConverter.Bin2Num<uint8_t>();
			}

			case WSJT_X_Msg_Reply:
			{
				jRet["type_name"] = "Reply";
				jRet["time"] = binaryConverter.Bin2Num<uint32_t>();
				jRet["snr"] = binaryConverter.Bin2Num<int32_t>();
				jRet["delta_time"] = binaryConverter.Bin2Num<double>();
				jRet["delta_frequency"] = binaryConverter.Bin2Num<uint32_t>();
				jRet["mode"] = binaryConverter.Bin2Str();
				jRet["message_text"] = binaryConverter.Bin2Str();
				jRet["low_confidence"] = binaryConverter.Bin2Num<bool>();
				jRet["modifiers"] = binaryConverter.Bin2Num<uint8_t>();

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
				jRet["date_off"] = binaryConverter.Bin2Num<uint64_t>();
				jRet["time_off"] = binaryConverter.Bin2Num<uint32_t>();
				jRet["timespec_off"] = binaryConverter.Bin2Num<uint8_t>();
				// jRet["offset_off"] = binaryConverter.Bin2Num<uint32_t>();

				// std::cout << Pretty({binaryConverter.begin(), binaryConverter.end()}) << std::endl;
				// std::cout << jRet.write(true) << std::endl;

				jRet["dx_call"] = binaryConverter.Bin2Str();
				jRet["dx_grid"] = binaryConverter.Bin2Str();
				jRet["tx_freq"] = binaryConverter.Bin2Num<uint64_t>();
				jRet["mode"] = binaryConverter.Bin2Str();
				jRet["report_sent"] = binaryConverter.Bin2Str();
				jRet["report_rcvd"] = binaryConverter.Bin2Str();
				jRet["tx_pwr"] = binaryConverter.Bin2Str();
				jRet["comments"] = binaryConverter.Bin2Str();
				jRet["name"] = binaryConverter.Bin2Str();
				jRet["date_on"] = binaryConverter.Bin2Num<uint64_t>();
				jRet["time_on"] = binaryConverter.Bin2Num<uint32_t>();
				jRet["timespec_on"] = binaryConverter.Bin2Num<uint8_t>();
				jRet["offset_on"] = binaryConverter.Bin2Num<uint32_t>();
				jRet["operator_call"] = binaryConverter.Bin2Str();
				jRet["my_call"] = binaryConverter.Bin2Str();
				jRet["my_grid"] = binaryConverter.Bin2Str();
				jRet["exch_sent"] = binaryConverter.Bin2Str();
				jRet["exch_rcvd"] = binaryConverter.Bin2Str();
				jRet["adif_prop"] = binaryConverter.Bin2Str();

				break;
			}

			case WSJT_X_Msg_WSPRDecode:
			{
				jRet["type_name"] = "WSPRDecode";
				jRet["is_new"] = binaryConverter.Bin2Num<bool>();
				jRet["time"] = binaryConverter.Bin2Num<uint32_t>();
				jRet["snr"] = binaryConverter.Bin2Num<int32_t>();
				jRet["delta_time"] = binaryConverter.Bin2Num<double>();
				jRet["frequency"] = binaryConverter.Bin2Num<uint64_t>();
				jRet["drift"] = binaryConverter.Bin2Num<uint32_t>();
				jRet["callsign"] = binaryConverter.Bin2Str();
				jRet["grid"] = binaryConverter.Bin2Str();
				jRet["power"] = binaryConverter.Bin2Num<uint32_t>();
				jRet["off_air"] = binaryConverter.Bin2Num<bool>();
			}

			case WSJT_X_Msg_LoggedADIF:
			{
				jRet["type_name"] = "LoggedADIF";
				jRet["adif_text"] = binaryConverter.Bin2Str();

				break;
			}

			default:
				jRet["error"] = true;
				jRet["error_message"] = "WSJT-X: Unhandled message type.";
				break;
		}

		if (binaryConverter.current() != binaryConverter.end()) {
			jRet["remaining"] = Pretty({binaryConverter.begin(), binaryConverter.end()});
		}
	} catch (const HamLabException& e) {
		std::cerr << "WSJTXReceiver Error decoding " << jRet["type_name"].string() << "message:" << std::endl;
		std::cerr << "    What: " << e.what() << std::endl;
		std::cerr << "    Done: " << Pretty({binaryConverter.begin(), binaryConverter.current()}) << std::endl;
		std::cerr << "    Left: " << Pretty({binaryConverter.current(), binaryConverter.end()}) << std::endl;
		std::cerr << "    JSON:\n" << jRet.write(true) << std::endl;
		jRet.destroy();
	}

	return jRet;
}
