#include "rigctl.hpp"

#include <hamlib/rig.h>

// #include "utils.hpp"
#include "json.hpp"
#include "utils.hpp"

#include <boost/dll/alias.hpp>
json::document jRigs;
json::document jLog;

std::string PrettyFreq(freq_t freq)
{
	if (freq != 0.0) {
		std::string sBreakpoint;
	}
	std::string sRet = std::to_string((int)freq);
	size_t p = sRet.find('.');
	int iStart = sRet.size() - 3;
	int iEnd = 0;
	if (p != std::string::npos) {
		iEnd = p + 1;
	}
	for (int i = iStart; i > iEnd; i -= 3) {
		sRet.insert(i, ".");
	}
	return sRet;
}

const std::map<std::string, rig_debug_level_e> debug_levels = {
	{"RIG_DEBUG_NONE", RIG_DEBUG_NONE},		  {"RIG_DEBUG_BUG", RIG_DEBUG_BUG},		{"RIG_DEBUG_ERR", RIG_DEBUG_ERR},	  {"RIG_DEBUG_WARN", RIG_DEBUG_WARN},
	{"RIG_DEBUG_VERBOSE", RIG_DEBUG_VERBOSE}, {"RIG_DEBUG_TRACE", RIG_DEBUG_TRACE}, {"RIG_DEBUG_CACHE", RIG_DEBUG_CACHE},
};

const std::vector<std::string> level_descriptions = {
	"PREAMP",
	"ATT",
	"VOXDELAY",
	"AF",
	"RF",
	"SQL",
	"IF",
	"APF",
	"NR",
	"PBT_IN",
	"PBT_OUT",
	"CWPITCH",
	"RFPOWER",
	"MICGAIN",
	"KEYSPD",
	"NOTCHF",
	"COMP",
	"AGC",
	"BKINDL",
	"BALANCE",
	"METER",
	"VOXGAIN",
	"ANTIVOX",
	"SLOPE_LOW",
	"SLOPE_HIGH",
	"BKIN_DLYMS",
	"RAWSTR",
	"--",
	"SWR",
	"ALC",
	"STRENGTH",
	"--",
	"RFPOWER_METER",
	"COMP_METER",
	"VD_METER",
	"ID_METER",
	"NOTCHF_RAW",
	"MONITOR_GAIN",
	"NB",
	"RFPOWER_METER_WATTS",
	"SPECTRUM_MODE",
	"SPECTRUM_SPAN",
	"SPECTRUM_EDGE_LOW",
	"SPECTRUM_EDGE_HIGH",
	"SPECTRUM_SPEED",
	"SPECTRUM_REF",
	"SPECTRUM_AVG",
	"SPECTRUM_ATT",
	"TEMP_METER",
	"BAND_SELECT",
	"USB_AF",
	"AGC_TIME",
};
std::string ModeString(rmode_t mode)
{
	std::string sMode;

	if (mode & RIG_MODE_AM) {
		sMode.append("AM ");
	}
	if (mode & RIG_MODE_CW) {
		sMode.append("CW ");
	}
	if (mode & RIG_MODE_USB) {
		sMode.append("USB ");
	}
	if (mode & RIG_MODE_LSB) {
		sMode.append("LSB ");
	}
	if (mode & RIG_MODE_RTTY) {
		sMode.append("RTTY ");
	}
	if (mode & RIG_MODE_FM) {
		sMode.append("FM ");
	}
	if (mode & RIG_MODE_WFM) {
		sMode.append("WFM ");
	}
	if (mode & RIG_MODE_CWR) {
		sMode.append("CWR ");
	}
	if (mode & RIG_MODE_RTTYR) {
		sMode.append("RTTYR ");
	}
	if (mode & RIG_MODE_AMS) {
		sMode.append("AMS ");
	}
	if (mode & RIG_MODE_PKTLSB) {
		sMode.append("PKTLSB ");
	}
	if (mode & RIG_MODE_PKTUSB) {
		sMode.append("PKTUSB ");
	}
	if (mode & RIG_MODE_PKTFM) {
		sMode.append("PKTFM ");
	}
	if (mode & RIG_MODE_ECSSUSB) {
		sMode.append("ECSSUSB ");
	}
	if (mode & RIG_MODE_ECSSLSB) {
		sMode.append("ECSSLSB ");
	}
	if (mode & RIG_MODE_FAX) {
		sMode.append("FAX ");
	}
	if (mode & RIG_MODE_SAM) {
		sMode.append("SAM ");
	}
	if (mode & RIG_MODE_SAL) {
		sMode.append("SAL ");
	}
	if (mode & RIG_MODE_SAH) {
		sMode.append("SAH ");
	}
	if (mode & RIG_MODE_DSB) {
		sMode.append("DSB ");
	}
	if (mode & RIG_MODE_FMN) {
		sMode.append("FMN ");
	}
	if (mode & RIG_MODE_PKTAM) {
		sMode.append("PKTAM ");
	}
	if (mode & RIG_MODE_P25) {
		sMode.append("P25 ");
	}
	if (mode & RIG_MODE_DSTAR) {
		sMode.append("DSTAR ");
	}
	if (mode & RIG_MODE_DPMR) {
		sMode.append("DPMR ");
	}
	if (mode & RIG_MODE_NXDNVN) {
		sMode.append("NXDNVN ");
	}
	if (mode & RIG_MODE_NXDN_N) {
		sMode.append("NXDN_N ");
	}
	if (mode & RIG_MODE_DCR) {
		sMode.append("DCR ");
	}
	if (mode & RIG_MODE_AMN) {
		sMode.append("AMN ");
	}
	if (mode & RIG_MODE_PSK) {
		sMode.append("PSK ");
	}
	if (mode & RIG_MODE_PSKR) {
		sMode.append("PSKR ");
	}
	if (mode & RIG_MODE_DD) {
		sMode.append("DD ");
	}
	if (mode & RIG_MODE_C4FM) {
		sMode.append("C4FM ");
	}
	if (mode & RIG_MODE_PKTFMN) {
		sMode.append("PKTFMN ");
	}
	if (mode & RIG_MODE_SPEC) {
		sMode.append("SPEC ");
	}
	if (mode & RIG_MODE_CWN) {
		sMode.append("CWN ");
	}
	if (mode & RIG_MODE_IQ) {
		sMode.append("IQ ");
	}
	if (mode & RIG_MODE_ISBUSB) {
		sMode.append("ISBUSB ");
	}
	if (mode & RIG_MODE_ISBLSB) {
		sMode.append("ISBLSB ");
	}

	return sMode;
}

int rig_logger(enum rig_debug_level_e level, rig_ptr_t, const char * fmt, va_list args)
{
	// Get the current time_point
	auto currentTime = std::chrono::system_clock::now();

	// Convert the current time_point to time_t
	std::time_t currentTime_t = std::chrono::system_clock::to_time_t(currentTime);

	// Get the local time
	std::tm * localTimeInfo = std::localtime(&currentTime_t);
	std::ostringstream localTimeStream;
	localTimeStream << std::put_time(localTimeInfo, "%Y-%m-%d %H:%M:%S");

	// Get the GMT time
	std::tm * gmtTimeInfo = std::gmtime(&currentTime_t);
	std::ostringstream gmtTimeStream;
	gmtTimeStream << std::put_time(gmtTimeInfo, "%Y-%m-%d %H:%M:%S");

	// Print the local time and GMT time
	// std::cout << "Local Time: " << localTimeStream.str() << std::endl;
	// std::cout << "GMT Time: " << gmtTimeStream.str() << std::endl;

	char buf[256];
	memset(buf, 0, sizeof(buf));
	int w = vsnprintf(buf, sizeof(buf) - 1, fmt, args);
	json::value jThisLog;
	jThisLog["level"] = (int)level;
	jThisLog["local_time"] = localTimeStream.str();
	jThisLog["universal_time"] = gmtTimeStream.str();

	jThisLog["message"] = std::string(buf, w);
	jLog.push_back(jThisLog);
	return 1;
}

json::document createRigComboBoxMap()
{
	// Initialize Hamlib
	rig_set_debug(RIG_DEBUG_NONE); // Set debug level if needed

	auto callback2 = [](const struct rig_caps * caps, rig_ptr_t rigp)
	{
		RIG * rig = (RIG *)rigp;

		rig = rig_init(caps->rig_model);

		if (!rig) {
			std::cerr << "Unknown rig num: " << caps->rig_model << std::endl;
			std::cerr << "Please check riglist.h" << std::endl;
			return 0; /* whoops! something went wrong (mem alloc?) */
		}

		jRigs[caps->mfg_name][caps->model_name]["rig_model"] = caps->rig_model;
		jRigs[caps->mfg_name][caps->model_name]["async_data_supported"] = caps->async_data_supported;
		jRigs[caps->mfg_name][caps->model_name]["serial_data_bits"] = caps->serial_data_bits;
		jRigs[caps->mfg_name][caps->model_name]["serial_rate_max"] = caps->serial_rate_max;
		jRigs[caps->mfg_name][caps->model_name]["serial_rate_min"] = caps->serial_rate_min;
		jRigs[caps->mfg_name][caps->model_name]["serial_parity"] = (int)caps->serial_parity;
		jRigs[caps->mfg_name][caps->model_name]["serial_handshake"] = (int)caps->serial_handshake;
		jRigs[caps->mfg_name][caps->model_name]["serial_stop_bits"] = (int)caps->serial_stop_bits;

		auto * cfgparams = caps->cfgparams;
		while (cfgparams && cfgparams->label) {
			jRigs[caps->mfg_name][caps->model_name]["cfgparams"][cfgparams->name]["label"] = cfgparams->label;
			jRigs[caps->mfg_name][caps->model_name]["cfgparams"][cfgparams->name]["name"] = cfgparams->name;
			jRigs[caps->mfg_name][caps->model_name]["cfgparams"][cfgparams->name]["dflt"] = cfgparams->dflt;
			jRigs[caps->mfg_name][caps->model_name]["cfgparams"][cfgparams->name]["tooltip"] = cfgparams->tooltip;
			jRigs[caps->mfg_name][caps->model_name]["cfgparams"][cfgparams->name]["type"] = (int)cfgparams->type;
			jRigs[caps->mfg_name][caps->model_name]["cfgparams"][cfgparams->name]["token"] = cfgparams->token;
			if (cfgparams->type == RIG_CONF_NUMERIC) {
				jRigs[caps->mfg_name][caps->model_name]["cfgparams"][cfgparams->name]["min"] = cfgparams->u.n.min;
				jRigs[caps->mfg_name][caps->model_name]["cfgparams"][cfgparams->name]["max"] = cfgparams->u.n.max;
				jRigs[caps->mfg_name][caps->model_name]["cfgparams"][cfgparams->name]["step"] = cfgparams->u.n.step;
			} else if (cfgparams->type == RIG_CONF_COMBO) {
				for (int i = 0; i < RIG_COMBO_MAX && cfgparams->u.c.combostr[i]; ++i) {
					jRigs[caps->mfg_name][caps->model_name]["cfgparams"][cfgparams->name]["combo_value"].push_back(cfgparams->u.c.combostr[i]);
				}
			}
			cfgparams += sizeof(confparams);
		}

		jRigs[caps->mfg_name][caps->model_name]["rig_port_type"] = (int)rig->state.rigport.type.rig;
		jRigs[caps->mfg_name][caps->model_name]["ptt_port_type"] = (int)rig->state.rigport.type.ptt;
		jRigs[caps->mfg_name][caps->model_name]["dcd_port_type"] = (int)rig->state.rigport.type.dcd;
		jRigs[caps->mfg_name][caps->model_name]["has_get_func"] = (int)rig->state.has_get_func;
		jRigs[caps->mfg_name][caps->model_name]["has_set_func"] = (int)rig->state.has_set_func;
		jRigs[caps->mfg_name][caps->model_name]["has_get_level"] = (int)rig->state.has_get_level;
		jRigs[caps->mfg_name][caps->model_name]["has_set_level"] = (int)rig->state.has_set_level;
		jRigs[caps->mfg_name][caps->model_name]["has_get_parm"] = (int)rig->state.has_get_parm;
		jRigs[caps->mfg_name][caps->model_name]["has_set_parm"] = (int)rig->state.has_set_parm;
		jRigs[caps->mfg_name][caps->model_name]["power_min"] = (int)rig->state.power_min;
		jRigs[caps->mfg_name][caps->model_name]["power_max"] = (int)rig->state.power_max;

		rig_cleanup(rig);
		return 1;
	};

	// Load all available backends
	jRigs.clear();
	using namespace std::placeholders;
	if (rig_load_all_backends() == RIG_OK) {
		RIG rig;
		rig_list_foreach(callback2, &rig);
	} else {
		std::cerr << "Failed to load backends from Hamlib.\n";
	}

	return jRigs;
}

RigControl::RigControl(HamLab::DataShareClass & pDataShareIn, const std::string & name_in) : HamLab::PluginBase(pDataShareIn, name_in)
{
	jMyRigs = createRigComboBoxMap();
	jLocalData["current_rig"]["available_settings"].clear();

	rig_set_debug_callback(rig_logger, nullptr);

	tStatus = std::thread(
		[&]()
	{
		while (!bDone) {
			auto start = std::chrono::steady_clock::now();
			if (m_rig) {
				if (m_rig->state.comm_state) {
					rig_set_vfo_callback(
						m_rig,
						[](RIG * rig, vfo_t vfo, rig_ptr_t pData)
					{
						MutexJSONCallback & mutexData = *(MutexJSONCallback *)pData;
						std::lock_guard lk(mutexData.mtx);
						mutexData.jData["VFO"] = vfo;
						return 1;
					},
						&mutexData);

					rig_set_spectrum_callback(
						m_rig,
						[](RIG * rig, struct rig_spectrum_line * line, rig_ptr_t pData)
					{
						MutexJSONCallback & mutexData = *(MutexJSONCallback *)pData;
						std::lock_guard lk(mutexData.mtx);
						auto & spectrum = mutexData.jData["spectrum"];
						spectrum["data_level_min"] = line->data_level_min;
						spectrum["data_level_max"] = line->data_level_max;
						spectrum["signal_strength_min"] = line->signal_strength_min;
						spectrum["spectrum_mode"] = line->spectrum_mode;
						spectrum["center_freq"] = line->center_freq;
						spectrum["span_freq"] = line->span_freq;
						spectrum["low_edge_freq"] = line->low_edge_freq;
						spectrum["high_edge_freq"] = line->high_edge_freq;
						spectrum["spectrum_data_length"] = line->spectrum_data_length;
						spectrum["line"] = std::string((char *)line->spectrum_data, (size_t)line->spectrum_data_length);
						return 1;
					},
						&mutexData);

					rig_set_freq_callback(
						m_rig,
						[](RIG * rig, vfo_t vfo, freq_t freq, rig_ptr_t pData)
					{
						MutexJSONCallback & mutexData = *(MutexJSONCallback *)pData;
						std::lock_guard lk(mutexData.mtx);
						mutexData.jData["VFO"]["freq"] = freq;
						return 1;
					},
						&mutexData);

					rig_set_mode_callback(
						m_rig,
						[](RIG * rig, vfo_t vfo, rmode_t mode, pbwidth_t width, rig_ptr_t pData)
					{
						MutexJSONCallback & mutexData = *(MutexJSONCallback *)pData;
						std::lock_guard lk(mutexData.mtx);
						mutexData.jData["VFO"]["mode"] = mode;
						mutexData.jData["VFO"]["width"] = width;
						return 1;
					},
						&mutexData);

					rig_set_ptt_callback(
						m_rig,
						[](RIG * rig, vfo_t vfo, ptt_t ptt, rig_ptr_t pData)
					{
						MutexJSONCallback & mutexData = *(MutexJSONCallback *)pData;
						std::lock_guard lk(mutexData.mtx);
						mutexData.jData["VFO"]["ptt"] = ptt;
						return 1;
					},
						&mutexData);

					freq_t freq = 0;
					rmode_t mode = RIG_MODE_NONE;
					pbwidth_t width = 0;
					split_t split;
					int satmode = 0;

					int retval = rig_get_vfo_info(m_rig, RIG_VFO_CURR, &freq, &mode, &width, &split, &satmode);

					{
						std::lock_guard lk(mtx);

						if (retval == RIG_OK) {
							std::lock_guard lk(mtx);
							jRigStatus["VFO"]["freq"] = freq;
							jRigStatus["VFO"]["mode"] = mode;
							jRigStatus["VFO"]["width"] = width;
							jRigStatus["VFO"]["split"] = split;
							jRigStatus["VFO"]["satmode"] = satmode;
						}
					}

					value_t strength;

					const struct rig_caps * caps = m_rig->caps;

					retval = rig_get_strength(m_rig, RIG_VFO_CURR, &strength);
					{
						std::lock_guard lk(mutexData.mtx);

						if (retval == RIG_OK) {
							std::lock_guard lk(mtx);
							jRigStatus["strength"] = strength.i;
						}
					}

					if (caps->get_level != nullptr) {
						uint64_t level = 1;
						for (int i = 0; i < 64; ++i) {
							if (rig_has_get_level(m_rig, level)) {
								retval = rig_get_level(m_rig, RIG_VFO_CURR, level, &strength);
								if (retval == RIG_OK) {
									unsigned int mwpower;
									retval = rig_power2mW(m_rig, &mwpower, strength.f, jRigStatus["VFO"]["freq"]._double(), jRigStatus["VFO"]["mode"]._int());
									if (retval == RIG_OK) {
										std::lock_guard lk(mtx);
										jRigStatus["levels"][level_descriptions[i]] = mwpower;
									}
								}
							}
							level = level << 1;
						}
					}

					retval = rig_get_level(m_rig, RIG_VFO_CURR, RIG_LEVEL_RAWSTR, &strength);
					if (retval == RIG_OK) {
						std::lock_guard lk(mtx);
						jRigStatus["raw_strength"] = strength.i;
					}

					retval = rig_get_level(m_rig, RIG_VFO_CURR, RIG_LEVEL_SWR, &strength);
					if (retval == RIG_OK) {
						std::lock_guard lk(mtx);
						jRigStatus["swr"] = strength.i;
					}

					retval = rig_get_level(m_rig, RIG_VFO_CURR, RIG_LEVEL_ALC, &strength);
					if (retval == RIG_OK) {
						std::lock_guard lk(mtx);
						jRigStatus["alc"] = strength.i;
					}

					retval = rig_get_level(m_rig, RIG_VFO_CURR, RIG_LEVEL_METER, &strength);
					if (retval == RIG_OK) {
						std::lock_guard lk(mtx);
						jRigStatus["meter"] = strength.i;
					}

					retval = rig_get_level(m_rig, RIG_VFO_CURR, RIG_LEVEL_VD_METER, &strength);
					if (retval == RIG_OK) {
						std::lock_guard lk(mtx);
						jRigStatus["vd"] = strength.i;
					}

					retval = rig_get_level(m_rig, RIG_VFO_CURR, RIG_LEVEL_RFPOWER_METER, &strength);
					if (retval == RIG_OK) {
						std::lock_guard lk(mtx);
						jRigStatus["rfpower"] = strength.f;
					}

					long short_freq = 0;

					retval = rig_get_rit(m_rig, RIG_VFO_CURR, &short_freq);

					if (retval == RIG_OK) {
						std::lock_guard lk(mtx);
						jRigStatus["rit"] = short_freq;
					}

					retval = rig_get_xit(m_rig, RIG_VFO_CURR, &short_freq);

					if (retval == RIG_OK) {
						std::lock_guard lk(mtx);
						jRigStatus["xit"] = short_freq;
					}

					ptt_t ptt;
					retval = rig_get_ptt(m_rig, RIG_VFO_CURR, &ptt);
					if (retval == RIG_OK) {
						std::lock_guard lk(mtx);
						jRigStatus["ptt"] = (int)ptt;
					}
				}
			}
			std::unique_lock lk(cv_mtx);
			cv.wait_until(lk, start + 50ms, [&]() { return bDone || bWakeUp; });
		}
	});
}

RigControl::~RigControl()
{
	{
		std::unique_lock lk(cv_mtx);
		bDone = true;
	}
	cv.notify_one();
	if (tStatus.joinable()) {
		tStatus.join();
	}
}

bool RigControl::DrawSideBar(bool bOpen)
{
	static bool bPolledSettings = false;
	static bool bSetSettings = false;
	std::lock_guard lk(mutexData.mtx);
	if (ImGui::CollapsingHeader("Rig Control", bOpen ? ImGuiTreeNodeFlags_DefaultOpen : 0)) {
		if (ImGui::BeginCombo("Rig", jsonTypedRef<char *>(jLocalData["current_rig_name"]))) {
			int i = 0;
			for (auto & mfgr : jMyRigs) {
				std::string sMfgr = mfgr.key();
				for (auto & rig : mfgr) {
					std::string sRig = rig.key();
					const bool isSelected = (jLocalData["current_rig_item"]._int() == i);
					if (ImGui::Selectable((sMfgr + " " + sRig).c_str(), isSelected)) {
						jLocalData["current_rig_item"] = i;
						jLocalData["current_rig"] = rig;
						jLocalData["current_rig_name"] = sMfgr + " " + sRig;
						bPolledSettings = false;
					}
					++i;
				}
			}

			ImGui::EndCombo();
		}

		ImGui::Checkbox("Show Rigs Info", jsonTypedRef<bool>(jLocalData["show_rig_info"]));
		if (jLocalData["show_rig_info"].boolean()) {
			ShowJsonWindow("Rigs Info", jRigs, *jsonTypedRef<bool>(jLocalData["show_rig_info"]));
		}

		ImGui::Checkbox("Show Local Data", jsonTypedRef<bool>(jLocalData["show_local_data"]));
		if (jLocalData["show_local_data"].boolean()) {
			ShowJsonWindow("Local Data", jLocalData, *jsonTypedRef<bool>(jLocalData["show_local_data"]));
		}

		ImGui::Checkbox("Show Rig Status", jsonTypedRef<bool>(jLocalData["show_rig_status"]));
		if (jLocalData["show_rig_status"].boolean()) {
			std::lock_guard lk(mtx);
			ShowJsonWindow("Rig Status", jRigStatus, *jsonTypedRef<bool>(jLocalData["show_rig_status"]));
		}

		ImGui::Checkbox("Show Log", jsonTypedRef<bool>(jLocalData["show_Log"]));

		if (ImGui::BeginCombo("Debug Level", jsonTypedRef<char *>(jLocalData["debug_level"]))) {
			for (auto & level : debug_levels) {
				const bool isSelected = (jLocalData["debug_level"] == level.first);
				if (ImGui::Selectable(level.first.c_str(), isSelected)) {
					rig_set_debug(level.second);
					jLocalData["debug_level"] = level.first;
				}
			}
			ImGui::EndCombo();
		}

		if (jLocalData["current_rig"]["rig_model"]._uint()) {
			struct RigConfData
			{
					json::value & jData;
					RIG * rig;
			};

			auto ext_func_callback = [](RIG * rig, const struct confparams * conf, rig_ptr_t pData)
			{
				RigConfData & data = *(RigConfData *)pData;
				json::value & jData = data.jData;
				jData[conf->name]["label"] = conf->label;
				jData[conf->name]["dflt"] = conf->dflt;
				jData[conf->name]["tooltip"] = conf->dflt;
				jData[conf->name]["token"] = conf->token;
				jData[conf->name]["type"] = conf->type;
				return 1;
			};

			auto ext_func_callback2 = [](/*RIG * rig, */ const struct confparams * conf, rig_ptr_t pData)
			{
				RigConfData & data = *(RigConfData *)pData;
				json::value & jData = data.jData;
				char buf[128] = "";
				rig_get_conf(data.rig, conf->token, buf);
				jData[conf->name]["label"] = conf->label;
				jData[conf->name]["dflt"] = conf->dflt;
				jData[conf->name]["tooltip"] = conf->tooltip;
				jData[conf->name]["token"] = conf->token;
				jData[conf->name]["type"] = (int)conf->type;
				jData[conf->name]["value"] = buf;
				switch (conf->type) {
					case RIG_CONF_NUMERIC:
						jData[conf->name]["min"] = conf->u.n.min;
						jData[conf->name]["max"] = conf->u.n.max;
						jData[conf->name]["step"] = conf->u.n.step;
						break;

					case RIG_CONF_COMBO:
						if (!conf->u.c.combostr[0]) {
							break;
						}

						jData[conf->name]["combo"].push_back(conf->u.c.combostr[0]);
						for (int i = 1; i < RIG_COMBO_MAX && conf->u.c.combostr[i]; i++) {
							jData[conf->name]["combo"].push_back(conf->u.c.combostr[i]);
						}

						break;
					default:
						break;
				}
				return 1;
			};
			if (!m_rig) {
				m_rig = rig_init(jLocalData["current_rig"]["rig_model"]._uint());
				bPolledSettings = false;
				if (m_rig) {
					rig_set_debug_callback(rig_logger, nullptr);
					auto it = debug_levels.find(jLocalData["debug_level"].string());
					if (it != debug_levels.end()) {
						rig_set_debug(it->second);
					} else {
						jLocalData["debug_level"] = "RIG_DEBUG_NONE";
					}
				}
			}

			if (!m_rig) {
				std::cerr << "Unknown rig num: " << jLocalData["current_rig"]["rig_model"]._uint() << std::endl;
				std::cerr << "Please check riglist.h" << std::endl;
			} else {
				// ojson::document jRigSettings;
				if (m_rig && !bPolledSettings) {
					bPolledSettings = true;
					RigConfData data {jLocalData["current_rig"]["available_settings"], m_rig};
					RigConfData data1 {jLocalData["current_rig"]["available_settings"], m_rig};
					RigConfData data2 {jLocalData["current_rig"]["available_settings"], m_rig};
					rig_token_foreach(m_rig, ext_func_callback2, &data);
					rig_ext_parm_foreach(m_rig, ext_func_callback, &data1);
					rig_ext_func_foreach(m_rig, ext_func_callback, &data2);
					for (auto & set : jLocalData["current_rig"]["settings"]) {
						set["set"] = false;
					}
				}
				for (auto & setting : jLocalData["current_rig"]["available_settings"]) {
					auto & jCurrentSetting = jLocalData["current_rig"]["settings"][setting.key()];
					jCurrentSetting["token"] = setting["token"];
					switch (setting["type"]._int()) {
						case RIG_CONF_STRING:
						{
							if ((ImGui::InputText(setting["label"].c_str(), jsonTypedRef<char *>(jCurrentSetting.value_or("value", setting["value"])), 256) || jCurrentSetting["set"] == false) && m_rig) {
								rig_set_conf(m_rig, setting["token"]._long(), jCurrentSetting["value"].c_str());
								jCurrentSetting["set"] = true;
							}

							break;
						}

						case RIG_CONF_COMBO:
						{
							if (jCurrentSetting["value"].IsVoid()) {
								jCurrentSetting["value"] = setting["value"];
							} else {
								if (m_rig && jCurrentSetting["set"] == false) {
									rig_set_conf(m_rig, setting["token"]._long(), jCurrentSetting["value"].c_str());
									jCurrentSetting["set"] = true;
								}
							}
							if (ImGui::BeginCombo(setting["label"].c_str(), jsonTypedRef<char *>(jCurrentSetting["value"]))) {
								int i = 0;
								for (auto & item : setting["combo"]) {
									const bool isSelected = (jCurrentSetting["value"] == item);
									if (ImGui::Selectable(item.c_str(), isSelected)) {
										jCurrentSetting["index"] = i;
										jCurrentSetting["value"] = item;
										if (m_rig) {
											rig_set_conf(m_rig, setting["token"]._long(), item.c_str());
											jCurrentSetting["set"] = true;
										}
									}
									++i;
								}
								ImGui::EndCombo();
							}
							break;
						}

						case RIG_CONF_NUMERIC:
						{
							if ((ImGui::InputFloat(setting["label"].c_str(), jsonTypedRef<float>(jCurrentSetting.value_or("value", setting["value"]._float())), setting["step"]._float()) || jCurrentSetting["set"] == false) && m_rig) {
								rig_set_conf(m_rig, setting["token"]._long(), jCurrentSetting["value"].c_str());
								jCurrentSetting["set"] = true;
							}
							break;
						}

						case RIG_CONF_CHECKBUTTON:
						{
							if ((ImGui::Checkbox(setting["label"].c_str(), jsonTypedRef<bool>(jCurrentSetting.value_or("value", setting["value"] == "1"))) || jCurrentSetting["set"] == false) && m_rig) {
								if (jCurrentSetting["value"].boolean()) {
									rig_set_conf(m_rig, setting["token"]._long(), "1");
								} else {
									rig_set_conf(m_rig, setting["token"]._long(), "0");
								}
								jCurrentSetting["set"] = true;
							}
							break;
						}

						case RIG_CONF_BUTTON:
						{
							// ImGui::Button(setting["label"].c_str());
							break;
						}

						case RIG_CONF_BINARY:
						{
							// ImGui::InputText(setting["label"].c_str(), jsonTypedRef<char *>(jCurrentSetting.value_or("value", setting["value"])), 256);
							break;
						}
					}
					if (ImGui::BeginItemTooltip()) {
						ImGui::Text("%s", setting["tooltip"].c_str());
						ImGui::EndTooltip();
					}
				}
				std::string sConnect = "Connected";
				if (m_rig && m_rig->state.comm_state == 0) {
					sConnect = "Connect";
				}
				if (ImGui::Button(sConnect.c_str()) && m_rig) {
					if (m_rig->state.comm_state == 0) {
						rig_open(m_rig);
					} else {
						rig_close(m_rig);
					}
				}
			}
		}
		return true;
	}
	return false;
}

void RigControl::DrawTab()
{
	std::lock_guard lk(mutexData.mtx);
	ImGuiIO & io = ImGui::GetIO();
	if (ImGui::BeginTabItem(sName.c_str())) {
		ImGui::BeginChild("RigTab_child");
		{
			auto showVFO = [](ImGuiIO & io, json::value & vfo, int iFontIndex)
			{
				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
				ImGui::BeginChild(("RigTab_" + vfo.key()).c_str(), {0, 0}, ImGuiChildFlags_AlwaysAutoResize | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Border);
				ImGui::PushFont(io.Fonts->Fonts[iFontIndex]);
				ImGui::Text("%s", PrettyFreq(vfo["freq"]._uint64()).c_str());
				ImGui::PopFont();
				ImGui::SameLine();
				ImGui::Text("%s", ModeString(vfo["mode"]._uint64()).c_str());
				// ImGui::Text("%s", vfo["width"].c_str());
				// ImGui::Text("%s", vfo["split"].c_str());
				// ImGui::Text("%s", vfo["satmode"].c_str());
				ImGui::EndChild();
				ImGui::PopStyleColor(2);
			};
			std::lock_guard lk(mtx);
			ImGui::SeparatorText("VFO");
			showVFO(io, jRigStatus["VFO"], 11);
			// ImGui::SeparatorText("VFO B");
			// showVFO(io, jRigStatus["VFO_B"], 10);
		}

		std::string sConnect = "Connected";
		if (m_rig && m_rig->state.comm_state == 0) {
			sConnect = "Connect";
		} else {
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.5f, 0.0f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.6f, 0.0f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.7f, 0.0f, 1.0f));
		}

		bool bChange = false;

		if ((ImGui::Button(sConnect.c_str()))) {
			bChange = true;
		}

		if (m_rig) {
			if (jLocalData["was_connected"].boolean() && m_rig->state.comm_state == 0) {
				bChange = true;
			}
			if (bChange) {
				if (m_rig->state.comm_state == 0) {
					rig_open(m_rig);
				} else {
					rig_close(m_rig);
				}
				jLocalData["was_connected"] = (m_rig->state.comm_state != 0);
			}
		}

		if (sConnect == "Connected") {
			ImGui::PopStyleColor(3);
		}

		{
			ImGui::SameLine();
			std::lock_guard lk(mtx);
			std::string sPTT;
			bool bColour = false;
			switch (jRigStatus["ptt"]._int()) {
				case RIG_PTT_OFF:
				{
					sPTT = "PTT";
					break;
				}
				case RIG_PTT_ON:
				{
					sPTT = "TX";
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.0f, 0.0f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 0.0f, 0.0f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.0f, 0.0f, 1.0f));
					bColour = true;
					break;
				}
				case RIG_PTT_ON_MIC:
				{
					sPTT = "TX Mic";
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.0f, 0.0f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 0.0f, 0.0f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.0f, 0.0f, 1.0f));
					bColour = true;
					break;
				}
				case RIG_PTT_ON_DATA:
				{
					sPTT = "TX Data";
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.0f, 0.0f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 0.0f, 0.0f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.0f, 0.0f, 1.0f));
					bColour = true;
					break;
				}
			}
			static bool bIPPTd = false;
			ImGui::Button(sPTT.c_str());
			// if (ImGui::IsItemClicked(0)) {
			// 	if (m_rig && jRigStatus["ptt"] == 0) {
			// 		rig_set_ptt(m_rig, RIG_VFO_CURR, RIG_PTT_ON);
			// 		bIPPTd = true;
			// 	}
			// }
			// if (!ImGui::IsItemActive()) {
			// 	if (m_rig && jRigStatus["ptt"] != 0 && bIPPTd) {
			// 		rig_set_ptt(m_rig, RIG_VFO_CURR, RIG_PTT_OFF);
			// 	}
			// 	bIPPTd = false;
			// }
			if (bColour) {
				ImGui::PopStyleColor(3);
			}
		}

		ImGui::EndChild();
		ImGui::EndTabItem();
	}

	if (jLocalData["show_Log"].boolean()) {
		std::lock_guard lk(mtx);
		ImGui::Begin("RigCtl Log", jsonTypedRef<bool>(jLocalData["show_Log"]));

		static ImGuiTableFlags flags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody;
		ImGui::BeginTable("RigCtlLog", 4, flags);

		ImGui::TableSetupColumn("Level", ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_WidthStretch, 0.1);
		ImGui::TableSetupColumn("Local Time", ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_WidthStretch, 0.2);
		ImGui::TableSetupColumn("GMT", ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_WidthStretch, 0.2);
		ImGui::TableSetupColumn("Message", ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableHeadersRow();

		for (auto & item : jLog) {
			bool bSetColor = false;
			std::string sLevel = "Unknown";
			switch (item["level"]._int()) {
				case RIG_DEBUG_NONE:
				{
					sLevel = "None";
					break;
				}

				case RIG_DEBUG_BUG:
				{
					sLevel = "Bug";
					bSetColor = true;
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.0f, 1.0f, 1.0f));
					break;
				}

				case RIG_DEBUG_ERR:
				{
					sLevel = "Error";
					bSetColor = true;
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
					break;
				}

				case RIG_DEBUG_WARN:
				{
					sLevel = "Warning";
					bSetColor = true;
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.5f, 0.0f, 1.0f));
					break;
				}

				case RIG_DEBUG_VERBOSE:
				{
					sLevel = "Verbose";
					break;
				}

				case RIG_DEBUG_TRACE:
				{
					sLevel = "Trace";
					break;
				}

				case RIG_DEBUG_CACHE:
				{
					sLevel = "Cache";
					break;
				}
			}

			ImGui::TableNextRow();
			ImGui::TableNextColumn();

			ImGui::Text("%s", sLevel.c_str());
			ImGui::TableNextColumn();
			ImGui::Text("%s", item["local_time"].c_str());
			ImGui::TableNextColumn();
			ImGui::Text("%s", item["universal_time"].c_str());
			ImGui::TableNextColumn();
			ImGui::Text("%s", item["message"].c_str());
			if (bSetColor) {
				ImGui::PopStyleColor();
			}
		}
		ImGui::EndTable();
		ImGui::End();
	}
}

HamLab::PluginBase * RigControl::create(HamLab::DataShareClass & data_share_, const std::string & name) { return new RigControl(data_share_, name); }

BOOST_DLL_ALIAS(RigControl::create, // <-- this function is exported with...
				CreatePlugin		// <-- ...this alias name
)

const char * MyName() { return "Rig Control"; }

BOOST_DLL_ALIAS(MyName, // <-- this function is exported with...
				Name	// <-- ...this alias name
)

const HamLab::APIVersion * MyVersion() { return &HamLab::CURRENT_API_VERSION; }

BOOST_DLL_ALIAS(MyVersion, // <-- this function is exported with...
				Version	   // <-- ...this alias name
)
