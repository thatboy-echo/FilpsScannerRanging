// TryLIM.cpp
//

#include "EquipmentComm/EquipmentComm.h"
#include "LIM/lim.h"

#include <atomic>

#include <easyx.h>
#include <algorithm>
#include <mutex>
#include <chrono>
#include <map>
#include <conio.h>



namespace lim
{
	std::atomic_bool isInited = false;
	std::atomic_bool isConnected = false;

	enum :int {
		USER_TASK_CID = 1
	};
	constexpr double pi = 3.141592654;
	char deviceIP[] = "192.168.1.210";

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

	std::mutex coordLock;
	size_t coordSize = 541;
	PolarCoord polarCoord[541];
	RectaCoord rectaCoord[541];


	void calcRectaCoord()
	{
		coordLock.lock();
		for (int i = 0; i< coordSize;++i)
		{
			if (polarCoord[i].length > 900)
				polarCoord[i].length = i > 0 ? polarCoord[i - 1].length : 0;
			rectaCoord[i].x = polarCoord[i].length * cos(polarCoord[i].angle / 180 * pi);
			rectaCoord[i].y = polarCoord[i].length * sin(polarCoord[i].angle / 180 * pi);
		}
		coordLock.unlock();
	}

	namespace callback
	{
		void CALLBACK onLMDRecive(LIM_HEAD& lim)
		{
			if (lim.nCode != LIM_CODE_LMD)
				return;
			LMD_INFO& lmd_info = *LMD_Info(&lim);
			LMD_D_Type* lmd = LMD_D(&lim);

			coordLock.lock();
			coordSize = lmd_info.nMDataNum;
			for (int i = 0; i < lmd_info.nMDataNum; i++)
			{
				polarCoord[i].angle = (lmd_info.nBAngle + i * (float)(lmd_info.nEAngle - lmd_info.nBAngle) / (lmd_info.nMDataNum - 1)) / 1000.0;
				polarCoord[i].length = lmd[i];
			}
			coordLock.unlock();
			calcRectaCoord();
		}


		void CALLBACK onDataCallBack(int _cid, unsigned int _lim_code, void* _lim, int _lim_len, int _paddr)
		{
			LIM_HEAD& lim = *(LIM_HEAD*)_lim;

			if (LIM_CheckSum(&lim) != lim.CheckSum)
			{
				printf("\tLIM checksum error!\n");
				return;
			}

			switch (lim.nCode)
			{
			case LIM_CODE_LMD:
				onLMDRecive(lim);
				break;
			case LIM_CODE_LMD_RSSI:
				break;
			default:
				break;
			}
		}


		void CALLBACK onStateCallBack(int _cid, unsigned int _state_code, char* _ip, int _port, int _paddr)
		{
			if (_state_code == EQCOMM_STATE_OK)
			{
				LIM_HEAD* lim = NULL;
				LIM_Pack(lim, USER_TASK_CID, LIM_CODE_GET_LDBCONFIG, NULL);
				SendLIM(USER_TASK_CID, lim, lim->nLIMLen);
				LIM_Release(lim);

				isConnected = true;
			}
			else if ((_state_code == EQCOMM_STATE_ERR) || (_state_code == EQCOMM_STATE_LOST))
			{
				isConnected = false;

				printf("连接失败.code---------------\n\n\n--------:%d\n", _state_code);
			}
			isInited = true;
		}
	}
}



int main()
{

	EquipmentCommInit(NULL, lim::callback::onDataCallBack, lim::callback::onStateCallBack); // 初始化设备库

	OpenEquipmentComm(lim::USER_TASK_CID, lim::deviceIP, LIM_USER_PORT);	// 连接设备

	// 等待设备启动
	using namespace std::chrono;
	while (!lim::isInited)
		std::this_thread::sleep_for(10ms);

	if (lim::isConnected)
	{
		LIM_HEAD* lim = NULL;

		LIM_Pack(lim, lim::USER_TASK_CID, LIM_CODE_START_LMD); //Start LMD LIM transferring.
		SendLIM(lim::USER_TASK_CID, lim, lim->nLIMLen);
		LIM_Release(lim);
		Sleep(100);
	}

	initgraph(900, 900);
	BeginBatchDraw();
	setbkcolor(WHITE);
	setlinestyle(PS_SOLID, 2);
	settextcolor(BLACK);
	setbkmode(TRANSPARENT);

	bool bIfLine = true;
	bool bIfPoint = true;
	bool bIfUI = true;
	MOUSEMSG Msg = { NULL };
	POINT mousePos = { NULL };
	POINT originPos = { NULL };


	originPos.x = getwidth() / 2;
	originPos.y = getheight() / 2;
	float scale = 1;
	setaspectratio(scale, scale);
	setorigin(originPos.x, originPos.y);


	FlushMouseMsgBuffer();

	while (lim::isConnected)
	{

		while (MouseHit())
		{
			Msg = GetMouseMsg();
			if (Msg.wheel != 0)
			{
				while (Msg.wheel > 0)
				{
					scale *= 1.1;
					Msg.wheel -= 120;
				}
				while (Msg.wheel < 0)
				{
					scale *= 0.9;
					Msg.wheel += 120;
				}
				setaspectratio(scale, scale);
				setorigin(originPos.x, originPos.y);
			}
			if (Msg.uMsg == WM_LBUTTONDOWN)
			{
				mousePos.x = Msg.x;
				mousePos.y = Msg.y;
			}
			if (Msg.mkLButton)
			{
				originPos.x += Msg.x - mousePos.x;
				originPos.y += Msg.y - mousePos.y;
				mousePos.x = Msg.x;
				mousePos.y = Msg.y;

				setorigin(originPos.x, originPos.y);
			}

			if (Msg.mkMButton)
			{
				scale = 1;
				originPos.x = getwidth() / 2;
				originPos.y = getheight() / 2;
				setaspectratio(scale, scale);
				setorigin(originPos.x, originPos.y);
			}
			
		}
		if (_kbhit())
		{
			switch (_getch())
			{
			case 'u':
			case 'U':
				bIfUI = !bIfUI;
				break;
			case 'l':
			case 'L':
				bIfLine = !bIfLine;
				break;
			case 'p':
			case 'P':
				bIfPoint = !bIfPoint;
				break;
			case 'q':
			case 'Q':
				goto closeJmp;
			default:
				break;
			}
		}

		cleardevice();

		if (bIfUI)
		{
			setlinecolor(0XD5C9C9);
			setfillcolor(0XD5C9C9);
			solidpie(-10000, -10000, 10000, 10000, -45 * lim::pi / 180, 225 * lim::pi / 180);


			setlinecolor(0XB4A8A8);
			for (int i = 0; i < 100; i++)
			{
				for (int j = 0; j < 100; j++)
				{
					line(-i * 100, -j * 100, i * 100, -j * 100);
					line(-i * 100, j * 100, i * 100, j * 100);
					line(-i * 100, -j * 100, -i * 100, j * 100);
					line(i * 100, -j * 100, i * 100, j * 100);
				}
			}

			setlinecolor(0XCC7A00);
#define TEXT_ARC(_len) \
			arc(-_len##00,-_len##00,_len##00,_len##00, -45 * lim::pi / 180, 225 * lim::pi / 180); \
			outtextxy(_len##00/1.414-10, _len##00/1.414, L## #_len##"m");

			TEXT_ARC(2);
			TEXT_ARC(5);
			TEXT_ARC(10);
			TEXT_ARC(15);
			TEXT_ARC(20);
			TEXT_ARC(30);

#undef TEXT_ARC

			setfillcolor(RGB(0XF0, 0, 0));
			solidcircle(0, 0, 5);
		}
		if (bIfLine)
		{
			setlinecolor(0X00F000);
			moveto(lim::rectaCoord[0].x , -lim::rectaCoord[0].y );
			for (size_t i = 0; i < lim::coordSize; i++)
				lineto(lim::rectaCoord[i].x, -lim::rectaCoord[i].y);
		}
		if (bIfPoint)
		{
			setfillcolor(RGB(0, 0, 0X80));
			for (size_t i = 0; i < lim::coordSize; i++)
				solidcircle(lim::rectaCoord[i].x, -lim::rectaCoord[i].y, 2);
		}

		FlushBatchDraw();
		std::this_thread::sleep_for(100ms);
	}

closeJmp:
	EndBatchDraw();
	closegraph();

	if (lim::isConnected)
	{
		LIM_HEAD* lim = NULL;
		LIM_Pack(lim, lim::USER_TASK_CID, LIM_CODE_IOSET_RELEASE, NULL);
		SendLIM(lim::USER_TASK_CID, lim, lim->nLIMLen);
		LIM_Release(lim);


		LIM_Pack(lim, lim::USER_TASK_CID, LIM_CODE_STOP_LMD, NULL);
		SendLIM(lim::USER_TASK_CID, lim, lim->nLIMLen);
		LIM_Release(lim);
		Sleep(1000);
	}

	CloseEquipmentComm(lim::USER_TASK_CID); // 关闭连接
	EquipmentCommDestory(); // 关闭库

	return 0;
}

