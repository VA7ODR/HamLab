#ifndef WSJTXRECEIVER_HPP
#define WSJTXRECEIVER_HPP

//#include <QObject>


#include "json.hpp"

#include <iostream>
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


class WSJTXReceiver// : public QObject
{
		//		Q_OBJECT
	public:

		WSJTXReceiver(const std::string& send_address, const unsigned short send_port, const unsigned short listen_port) :
			address_(send_address),
			send_port_(send_port)
		{
			// Open the socket and join the multicast group
			boost::asio::ip::udp::endpoint endpoint_(boost::asio::ip::address::from_string("0.0.0.0"), listen_port);
			socket_.open(endpoint_.protocol());
			socket_.set_option(boost::asio::ip::udp::socket::reuse_address(true));
			socket_.bind(endpoint_);

			Start();

			StartReceive();
		}

		void Start() {
			if (!io_thread_.joinable()) {
				io_thread_ = std::thread([this] { io_context_.run(); });
			}
		}

		void Restart(const std::string& send_address, const unsigned short send_port, const unsigned short listen_port) {
			Stop();

			boost::asio::ip::udp::endpoint endpoint_(boost::asio::ip::address::from_string("0.0.0.0"), listen_port);
			if (socket_.is_open()) {
				socket_.close();
			}
			socket_.open(endpoint_.protocol());
			socket_.set_option(boost::asio::ip::udp::socket::reuse_address(true));
			socket_.bind(endpoint_);

			Start();
		}

		void Stop() {
			io_context_.stop();
			if (io_thread_.joinable()) {
				io_thread_.join();
			}
			io_context_.restart();
		}

		json::document history();

		void SendReply(json::document & jData);

		void Replay();

	private:
		void Send(const std::vector<unsigned char>& data) {
			boost::asio::ip::udp::resolver resolver(io_context_);
			boost::asio::ip::udp::resolver::results_type endpoints = resolver.resolve(address_, std::to_string(send_port_));
			boost::asio::ip::udp::endpoint sendEndpoint = *endpoints.begin();
			boost::asio::ip::udp::socket socket(io_context_);
			socket.open(boost::asio::ip::udp::v4());
			socket.set_option(boost::asio::ip::udp::socket::reuse_address(true));
			socket.set_option(boost::asio::socket_base::broadcast(true));
			socket.send_to(boost::asio::buffer(data), sendEndpoint);
		}

		void StartReceive() {
			socket_.async_receive_from(boost::asio::buffer(data_), senderEndpoint_, [this](const boost::system::error_code& error, std::size_t bytesReceived)
			{
				if (!error) {
					ProcessMessage(bytesReceived);
					StartReceive(); // Continue to listen for more messages
				} else {
					std::cerr << "Error receiving data: " << error.message() << std::endl;
				}
			});
		}

		void ProcessMessage(std::size_t bytesReceived);

		static json::document DecodeMessage(const std::vector<unsigned char> & vMessage);
		
		static constexpr std::size_t max_length = 0x10000;
		std::array<unsigned char, max_length> data_;
		boost::asio::io_context io_context_;
		std::thread io_thread_;
		boost::asio::ip::udp::socket socket_{io_context_};
		boost::asio::ip::udp::socket send_socket_{io_context_};
		boost::asio::ip::udp::endpoint senderEndpoint_;
		std::string address_;
		unsigned short send_port_;

		std::recursive_mutex mtx;

		json::document jHistory;
};

#endif // WSJTXRECEIVER_HPP
