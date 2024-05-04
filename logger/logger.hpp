#pragma once

#include "HamLabPlugin.hpp"

class Logger : public HamLab::PluginBase
{
	public:
		Logger(HamLab::DataShareClass & pDataShareIn, const std::string & name_in);
		~Logger();

		bool DrawSideBar(bool bOpen) override;
		void DrawTab() override {};

		static HamLab::PluginBase * create(HamLab::DataShareClass & data_share_, const std::string & name);

		virtual bool ShowTab() override { return false; }
};

