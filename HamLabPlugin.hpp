#pragma once

#include "json.hpp"
#include "shared_recursive_mutex.hpp"
#include "easyappbase.hpp"

#include <string>

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

namespace HamLab
{
	struct APIVersion {
		constexpr APIVersion(int majorIn, int minorIn, int patchIn) : major(majorIn), minor(minorIn), patch(patchIn) {}

		int major;
		int minor;
		int patch;

		[[nodiscard]] const char* toString() const;
	};

	constexpr APIVersion CURRENT_API_VERSION{0, 0, 1};

	std::ostream & operator<<(std::ostream & os, const APIVersion & version);

	class DataShareClass
	{
		public:
			explicit DataShareClass(const std::string & sFileNameIn);
			explicit DataShareClass(std::string && sFileNameIn);
			~DataShareClass();

			json::value GetData(const std::string & sKey, const json::value & jDefault);
			json::value GetData(const std::string & sKey);
			void SetData(const std::string & sKey, const json::value & jValue);

		private:
			SharedRecursiveMutex mtx;
			std::string sFileName;
			json::document jData;
	};

	class PluginBase
	{
		public:
			PluginBase(DataShareClass & dataShareIn, const std::string & sNameIn);
			PluginBase(DataShareClass & dataShareIn, std::string && sNameIn);
			virtual ~PluginBase();

			constexpr const std::string & Name() { return sName; }

			static const APIVersion * Version() { return &CURRENT_API_VERSION; }

			virtual void DrawTab() = 0;
			virtual bool DrawSideBar(bool) = 0;

			virtual bool ShowTab() { return true; }
			virtual bool ShowSideBar() { return true; }

		protected:
			DataShareClass & dataShare;
			std::string sName;
			json::document jLocalData;
	};
} //HamLab
