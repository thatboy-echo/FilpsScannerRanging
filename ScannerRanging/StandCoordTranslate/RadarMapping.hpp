#pragma once

#include <string>
#include <vector>
#include "Config.h"
#include "../LidarDataPackage/EquipmentComm/EquipmentComm.h"
#include "../LidarDataPackage/LIM/lim.h"
#include "../../../Logger/Logger/Logger.hpp"

namespace Radar
{
	enum { CID_DATA = 0XAA, CID_DISTANCE };
	bool dataDeviceOnline{ false };
	bool distanceDeviceOnline{ false };
	std::string dataDeviceIP;
	std::string distanceDeviceIP;
	clock_t dataSendTimeSpan;
	clock_t deviceConnectTimeOut;
	int mapXAccuracy;
	constexpr auto PropertiesFilePath{ "Application.properties" };
	thatboy::logger::FileLogger logger{ "lidar.log" };

	std::string serverHost;
	std::string serverPath;
	int serverPort;
	// 数据
	std::vector<int> lidarPolar;
	int lidarBegin;
	int lidarEnd;
	int lidarX;
	std::mutex lidarPolarLock;
	std::mutex lidarXLock;


	/// <summary>
	/// 数据回调函数
	/// </summary>
	/// <param name="_cid">连接编号</param>
	/// <param name="_lim_code">LIM Code</param>
	/// <param name="_lim">LIM</param>
	/// <param name="_lim_len">LIM包长度</param>
	/// <param name="_paddr">用户数据</param>
	void CALLBACK onDataCallBack(INT _cid, UINT _lim_code, LPVOID _lim, INT _lim_len, INT _paddr)
	{
		auto& lim = *static_cast<LIM_HEAD*>(_lim);

		if (LIM_CheckSum(&lim) != lim.CheckSum)
			return;
		switch (lim.nCode)
		{
		case LIM_CODE_LMD:
			switch (lim.nCID)
			{
			case CID_DATA:
			{
				// 保存极坐标数组
				LMD_INFO& lmd_info = *LMD_Info(&lim);
				LMD_D_Type* lmd = LMD_D(&lim);
				std::lock_guard<std::mutex> lockGuard(lidarPolarLock);
				lidarBegin = lmd_info.nBAngle;
				lidarEnd = lmd_info.nEAngle;
				if (lidarPolar.size() != lmd_info.nMDataNum)
					lidarPolar.resize(lmd_info.nMDataNum);
				for (size_t i = 0; i < lmd_info.nMDataNum; i++)
					lidarPolar[i] = lmd[i];
			}
			break;
			case CID_DISTANCE:
			{
				// 保存90°距离
				LMD_INFO& lmd_info = *LMD_Info(&lim);
				LMD_D_Type* lmd = LMD_D(&lim);
				std::lock_guard<std::mutex> lockGuard(lidarXLock);
				if (lmd_info.nBAngle <= 9000 || lmd_info.nEAngle >= 9000)
					lidarX = lmd[(90 - lmd_info.nBAngle) * 2] / 10 * 10;
				else
					lidarX = -10000;
			}
			break;
			default:
				break;
			}
			break;
		default:
			break;
		}
	}

	/// <summary>
	/// 状态回调函数
	/// </summary>
	/// <param name="_cid">连接编号</param>
	/// <param name="_state_code">连接状态</param>
	/// <param name="_ip">IP</param>
	/// <param name="_port">端口</param>
	/// <param name="_paddr">用户数据</param>
	void CALLBACK onStateCallBack(INT _cid, UINT _state_code, LPCSTR _ip, INT _port, INT _paddr)
	{
		switch (_state_code)
		{
		case EQCOMM_STATE_OK:
		{
			LIM_HEAD* lim = NULL;
			LIM_Pack(lim, _cid, LIM_CODE_GET_LDBCONFIG, NULL);
			SendLIM(_cid, lim, lim->nLIMLen);
			LIM_Release(lim);

			if (_cid == CID_DATA)
				dataDeviceOnline = true;
			else if (_cid == CID_DISTANCE)
				distanceDeviceOnline = true;
		}
		break;
		case EQCOMM_STATE_ERR:
		case EQCOMM_STATE_LOST:
		{
			if (_cid == CID_DATA)
				dataDeviceOnline = false;
			else if (_cid == CID_DISTANCE)
				distanceDeviceOnline = false;
		}
		break;
		default:
			break;
		}
	}
	
	/// <summary>
	/// 等待设备全部上线
	/// </summary>
	/// <param name="timeOutMs">超时时长</param>
	/// <returns>超时未上线则返回false</returns>
	bool waitDevicesOnline(int timeOutMs)
	{
		using namespace std::chrono;
		auto in{ steady_clock::now() };
		auto duration = std::chrono::milliseconds(timeOutMs);

		while (!dataDeviceOnline && !distanceDeviceOnline)
		{
			std::this_thread::sleep_for(10ms);
			if (steady_clock::now() - in > duration)
				return false;
		}
		return true;
	}

	/// <summary>
	/// 开启设备
	/// </summary>
	void startDevices()
	{
		LIM_HEAD* lim = nullptr;
		LIM_Pack(lim, CID_DATA, LIM_CODE_START_LMD);
		SendLIM(CID_DATA, lim, lim->nLIMLen);
		LIM_Release(lim);
		lim = nullptr;
		LIM_Pack(lim, CID_DISTANCE, LIM_CODE_START_LMD);
		SendLIM(CID_DISTANCE, lim, lim->nLIMLen);
		LIM_Release(lim);
	}

	bool initDevices()
	{
		if (!EquipmentCommInit(NULL, Radar::onDataCallBack, Radar::onStateCallBack))
			return false;
		OpenEquipmentComm(Radar::CID_DATA, Radar::dataDeviceIP.c_str(), 2112);
		OpenEquipmentComm(Radar::CID_DISTANCE, Radar::distanceDeviceIP.c_str(), 2112);
		return true;
	}

	/// <summary>
	/// 指示设备在线
	/// </summary>
	inline bool devicesOnline()
	{
		return dataDeviceOnline && distanceDeviceOnline;
	}

	/// <summary>
	/// 加载配置信息
	/// </summary>
	void loadProperites()
	{
		std::string serverUrl;
		Config cfg(PropertiesFilePath);
		cfg.ReadInto(dataDeviceIP, "Device.Data.IP");
		cfg.ReadInto(distanceDeviceIP, "Device.Distance.IP");
		cfg.ReadInto(dataSendTimeSpan, "Device.SendTimeSpan");
		cfg.ReadInto(deviceConnectTimeOut, "Device.ConnectTimeOut");
		cfg.ReadInto(mapXAccuracy, "Device.Distance.Accuracy");
		cfg.ReadInto(serverUrl, "Server.Url");

		auto postIpMark = serverUrl.find_first_of(':');
		auto postPortMark = serverUrl.find_first_of('/');
		serverHost = serverUrl.substr(0, postIpMark);
		serverPath = serverUrl.substr(postPortMark);
		serverPort = stoi(serverUrl.substr(postIpMark + 1, postPortMark));
	}
};

