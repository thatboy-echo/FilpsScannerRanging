#include <httplib.h>
#include <json.hpp>
#include <sstream>
#include <iomanip>
#include <conio.h>
#include <cmath>
#include "../MultipleDeviceClient/LimDevice.h"


using json = nlohmann::json;
using Client = httplib::Client;
using Request = httplib::Request;
using Response = httplib::Response;

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
	LimDevice::OpenEquipment("192.168.1.210");
	LimDevice::WaitFirstDeviceTryConnected();
	LimDevice::StartLMDData();
	Client client("192.168.152.128", 8080);

	int x = 0;
	while (true)
	{
		constexpr int Y_STEP = 50;
		constexpr int X_STEP = 50;
		constexpr int X_NUM = 60;
		constexpr int Y_NUM = 180;

		json body;
		auto& device = LimDevice::DeviceList.begin()->second;
		if (device.negHeightCoord.empty())
			continue;
		device.LockCoord();
		body["cid"] = LimDevice::DeviceList.begin()->first;
		body["x"] = x * X_STEP / 0.5;
		body["intime"] = std::chrono::system_clock::now().time_since_epoch().count();

		int begin = ceil(device.negHeightCoord.front().x / (double)Y_STEP) * Y_STEP;
		int end = floor(device.negHeightCoord.back().x / (double)Y_STEP) * Y_STEP;


		body["begin"] = begin;
		body["end"] = end;

		for (int x = begin; x <= end; x+= Y_STEP)
			body["h"].push_back(device.negHeightCoord[x - device.negHeightCoord.front().x].y);
		
		//for (auto& [x, y] : device.negHeightCoord)
		//	body["h"].push_back(y);
		device.UnlockCoord();
		client.Post("/map/col", body.dump(), "application/json");
		x = (x + 1) % 30;

		using namespace std::chrono;
		std::this_thread::sleep_for(1000ms);
	}
	return 0;
}