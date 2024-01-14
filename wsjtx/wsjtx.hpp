#ifndef WSJTXRECEIVER_HPP
#define WSJTXRECEIVER_HPP

#include "HamLabPlugin.hpp"
#include "json.hpp"

#include <vector>
#include <thread>
// #include <mutex>
#include <boost/asio.hpp>

enum WSJT_X_Msg_Type
{
	WSJT_X_Msg_Heartbeat,
	WSJT_X_Msg_Status,
	WSJT_X_Msg_Decode,
	WSJT_X_Msg_Clear,
	WSJT_X_Msg_Reply,
	WSJT_X_Msg_QSOLogged,
	WSJT_X_Msg_Close,
	WSJT_X_Msg_Replay,
	WSJT_X_Msg_HaltTx,
	WSJT_X_Msg_FreeText,
	WSJT_X_Msg_WSPRDecode,
	WSJT_X_Msg_Location,
	WSJT_X_Msg_LoggedADIF,
	WSJT_X_Msg_HighlightCallsign,
	WSJT_X_Msg_SwitchConfiguration,
	WSJT_X_Msg_Configure,
	WSJT_X_Msg_maximum_message_type_     // ONLY add new message types
	// immediately before here
};


class WSJTXReceiver : public HamLab::PluginBase
{
	public:
		WSJTXReceiver(HamLab::DataShare & pDataShareIn, const std::string & name_in);
		~WSJTXReceiver();

		bool DrawSideBar(bool bOpen) override;
		void DrawTab() override;

		static HamLab::PluginBase* create(HamLab::DataShare &data_share_, const std::string & name);

	private:
		void Start();
		void Restart();
		void Stop();

		ojson::document history();

		void SendReply(ojson::document & jData);
		void Replay();

		void Send(const std::vector<unsigned char>& data);

		void StartReceive();
		void ProcessMessage(std::size_t bytesReceived);
		static ojson::document DecodeMessage(const std::vector<unsigned char> & vMessage);

		static constexpr std::size_t max_length = 0x10000;
		std::array<unsigned char, max_length> data_;
		boost::asio::io_context io_context_;
		std::thread io_thread_;
		boost::asio::ip::udp::socket socket_{io_context_};
		boost::asio::ip::udp::socket send_socket_{io_context_};
		boost::asio::ip::udp::endpoint senderEndpoint_;

		std::recursive_mutex mtx;
		ojson::document jHistory;

		// char szWSJTXSendAddress[256];
		// int send_port = 2237;
		// int listen_port = 2237;
		// bool bShowJSON = false;
};

#endif // WSJTXRECEIVER_HPP
