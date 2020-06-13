#pragma once

#include <string>
#include "Config.h"



class RadarMapping
{
	using string = std::string;

protected:
	
#pragma region 常量
	/// <summary>
	/// 配置文件
	/// </summary>
	constexpr static auto ConfigFilePath = "Application.properties";
	/// <summary>
	/// 设备起始下标
	/// </summary>
	constexpr static double DeviceAngleRangeMin = -45;
	/// <summary>
	/// 角度分辨率
	/// </summary>
	constexpr static double DeviceAngleScale = 2;
	/// <summary>
	/// 设备原始数据长度
	/// </summary>
	constexpr static size_t DeviceRawDataLength = 541;

	/// <summary>
	/// 标准起始扫描角度
	/// </summary>
	constexpr static double StdScanAngleBegin = 0;
	/// <summary>
	/// 标准起始扫描角度
	/// </summary>
	constexpr static double StdScanAngleEnd = 180;

	constexpr static int ScaleX = 5;
	constexpr static int RegionXSize = 180;
#pragma endregion

#pragma region 工具函数

public:
	/// <summary>
	/// 角度转下标
	/// </summary>
	/// <param name="angle">角度</param>
	/// <returns>下标</returns>
	//static constexpr int angle2Index(double angle)
	//{
	//	return (angle - DeviceAngleRangeMin) * DeviceAngleScale;
	//}
	static constexpr int angle2Index()
	{
		return 0;
		std::stoi()
	}

	constexpr int  a = angle2Index();








	/// <summary>
	/// 下标转角度
	/// </summary>
	/// <param name="index">下标</param>
	/// <returns>角度</returns>
	static constexpr double index2Angle(int index)
	{
		return index / DeviceAngleScale + DeviceAngleRangeMin;
	}

#pragma endregion

#pragma region 数据
	/// <summary>
	/// 数据设备
	/// </summary>
	string dataDeviceIp;

	/// <summary>
	/// 距离测量设备
	/// </summary>
	string distanceDeviceIp;
	
	/// <summary>
	/// 原始极坐标数据
	/// </summary>
	int rawData[DeviceRawDataLength];

	/// <summary>
	/// 扫描起始坐标
	/// </summary>
	double scanAngleBegin;
	/// <summary>
	/// 扫描结束坐标
	/// </summary>
	double scanAngleEnd;

	int rawRegionData[RegionXSize][angle2Index(/*StdScanAngleEnd*/) - angle2Index(StdScanAngleBegin) + 1];

#pragma endregion

};

