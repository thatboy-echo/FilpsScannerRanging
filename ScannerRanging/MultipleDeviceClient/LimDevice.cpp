#include "LimDevice.h"

void LimDevice::processCoord()
{
	rectaCoord.clear();
	negHeightCoord.clear();

	// 要求设备扫描九十度
	if (angleBeg > 90 || angleEnd < 90)
		return;

	// 转换直角坐标、排除异常点，排除过高点，操作后，坐标点X有序
	double lastX = 0;
	// x -n > 0
	for (int i = (90 - angleBeg) * 2; i < polarCoord.size(); i++)
	{
		auto& [length, angle] = polarCoord[i];
		if (length > MaxLength)
			continue;
		RectaCoord thisCoord{ length * cos(angle / 180 * pi), length * sin(angle / 180 * pi) };
		if (thisCoord.x <= lastX)
		{
			lastX = thisCoord.x;
			rectaCoord.push_back(thisCoord);
		}
	}
	std::reverse(rectaCoord.begin(), rectaCoord.end());
	lastX = 0;
	// x 0 > +n
	for (int i = (90 - angleBeg) * 2 - 1; i >= 0; i--)
	{
		auto& [length, angle] = polarCoord[i];
		if (length > MaxLength)
			continue;
		RectaCoord thisCoord{ length * cos(angle / 180 * pi), length * sin(angle / 180 * pi) };
		if (thisCoord.x >= lastX)
		{
			lastX = thisCoord.x;
			rectaCoord.push_back(thisCoord);
		}
	}

	// 插值
	int beginX = std::lround(rectaCoord.front().x);
	int endX = std::lround(rectaCoord.back().x);

	auto searchIterator = rectaCoord.begin();
	auto getNextY = [&searchIterator, this](int x)->int
	{
		// 查找x左右两侧数值，searchIterator->greater
		while (searchIterator != rectaCoord.end())
			if (x < searchIterator->x)
				break;
			else
				++searchIterator;
		if (searchIterator == rectaCoord.end())
			return std::lround(rectaCoord.back().y);
		else if (x > searchIterator->x || searchIterator == rectaCoord.begin())
			return std::lround(searchIterator->y);
		else
			return std::lround(((searchIterator->x - x) * (searchIterator - 1)->y + (x - (searchIterator - 1)->x) * searchIterator->y) / (searchIterator->x - (searchIterator - 1)->x));
	};
	for (int x = beginX; x <= endX; ++x)
		negHeightCoord.push_back(StdCoord{ x, getNextY(x) });
}

void LimDevice::onLMDRecive(LIM_HEAD& lim)
{
	if (lim.nCode != LIM_CODE_LMD)
		return;
	LMD_INFO& lmd_info = *LMD_Info(&lim);
	LMD_D_Type* lmd = LMD_D(&lim);

	angleBeg = lmd_info.nBAngle / 1000.;
	angleEnd = lmd_info.nEAngle / 1000.;

	std::lock_guard<std::mutex> lockGuard(coordLock);
	if (polarCoord.size() != lmd_info.nMDataNum)
		polarCoord.resize(lmd_info.nMDataNum);
	for (int i = 0; i < lmd_info.nMDataNum; i++)
	{
		polarCoord[i].angle = static_cast<double>((lmd_info.nBAngle + i * (float)(lmd_info.nEAngle - lmd_info.nBAngle) / (lmd_info.nMDataNum - 1)) / 1000.0);
		polarCoord[i].length = static_cast<double>(lmd[i]);
	}
	processCoord();
}

void LimDevice::onDeviceQuit()
{
	if (isTryConnected)
	{
		if (isConnected)
		{
			LIM_HEAD* lim = NULL;
			LIM_Pack(lim, cid, LIM_CODE_IOSET_RELEASE, NULL);
			SendLIM(cid, lim, lim->nLIMLen);
			LIM_Release(lim);

			LIM_Pack(lim, cid, LIM_CODE_STOP_LMD, NULL);
			SendLIM(cid, lim, lim->nLIMLen);
			LIM_Release(lim);

			isConnected = false;
		}
		CloseEquipmentComm(cid);
		isTryConnected = false;
	}
}

LimDevice::~LimDevice()
{
	onDeviceQuit();
}

void LimDevice::InitEquipment()
{
	EquipmentCommInit(NULL, onDataCallBack, onStateCallBack); // 初始化设备库
}

int LimDevice::OpenEquipment(LPCSTR ip, int port)
{
	OpenEquipmentComm(SerialCID, ip, port);
	staticDataLock.lock();
	++SerialCID;
	staticDataLock.unlock();
	return SerialCID - 1;
}

void LimDevice::WaitFirstDeviceTryConnected()
{
	using namespace std::chrono;
	while (!hasDeviceTryConnected)
		std::this_thread::sleep_for(10ms);
}

void LimDevice::StartLMDData()
{
	for (auto& device : DeviceList)
	{
		if (device.second.isConnected)
		{
			LIM_HEAD* lim = NULL;
			LIM_Pack(lim, device.first, LIM_CODE_START_LMD); //Start LMD LIM transferring.
			SendLIM(device.first, lim, lim->nLIMLen);
			LIM_Release(lim);
		}
	}
}

void CALLBACK LimDevice::onDataCallBack(int _cid, unsigned int _lim_code, void* _lim, int _lim_len, int _paddr)
{
	LIM_HEAD& lim = *(LIM_HEAD*)_lim;

	if (LIM_CheckSum(&lim) != lim.CheckSum)
		return;
	switch (lim.nCode)
	{
	case LIM_CODE_LMD:
		DeviceList[_cid].onLMDRecive(lim);
		break;
	case LIM_CODE_LMD_RSSI:
		break;
	default:
		break;
	}
}

void CALLBACK LimDevice::onStateCallBack(int _cid, unsigned int _state_code, const char* _ip, int _port, int _paddr)
{
	switch (_state_code)
	{
	case EQCOMM_STATE_OK:
	{
		LIM_HEAD* lim = NULL;
		LIM_Pack(lim, _cid, LIM_CODE_GET_LDBCONFIG, NULL);
		SendLIM(_cid, lim, lim->nLIMLen);
		LIM_Release(lim);

		staticDataLock.lock();
		++OnlineDeviceNumber;
		staticDataLock.unlock();

		DeviceList[_cid].polarCoord.reserve(541);
		DeviceList[_cid].rectaCoord.reserve(541);
		DeviceList[_cid].deviceIP = _ip;
		DeviceList[_cid].isConnected = true;
		DeviceList[_cid].cid = _cid;
	}
	break;
	case EQCOMM_STATE_ERR:
	case EQCOMM_STATE_LOST:
		staticDataLock.lock();
		if (DeviceList[_cid].isConnected)
			DeviceList[_cid].isConnected = false;
		--OnlineDeviceNumber;
		staticDataLock.unlock();
		break;
	default:
		break;
	}
	DeviceList[_cid].isTryConnected = true;
	hasDeviceTryConnected = true;
}