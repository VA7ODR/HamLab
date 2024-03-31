#pragma once

#include "imgui.h"
#include "json.hpp"

#include <boost/core/demangle.hpp>
#include <boost/stacktrace/stacktrace.hpp>
#include <cstdint>
#include <iterator>
#include <string>
#include <vector>
#include <string>
#include <chrono>
#include <format>
#include <cstring>

using namespace std::chrono_literals;

inline auto Now()
{
	return std::chrono::utc_clock::now();
}

inline auto SystemNow()
{
	return std::chrono::system_clock::now();
}

inline auto SteadyNow()
{
	return std::chrono::steady_clock::now();
}

inline std::string PrettyDateTime(auto time, const std::string_view formatIn = "{:%Y-%m-%d %H:%M:%S}")
{
	try {
		auto ret = std::vformat(formatIn, std::make_format_args(time));
		return ret;
	} catch (std::format_error & e) {
		return "Invalid time: " + std::string(e.what());
	}
}

inline std::string PrettyDate(auto time)
{
	return sPrettyDateTime(time, "{:%Y-%m-%d}");
}

inline std::string PrettyTime(auto time)
{
	return sPrettyDateTime(time, "{:%H:%M:%S}");
}

inline std::string PrettyDuration(auto duration = 0s, const std::string_view format = "{:%H:%M:%S}")
{
	return std::vformat(format, std::make_format_args(duration));
}

inline std::string PrettyDuration(auto start, auto end, const std::string_view format = "{:%H:%M:%S}")
{
	return PrettyDuration(end - start, format);
}

inline std::chrono::utc_clock::time_point UTCTime(auto time)
{
	return std::chrono::clock_cast<std::chrono::utc_clock>(time);
}

inline std::chrono::system_clock::time_point SystemTime(auto time = std::chrono::utc_clock::now())
{
	return std::chrono::clock_cast<std::chrono::system_clock>(time);
}

inline std::chrono::steady_clock::time_point SteadyTime(auto time = std::chrono::utc_clock::now())
{
	return std::chrono::clock_cast<std::chrono::steady_clock>(time);
}

inline void Asserter(bool bCondition, const std::string& sCondition, const std::string & sFile, const std::string & sFunction, int iLine) {
	if (!bCondition) {
		throw std::runtime_error(sCondition + " failed in " + sFile + " " + sFunction + " " + std::to_string(iLine));
	}
}

#define ASSERT(bCondition) Asserter(bCondition, #bCondition, SOURCE_FILE, __FUNCTION__, __LINE__)
#define SOURCE_FILE std::string(__FILE__).substr(strlen(SOURCE_DIR) + 1)

std::string GetAppDataFolder();

void ShowJsonWindow(const std::string & sTitle, ojson::value & jData, bool & bShow);

ojson::value ImVec4ToJSONArray(const ImVec4 & in);
ImVec4 JSONArrayToImVec4(ojson::value & jData);

template <typename T>
class jsonTypedRef
{
	public:
		// Constructor for bool
		jsonTypedRef(ojson::value & jValIn)
			requires std::is_same_v<T, bool>
			: jVal(jValIn)
		{
			m_val = jVal.boolean();
		}

		// Constructor for floating-point types
		jsonTypedRef(ojson::value & jValIn)
			requires std::is_floating_point_v<T>
			: jVal(jValIn)
		{
			m_val = jVal._double();
		}

		// Constructor for integral types (excluding bool)
		// Constructor for signed integral types
		jsonTypedRef(ojson::value & jValIn)
			requires std::is_integral_v<T> && std::is_signed_v<T> && (!std::is_same_v<T, bool>)
			: jVal(jValIn)
		{
			m_val = jVal._int64();
		}

		// Constructor for unsigned integral types
		jsonTypedRef(ojson::value & jValIn)
			requires std::is_integral_v<T> && std::is_unsigned_v<T> && (!std::is_same_v<T, bool>)
			: jVal(jValIn)
		{
			m_val = jVal._uint64();
		}

		// Constructor for char*/std::string
		jsonTypedRef(ojson::value & jValIn)
			requires std::is_same_v<T, char *> || std::is_same_v<T, std::string> || std::is_same_v<T, sdstring>
			: jVal(jValIn)
		{
			m_val = jVal.string().data();
		}

		~jsonTypedRef() { jVal = m_val; }

		operator T *()
			requires(!std::is_pointer_v<T>)
		{
			return &m_val;
		}

		operator T()
			requires(std::is_pointer_v<T>)
		{
			return m_val;
		}

		// operator T &()
		// 	requires(!std::is_pointer_v<T>)
		// {
		// 	return m_val;
		// }

		// operator T &()
		// 	requires(std::is_pointer_v<T>)
		// {
		// 	return *m_val;
		// }

	private:
		T m_val;
		ojson::value & jVal;
};

template <typename T>
T swapEndianness(T value)
{
	static_assert(std::is_arithmetic<T>::value, "Only arithmatic types are supported");
	union {
		T value;
		uint8_t bytes[sizeof(T)];
	} src, dest;

	src.value = value;

	for (size_t i = 0; i < sizeof(T); ++i) {
		dest.bytes[i] = src.bytes[sizeof(T) - i - 1];
	}

	return dest.value;
}

class HamLabException : public std::exception
{
private:
	std::string errorMessage;

public:
	HamLabException(const std::string & sFile, int iLine, const std::string& message) : errorMessage("HamLab Exception: (" + sFile.substr(sFile.find_last_of('/') + 1) + ":" + std::to_string(iLine) + "): " +message) {}

	const char* what() const noexcept override {
		return errorMessage.c_str();
	}
};

#define THROW_HAMLAB_EXCPETION(X) throw HamLabException(__FILE__, __LINE__, X)

#define HAMLIB_ASSERT(X, Y) if (!(X)) { THROW_HAMLAB_EXCPETION(std::string("Assertion Failed: (") + #X + "): " + Y); }

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
	void Bin2Num(ojson::value & val)
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

	void Bin2Str(ojson::value & val)
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
		value = swapEndianness(value);
		std::copy(reinterpret_cast<const unsigned char*>(&value), reinterpret_cast<const unsigned char*>(&value) + size, end_);
		end_ += size;
	}

	template <typename FloatingPointType>
	void FloatingPoint2Bin(FloatingPointType value) {
		static_assert(std::is_floating_point_v<FloatingPointType>, "Only floating-point types are supported.");
		size_t size = sizeof(value);
		value = swapEndianness(value);
		std::copy(reinterpret_cast<const unsigned char*>(&value), reinterpret_cast<const unsigned char*>(&value) + size, end_);
		end_ += size;
	}

	std::vector<unsigned char> data_;
	std::vector<unsigned char>::iterator end_{data_.begin()};
};

std::string Pretty(const std::vector<unsigned char> &in);
