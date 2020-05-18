#pragma once

#include <string>
#include <mutex>
#include <map>
#include <atomic>
#include <vector>
#include <algorithm>


namespace lidar
{
	// 极坐标
	struct PolarCoord
	{
		double length;
		double angle;
	};


	// 直角坐标
	struct RectaCoord
	{
		double x;
		double y;
	};

	// 发送坐标
	struct RelativeCoord
	{
		int offset;
		int h;
	};

	// 设备
	struct LidarDevice
	{
	protected:
		friend struct LidarCoordLockGuard;
		std::mutex coordClock;

		// 数据
		double angleBeg{ 0 };
		double angleEnd{ 0 };

		// 状态
		bool isConnected{ false };
		bool hasDisconnected{ false };

	public:
		std::vector<PolarCoord> polar;			// 极坐标数组
		std::vector<RectaCoord> recta;			// 直角坐标数组
		std::vector<RelativeCoord> negHeight;	// 高度坐标数组

		std::string ip;					// 设备ip
		int port;						// 设备端口
		int id;							// 设备id
	};

	// 设备自动锁
	struct LidarCoordLockGuard
		: public std::lock_guard<std::mutex>
	{
		LidarCoordLockGuard(LidarDevice&device)
			: std::lock_guard<std::mutex>(device.coordClock) {}
	};


	// 设备
	class VirtualLidarBase
	{
		using string = std::string;
		using map = std::map<int, LidarDevice>;

	protected:
		/// 接口部分

		// 连接设备
		virtual int connectDevice(string ip, int port) = 0;
		virtual void disconnectDevice(int) = 0;


	protected:
		// 数据部分
		static constexpr double Pi = 3.141592654;
	};
}