#pragma once

#ifndef _LIM_DEVICE_H_
#define _LIM_DEVICE_H_

#include <string>
#include <mutex>
#include <map>
#include <atomic>
#include <vector>
#include <algorithm>
#include <cmath>

#include "../LidarDataPackage/EquipmentComm/EquipmentComm.h"
#include "../LidarDataPackage/LIM/lim.h"

struct LimDevice
{
#ifdef UNICODE
	using tstring = std::wstring;
#	ifndef to_tstring
#	define to_tstring std::to_wstring
#	endif
#else
	using tstring = std::string;
#	ifndef to_tstring
#	define to_tstring std::to_string
#	endif
#endif // UNICODE

public:
	constexpr static double pi = 3.141592654;
	struct PolarCoord
	{
		double length;
		double angle;
	};
	struct RectaCoord
	{
		double x;
		double y;
	};
	struct StdCoord
	{
		int x;
		int y;
	};

	/// Êý¾Ý
	bool isTryConnected = false;
	std::atomic_bool isConnected = false;
	std::string deviceIP;
	double angleBeg = -45;
	double angleEnd = 225;
	double lineContinusLen = 100;
	size_t lineLeastNumber = 3;

	std::vector<PolarCoord> polarCoord;
	std::vector<RectaCoord> rectaCoord;
	std::vector<StdCoord> negHeightCoord;

	static constexpr double MaxLength = 7000.;
	inline static std::mutex staticDataLock;
	inline static int OnlineDeviceNumber = 0;
	inline static int SerialCID = 0x80;
	inline static bool hasDeviceTryConnected = false;
	inline static std::map<int, LimDevice> DeviceList;

	void LockCoord() { coordLock.lock(); }
	void UnlockCoord() { coordLock.unlock(); }
	void SetLineContinusLen(double len) { lineContinusLen = len; }
	void SetLineLeastNumber(size_t num) { lineLeastNumber = num; }

	~LimDevice();

	static void InitEquipment();
	static int OpenEquipment(LPCSTR ip, int port = LIM_USER_PORT);
	static void WaitFirstDeviceTryConnected();
	static void StartLMDData();

protected:
	std::mutex coordLock;
	int cid = 0XFFFFFFF;
	void processCoord();
	void onLMDRecive(LIM_HEAD& lim);
	void onDeviceQuit();
	static void CALLBACK onDataCallBack(int _cid, unsigned int _lim_code, void* _lim, int _lim_len, int _paddr);
	static void CALLBACK onStateCallBack(int _cid, unsigned int _state_code, const char* _ip, int _port, int _paddr);
};

#endif // !_LIM_DEVICE_H_
