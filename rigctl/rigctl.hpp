#pragma once

#include "HamLabPlugin.hpp"

#include <hamlib/rig.h>

struct MutexJSONCallback
{
		std::recursive_mutex & mtx;
		json::value & jData;
};

class RigControl : public HamLab::PluginBase
{
	public:
		RigControl(HamLab::DataShareClass & pDataShareIn, const std::string & name_in);
		~RigControl();

		bool DrawSideBar(bool bOpen) override;
		void DrawTab() override;

		static HamLab::PluginBase * create(HamLab::DataShareClass & data_share_, const std::string & name);

	private:
		std::recursive_mutex mtx;
		json::document jRigStatus;
		std::thread tStatus;
		bool bDone = false;
		bool bWakeUp = false;
		std::mutex cv_mtx;
		std::condition_variable cv;
		json::document jMyRigs;
		RIG * m_rig = nullptr;
		MutexJSONCallback mutexData {mtx, jRigStatus};
};

