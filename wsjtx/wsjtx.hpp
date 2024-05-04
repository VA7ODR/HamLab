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

class BinaryConverter {
public:
	BinaryConverter() = default;

	template <typename Iterator>
	BinaryConverter(Iterator beginIn, Iterator endIn) : begin(beginIn), it(beginIn), end(endIn) {}

	// Copy constructor
	BinaryConverter(const BinaryConverter& other) : begin(other.begin), it(other.it), end(other.end) {}

	// Move constructor
	BinaryConverter(BinaryConverter&& other) noexcept : begin(other.begin), it(other.it), end(other.end) {}

	// Copy assignment operator
	BinaryConverter& operator=(const BinaryConverter& other) {
		if (this != &other) {
			begin = other.begin;
			it = other.it;
			end = other.end;
		}
		return *this;
	}

	// Move assignment operator
	BinaryConverter& operator=(BinaryConverter&& other) noexcept {
		if (this != &other) {
			begin = other.begin;
			it = other.it;
			end = other.end;
		}
		return *this;
	}

	template <typename OT>
	OT Bin2Num() {
		static_assert(std::is_arithmetic_v<OT>, "Only integral and floating-point types are supported.");
		if constexpr (std::is_floating_point_v<OT>) {
			return Bin2FloatingPoint<OT>();
		} else {
			return Bin2Integral<OT>();
		}
	}

	template <typename OT>
	void Bin2Num(json::value & val)
	{
		static_assert(std::is_arithmetic_v<OT>, "Only integral and floating-point types are supported.");
		HandleCurrentTag handler(sCurrentTag, val.key());
		val = Bin2Num<OT>();
	}

	std::string Bin2Str() {
		auto len = Bin2Num<uint32_t>();
		if (len == 0xFFFFFFFF || len == 0) {
			return "";
		}
		HAMLIB_ASSERT(len <= static_cast<size_t>(std::distance(it, end)), "Bin2Str \"" + sCurrentTag + "\" (" + std::to_string(len) + " > " + std::to_string(std::distance(it, end)) + ")");
		std::string ret(it, it + len);
		std::advance(it, len);
		// std::cerr << "String: " << ret << std::endl;
		return ret;
	}

	void Bin2Str(json::value & val)
	{
		HandleCurrentTag handler(sCurrentTag, val.key());
		val = Bin2Str();
	}

	[[nodiscard]] std::vector<unsigned char>::const_iterator Begin() const
	{
		return begin;
	}

	[[nodiscard]] std::vector<unsigned char>::const_iterator Current() const
	{
		return it;
	}

	[[nodiscard]] std::vector<unsigned char>::const_iterator End() const
	{
		return end;
	}

private:

	class HandleCurrentTag{
		public:
			HandleCurrentTag(std::string & sTagIn, const std::string & sSetTo) : sTag(sTagIn)
			{
				sTag.assign(sSetTo);
			}
			~HandleCurrentTag() { sTag.assign("Unknown"); }

		private:
			std::string & sTag;
	};

	template <typename IntegralType>
	IntegralType Bin2Integral() {
		static_assert(std::is_integral_v<IntegralType>, "Only floating-point types are supported.");
		IntegralType ret = 0;
		// std::cout << boost::core::demangle(typeid(IntegralType).name()) << ": " << std::flush;
		HAMLIB_ASSERT(sizeof(IntegralType) <= static_cast<size_t>(std::distance(it, end)), "Bin2Num<" + boost::core::demangle(typeid(IntegralType).name()) + "> \"" + sCurrentTag + "\" (" + std::to_string(sizeof(IntegralType)) + " > " + std::to_string(std::distance(it, end)) + ")");
		for (size_t i = 0; i < sizeof(IntegralType); ++i) {
			ret = ret << 8;
			ret |= *it++;
		}
		// std::cout << ret << std::endl;
		return ret;
	}

	template <typename FloatingPointType>
	FloatingPointType Bin2FloatingPoint() {
		static_assert(std::is_floating_point_v<FloatingPointType>, "Only floating-point types are supported.");
		// std::cout << boost::core::demangle(typeid(FloatingPointType).name()) << ": " << std::flush;
		HAMLIB_ASSERT(sizeof(FloatingPointType) <= sizeof(uint64_t), "Bin2Num" + boost::core::demangle(typeid(FloatingPointType).name()));

		union {
			uint64_t i;
			FloatingPointType f;
		} ret;

		ret.i = 0;
		for (size_t i = 0; i < sizeof(FloatingPointType); ++i) {
			ret.i = ret.i << 8;
			ret.i |= *it++;
		}
		// std::cout << ret.f << std::endl;
		return ret.f;
	}

	std::vector<unsigned char>::const_iterator begin;
	std::vector<unsigned char>::const_iterator it;
	std::vector<unsigned char>::const_iterator end;

	std::string sCurrentTag{"Unknown"};
};

class BinarySerializer {
public:
	BinarySerializer() : data(kInitialSize), end(data.begin()) {}

	// Copy constructor
	BinarySerializer(BinarySerializer& other) : data(other.data), end(data.begin() + std::distance(other.data.begin(), other.end)) {}

	// Move constructor
	BinarySerializer(BinarySerializer&& other) noexcept
	{
		auto size = std::distance(other.data.begin(), other.end);
		data = std::move(other.data);
		end = data.begin() + size;
	}

	// Copy assignment operator
	BinarySerializer& operator=(const BinarySerializer& other) {
		if (this != &other) {
			data = other.data;
			end = data.begin() + (other.data.begin() - other.end);
		}
		return *this;
	}

	// Move assignment operator
	BinarySerializer& operator=(BinarySerializer&& other) noexcept {
		if (this != &other) {
			auto size = std::distance(other.data.begin(), other.end);
			data = std::move(other.data);
			end = data.begin() + size;
		}
		return *this;
	}

	template <typename OT>
	void Num2Bin(OT value) {
		static_assert(std::is_arithmetic_v<OT>, "Only integral and floating-point types are supported.");
		if constexpr (std::is_floating_point_v<OT>) {
			FloatingPoint2Bin(value);
		} else {
			Integral2Bin(value);
		}
	}

	void Str2Bin(const std::string& value) {
		Num2Bin<uint32_t>(static_cast<uint32_t>(value.size()));
		std::ranges::copy(value, end);
		end += static_cast<long>(value.size());
	}

	void ShrinkToFit() {
		auto final_size = std::distance(data.begin(), end);
		data.resize(final_size);
		end = data.end();
	}

	[[nodiscard]] std::vector<unsigned char>::const_iterator Begin() const {
		return data.begin();
	}

	[[nodiscard]] std::vector<unsigned char>::const_iterator End() const {
		return end;
	}

	auto & operator*()
	{
		ShrinkToFit();
		return data;
	}

private:
	static constexpr size_t kInitialSize = 16*1024;

	template <typename IntegralType>
	void Integral2Bin(IntegralType value) {
		static_assert(std::is_integral_v<IntegralType>, "Only integral types are supported.");
		size_t size = sizeof(value);
		value = Network::swapEndianness(value);
		std::copy_n(reinterpret_cast<const unsigned char*>(&value), size, end);
		end += static_cast<long>(size);
	}

	template <typename FloatingPointType>
	void FloatingPoint2Bin(FloatingPointType value) {
		static_assert(std::is_floating_point_v<FloatingPointType>, "Only floating-point types are supported.");
		size_t size = sizeof(value);
		value = Network::swapEndianness(value);
		std::copy_n(reinterpret_cast<const unsigned char*>(&value), size, end);
		end += static_cast<long>(size);
	}

	std::vector<unsigned char> data;
	std::vector<unsigned char>::iterator end{data.begin()};
};

class WSJTXReceiver : public HamLab::PluginBase
{
	public:
		WSJTXReceiver(HamLab::DataShareClass & pDataShareIn, const std::string & name_in);
		~WSJTXReceiver() override;

		bool DrawSideBar(bool bOpen) override;
		void DrawTab() override;

		static HamLab::PluginBase* create(HamLab::DataShareClass &data_share_, const std::string & name);

	private:
		void Start();
		void Restart();
		void Stop();

		json::document history();

		void SendReply(json::document & jData);
		void Replay();

		void Send(const std::vector<unsigned char>& data);

		void StartReceive();
		void ProcessMessage(std::size_t bytesReceived);
		static json::document DecodeMessage(const std::vector<unsigned char> & vMessage);

		static constexpr std::size_t MaxLength = 0x10000;
		std::array<unsigned char, MaxLength> data;
		boost::asio::io_context ioc;
		std::thread ioThread;
		boost::asio::ip::udp::socket socket_{ioc};
		boost::asio::ip::udp::socket send_socket_{ioc};
		boost::asio::ip::udp::endpoint senderEndpoint;

		std::recursive_mutex mtx;
		json::document jHistory;

		// char szWSJTXSendAddress[256];
		// int send_port = 2237;
		// int listen_port = 2237;
		// bool bShowJSON = false;
};

#endif // WSJTXRECEIVER_HPP
