#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/config.hpp>
#include <memory>
#include <vector>

#include "eventhandler.hpp"
#include "thread.hpp"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

using namespace std::chrono_literals;

namespace Network
{
	std::string URLEncode(const std::string & in);
	std::string URLDecode(const std::string & in);

	class CoreBase
	{
		public:
			CoreBase(int threadCountIn);
			~CoreBase();

			void Exit();
			void WakeUp();
			net::io_context & IOContext();
			const std::string & Certificates() const;

		protected:
			net::io_context ioc;
			EventHandler::Event eWakeUp = EventHandler::CreateEvent("HTTP::Core::WakeUp", EventHandler::auto_reset);
			EventHandler::Event eExit = EventHandler::CreateEvent("HTTP::Core::Exit", EventHandler::manual_reset);
			std::vector<std::shared_ptr<Thread>> vThreads;
			std::string sCertificates;
			bool bExit = false;
	};

	using core_t = std::shared_ptr<CoreBase>;
	core_t & Core(int iThreadCountInit = 1);
	core_t & Init(int threadCountIn);

	namespace HTTP
	{
		using verb = http::verb;
		using request_t = std::shared_ptr<http::request<http::string_body>>;
		using response_t = std::shared_ptr<http::response<http::string_body>>;
		using handler_t = std::function<void(request_t req, response_t res, const std::string & sRremoteAddr, int iRemotePort)>;

		class ClientBase : public std::enable_shared_from_this<ClientBase>
		{
			public:
				ClientBase(const std::string & sAddressIn, int iPortIn, bool bSSLIn, bool bAllowSelfSignedIn = false);
				~ClientBase();

				void KeepAlive(bool bKeepAliveIn);
				bool KeepAlive();

				virtual bool Connected();
				virtual void Close();

				void Request(request_t reqIn, handler_t handlerIn, std::chrono::seconds timeout = 30s, bool bKeepAliveIn = false);

				void Head(const std::string & sPath, handler_t handlerIn, std::chrono::seconds timeout = 30s, bool bKeepAliveIn = false);
				void Get(const std::string & sPath, handler_t handlerIn, std::chrono::seconds timeout = 30s, bool bKeepAliveIn = false);
				void Put(const std::string & sPath, const std::string & sBody, const std::string & sContentType, handler_t handlerIn, std::chrono::seconds timeout = 30s, bool bKeepAliveIn = false);
				void Post(const std::string & sPath, const std::string & sBody, const std::string & sContentType, handler_t handlerIn, std::chrono::seconds timeout = 30s, bool bKeepAliveIn = false);
				void Delete(const std::string & sPath, handler_t handlerIn, std::chrono::seconds timeout = 30s, bool bKeepAliveIn = false);

			protected:
				beast::tcp_stream & tcp_stream();
				beast::ssl_stream<beast::tcp_stream> & ssl_stream();
				virtual void PrepStream();

				void do_resolve();
				virtual void do_connect();
				void do_handshake();
				void do_write();
				void do_read();

				core_t core;
				std::string sAddress;
				int iPort = 0;
				ssl::context ctx{ssl::context::tlsv12_client};
				tcp::resolver resolver;
				tcp::resolver::results_type resolve_results;
				handler_t handler;
				bool bKeepAlive = false;
				std::mutex mtx;
				request_t req;
				response_t res;
				beast::flat_buffer buffer;
				bool bThreadExited = false;

				bool bSSL= true;
				bool bAllowSelfSigned = false;
				std::shared_ptr<beast::ssl_stream<beast::tcp_stream>> stream;
				boost::asio::ssl::context ssl_ctx{boost::asio::ssl::context::tlsv12_client};
		};

		using client_t = std::shared_ptr<ClientBase>;

		client_t Client(const std::string & sAddress, int iPort, bool bSSLIn = true, bool bAllowSelfSignedIn = false);

	} // HTTP
} // Network
#define HTTP_HANDLER_LAMBDA [&](Network::HTTP::request_t req, Network::HTTP::response_t res, const std::string & sRremoteAddr, int iRemotePort)
