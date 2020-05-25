#include <httplib.h>
#include <json.hpp>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <conio.h>
#include <cmath>
#include "../MultipleDeviceClient/LimDevice.h"


using json = nlohmann::json;
using Client = httplib::Client;
using Request = httplib::Request;
using Response = httplib::Response;
using namespace std::chrono;

constexpr int limServerPort = 5056;

void getDistance(const Request& req, Response& res);
void getHeight(const Request& req, Response& res);
void getCoord(const Request& req, Response& res);
void getInfo(const Request& req, Response& res);
void getConnect(const Request& req, Response& res);
void getDisconnect(const Request& req, Response& res);

int main()
{
	LimDevice::InitEquipment();
	LimDevice::OpenEquipment("192.168.31.210");
	LimDevice::OpenEquipment("192.168.31.202");
	LimDevice::WaitFirstDeviceTryConnected();

	while (LimDevice::OnlineDeviceNumber < 2)
		std::this_thread::sleep_for(10ms);
	LimDevice::StartLMDData();
	Client client("192.168.152.128", 9090);

	auto& deviceMap = LimDevice::DeviceList[128];
	auto& deviceDst = LimDevice::DeviceList[129];

	while (true)
	{
		//constexpr int Y_STEP = 50;
		//constexpr int X_STEP = 50;
		//constexpr int X_NUM = 60;
		//constexpr int Y_NUM = 180;
		constexpr int DEBUG_SCALE = 10;
		constexpr int Y_STEP = 5;
		constexpr int X_STEP = 5;
		constexpr int H_MAX = 400;

		int distance = 0;


		json body;
		if (deviceMap.negHeightCoord.empty() || deviceDst.negHeightCoord.empty())
		{
			std::this_thread::sleep_for(50ms);
			continue;
		}

		deviceDst.LockCoord();
		auto Dst90 = std::find_if(deviceDst.negHeightCoord.begin(), deviceDst.negHeightCoord.end(), [](const LimDevice::StdCoord& coord) {return coord.x == 0; });
		if (Dst90 == deviceDst.negHeightCoord.end())
		{
			std::this_thread::sleep_for(50ms);
			continue;
		}
		else
			distance = Dst90->y;
		deviceDst.UnlockCoord();


		body["cid"] = LimDevice::DeviceList.begin()->first;
		// todo ·Å´ó¹ý
		body["x"] = (distance + X_STEP / 2) / X_STEP * X_STEP * DEBUG_SCALE;
		body["intime"] = std::chrono::system_clock::now().time_since_epoch().count();

		deviceMap.LockCoord();
		int begin = ceil(deviceMap.negHeightCoord.front().x / (double)Y_STEP) * Y_STEP;
		int end = floor(deviceMap.negHeightCoord.back().x / (double)Y_STEP) * Y_STEP;

		for (int yPtr = begin; yPtr <= end; yPtr += Y_STEP)
			body["h"].push_back(deviceMap.negHeightCoord[yPtr - deviceMap.negHeightCoord.front().x].y * DEBUG_SCALE);	// TODO
		deviceMap.UnlockCoord();
		body["begin"] = begin;
		body["end"] = end;

		std::cout << body.dump() << std::endl;
		client.Post("/map/col", body.dump(), "application/json");

		std::this_thread::sleep_for(500ms);
	}
	return 0;
}