#pragma once

#include "HamLabPlugin.hpp"

#include <hamlib/rig.h>

struct MutexJSONCallback
{
		std::recursive_mutex & mtx;
		ojson::value & jData;
};

class RigControl : public HamLab::PluginBase
{
	public:
		RigControl(HamLab::DataShare & pDataShareIn, const std::string & name_in);
		~RigControl();

		bool DrawSideBar(bool bOpen) override;
		void DrawTab() override;

		static HamLab::PluginBase * create(HamLab::DataShare & data_share_, const std::string & name);

	private:
		std::recursive_mutex mtx;
		ojson::document jRigStatus;
		std::thread tStatus;
		bool bDone = false;
		bool bWakeUp = false;
		std::mutex cv_mtx;
		std::condition_variable cv;
		ojson::document jMyRigs;
		RIG * m_rig = nullptr;
		MutexJSONCallback mutexData {mtx, jRigStatus};
};

