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
	BinaryConverter() {}

	template <typename Iterator>
	BinaryConverter(Iterator begin, Iterator end) : begin_(begin), it_(begin), end_(end) {}

	// Copy constructor
	BinaryConverter(const BinaryConverter& other) : begin_(other.begin_), it_(other.it_), end_(other.end_) {}

	// Move constructor
	BinaryConverter(BinaryConverter&& other) noexcept : begin_(std::move(other.begin_)), it_(std::move(other.it_)), end_(std::move(other.end_)) {}

	// Copy assignment operator
	BinaryConverter& operator=(const BinaryConverter& other) {
		if (this != &other) {
			begin_ = other.begin_;
			it_ = other.it_;
			end_ = other.end_;
		}
		return *this;
	}

	// Move assignment operator
	BinaryConverter& operator=(BinaryConverter&& other) noexcept {
		if (this != &other) {
			begin_ = std::move(other.begin_);
			it_ = std::move(other.it_);
			end_ = std::move(other.end_);
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
		HAMLIB_ASSERT(len <= static_cast<size_t>(std::distance(it_, end_)), "Bin2Str \"" + sCurrentTag + "\" (" + std::to_string(len) + " > " + std::to_string(std::distance(it_, end_)) + ")");
		std::string ret(it_, it_ + len);
		std::advance(it_, len);
		// std::cerr << "String: " << ret << std::endl;
		return ret;
	}

	void Bin2Str(json::value & val)
	{
		HandleCurrentTag handler(sCurrentTag, val.key());
		val = Bin2Str();
	}

	std::vector<unsigned char>::const_iterator begin()
	{
		return begin_;
	}

	std::vector<unsigned char>::const_iterator current()
	{
		return it_;
	}

	std::vector<unsigned char>::const_iterator end()
	{
		return end_;
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
		HAMLIB_ASSERT(sizeof(IntegralType) <= static_cast<size_t>(std::distance(it_, end_)), "Bin2Num<" + boost::core::demangle(typeid(IntegralType).name()) + "> \"" + sCurrentTag + "\" (" + std::to_string(sizeof(IntegralType)) + " > " + std::to_string(std::distance(it_, end_)) + ")");
		for (size_t i = 0; i < sizeof(IntegralType); ++i) {
			ret = ret << 8;
			ret |= *it_++;
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
			ret.i |= *it_++;
		}
		// std::cout << ret.f << std::endl;
		return ret.f;
	}

	std::vector<unsigned char>::const_iterator begin_;
	std::vector<unsigned char>::const_iterator it_;
	std::vector<unsigned char>::const_iterator end_;

	std::string sCurrentTag{"Unknown"};
};

class BinarySerializer {
public:
	BinarySerializer() : data_(kInitialSize), end_(data_.begin()) {}

	// Copy constructor
	BinarySerializer(BinarySerializer& other) : data_(other.data_), end_(data_.begin() + std::distance(other.data_.begin(), other.end_)) {}

	// Move constructor
	BinarySerializer(BinarySerializer&& other) noexcept
	{
		auto size = std::distance(other.data_.begin(), other.end_);
		data_ = std::move(other.data_);
		end_ = data_.begin() + size;
	}

	// Copy assignment operator
	BinarySerializer& operator=(BinarySerializer& other) {
		if (this != &other) {
			data_ = other.data_;
			end_ = data_.begin() + std::distance(other.data_.begin(), other.end_);
		}
		return *this;
	}

	// Move assignment operator
	BinarySerializer& operator=(BinarySerializer&& other) noexcept {
		if (this != &other) {
			auto size = std::distance(other.data_.begin(), other.end_);
			data_ = std::move(other.data_);
			end_ = data_.begin() + size;
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
		std::copy(value.begin(), value.end(), end_);
		end_ += value.size();
	}

	void shrink_to_fit() {
		auto final_size = std::distance(data_.begin(), end_);
		data_.resize(final_size);
		end_ = data_.end();
	}

	std::vector<unsigned char>::const_iterator begin() const {
		return data_.begin();
	}

	std::vector<unsigned char>::const_iterator end() const {
		return end_;
	}

	auto & operator*()
	{
		shrink_to_fit();
		return data_;
	}

private:
	static constexpr size_t kInitialSize = 16*1024;

	template <typename IntegralType>
	void Integral2Bin(IntegralType value) {
		static_assert(std::is_integral_v<IntegralType>, "Only integral types are supported.");
		size_t size = sizeof(value);
		value = Network::swapEndianness(value);
		std::copy(reinterpret_cast<const unsigned char*>(&value), reinterpret_cast<const unsigned char*>(&value) + size, end_);
		end_ += size;
	}

	template <typename FloatingPointType>
	void FloatingPoint2Bin(FloatingPointType value) {
		static_assert(std::is_floating_point_v<FloatingPointType>, "Only floating-point types are supported.");
		size_t size = sizeof(value);
		value = Network::swapEndianness(value);
		std::copy(reinterpret_cast<const unsigned char*>(&value), reinterpret_cast<const unsigned char*>(&value) + size, end_);
		end_ += size;
	}

	std::vector<unsigned char> data_;
	std::vector<unsigned char>::iterator end_{data_.begin()};
};

class WSJTXReceiver : public HamLab::PluginBase
{
	public:
		WSJTXReceiver(HamLab::DataShareClass & pDataShareIn, const std::string & name_in);
		~WSJTXReceiver();

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

		static constexpr std::size_t max_length = 0x10000;
		std::array<unsigned char, max_length> data_;
		boost::asio::io_context io_context_;
		std::thread io_thread_;
		boost::asio::ip::udp::socket socket_{io_context_};
		boost::asio::ip::udp::socket send_socket_{io_context_};
		boost::asio::ip::udp::endpoint senderEndpoint_;

		std::recursive_mutex mtx;
		json::document jHistory;

		// char szWSJTXSendAddress[256];
		// int send_port = 2237;
		// int listen_port = 2237;
		// bool bShowJSON = false;
};

#endif // WSJTXRECEIVER_HPP
