#include <httplib.h>
#include <json.hpp>
#include <sstream>
#include <iomanip>
#include <conio.h>
#include "../LidarDataPackage/fps.h"
#include "../LidarDataPackage/EasyPX.h"


using namespace nlohmann;
using namespace std;
using namespace httplib;
using namespace thatboy;

Client client("localhost", 5056);
map<int, int> points;

void writePoints()
{
	auto res = client.Get("/height?cid=128");
	json c;
	if (res)
		c = json::parse(res->body);
	auto height = c["h"];
	points.clear();
	int x = c["begin"].get<int>();
	for (auto& p : height)
		points[x++] = p.get<int>();
}

int main()
{
	EasyPX::initgraph(900, 900, EasyPX::EW_HASMAXIMIZI | EasyPX::EW_HASSIZEBOX);
	setbkcolor(WHITE);
	setlinestyle(PS_SOLID, 1);
	settextcolor(BLACK);
	setbkmode(TRANSPARENT);

	bool bIfLine = true;
	bool bIfPoint = true;
	bool bIfBg = true;
	bool bIfInfo = true;
	bool bIfMeasure = true;
	bool bIfGrid = true;
	bool bIfArea = true;
	bool bIfFixed = true;
	bool bIfFromInfoBox = false;

	MOUSEMSG Msg = { NULL };
	POINT mousePos = { NULL };
	POINT originPos = { NULL };
	size_t shutDownGridnfoFrameCount = 0;

	EasyPX::UI::InfoBox infoBox;

	originPos.x = getwidth() / 2;
	originPos.y = getheight() / 2;
	float scale = 1;
	setaspectratio(scale, scale);
	setorigin(originPos.x, originPos.y);

	infoBox.addInfo("I", "I - Info");
	infoBox.addInfo("P", "P - Point");
	infoBox.addInfo("L", "L - Line");
	infoBox.addInfo("A", "A - Area");
	infoBox.addInfo("B", "B - Background");
	infoBox.addInfo("G", "G - Grid");
	infoBox.addInfo("M", "M - Measure");
	infoBox.addInfo("F", "F - Fix the Info");
	infoBox.addInfo("R", "R - Reset the view");
	infoBox.addInfo(" ", " ");
	infoBox.addInfo("Scale", "Scale:");
	infoBox.addInfo("Angle", "Angle:");
	infoBox.addInfo("FPS", "FPS:");
	infoBox.addInfo("Borders", "Borders:");
	infoBox.addInfo("Len", "BorderContinusLen:");
	infoBox.addInfo("grid", "The fps is lower than 5, grid drawing has been turned off automatically:");
	infoBox.addInfo("fpsWarn", "The fps is lower than 20, recommend to turn off grid drawing by \'G\'.");

	infoBox.setArg("Scale", "1");
	infoBox.setArg("FPS", "0.00");

	infoBox.showInfo("grid", false);
	infoBox.showInfo("fpsWarn", false);
	infoBox.setInfoColor("grid", RED);
	infoBox.setInfoColor("fpsWarn", RED);

	infoBox.setArg("F", "Fixed");
	infoBox.setArg("I", "Show");
	infoBox.setArg("P", "Show");
	infoBox.setArg("L", "Show");
	infoBox.setArg("A", "Show");
	infoBox.setArg("B", "Show");
	infoBox.setArg("G", "Show");
	infoBox.setArg("M", "Show");

	infoBox.setArgColor("F", GREEN);
	infoBox.setArgColor("I", GREEN);
	infoBox.setArgColor("P", GREEN);
	infoBox.setArgColor("L", GREEN);
	infoBox.setArgColor("A", GREEN);
	infoBox.setArgColor("B", GREEN);
	infoBox.setArgColor("G", GREEN);
	infoBox.setArgColor("M", GREEN);

	infoBox.x = 10;
	infoBox.y = 10;

	BeginBatchDraw();
	FlushMouseMsgBuffer();
	while (true)
	{
		writePoints();
		if (MouseHit())
		{
			bool bIfOriChanged = false;
			bool bIfScaleChanged = false;
			while (MouseHit())
			{
				Msg = GetMouseMsg();
				if (Msg.wheel != 0)
				{
				
						while (Msg.wheel > 0)
						{
							scale *= 1.1;
							Msg.wheel -= 120;
							if (scale > 400)
							{
								Msg.wheel = 0;
								scale = 400;
							}
						}
						while (Msg.wheel < 0)
						{
							scale *= 0.9;
							Msg.wheel += 120;
							if (scale < 0.000125)
							{
								Msg.wheel = 0;
								scale = 0.000125;
							}
						}
						bIfScaleChanged = true;
						bIfOriChanged = true;
					
				}
				if (Msg.uMsg == WM_LBUTTONDOWN || Msg.uMsg == WM_RBUTTONDOWN)
				{
					bIfFromInfoBox = !bIfFixed && (Msg.x > infoBox.x + originPos.x && Msg.y > infoBox.y + originPos.y && Msg.x < infoBox.x + originPos.x + infoBox.getInfoBoxWidth() && Msg.y < infoBox.y + originPos.y + infoBox.getInfoBoxHeight());
					mousePos.x = Msg.x;
					mousePos.y = Msg.y;
				}
				if (Msg.mkLButton)
				{
					if (bIfFromInfoBox)
					{
						infoBox.x += Msg.x - mousePos.x;
						infoBox.y += Msg.y - mousePos.y;
						mousePos.x = Msg.x;
						mousePos.y = Msg.y;
					}
					else
					{
						originPos.x += Msg.x - mousePos.x;
						originPos.y += Msg.y - mousePos.y;
						mousePos.x = Msg.x;
						mousePos.y = Msg.y;
						bIfOriChanged = true;
					}
				}
			}
			if (bIfScaleChanged)
			{
				setaspectratio(scale, scale);
				infoBox.setArg("Scale", scale, true, 0, 4);
			}
			if (bIfOriChanged)
				setorigin(originPos.x, originPos.y);
		}
		if (_kbhit())
		{
			switch (_getch())
			{
			case 'a':
			case 'A':
				bIfArea = !bIfArea;
				infoBox.setArg("A", bIfArea ? "Show" : "Hide");
				infoBox.setArgColor("A", bIfArea ? GREEN : RED);
				break;
			case 'i':
			case 'I':
				bIfInfo = !bIfInfo;
				infoBox.setArg("I", bIfInfo ? "Show" : "Hide");
				infoBox.setArgColor("I", bIfInfo ? GREEN : RED);
				break;
			case 'b':
			case 'B':
				bIfBg = !bIfBg;
				infoBox.setArg("B", bIfBg ? "Show" : "Hide");
				infoBox.setArgColor("B", bIfBg ? GREEN : RED);
				break;
			case 'l':
			case 'L':
				bIfLine = !bIfLine;
				infoBox.setArg("L", bIfLine ? "Show" : "Hide");
				infoBox.setArgColor("L", bIfLine ? GREEN : RED);
				break;
			case 'p':
			case 'P':
				bIfPoint = !bIfPoint;
				infoBox.setArg("P", bIfPoint ? "Show" : "Hide");
				infoBox.setArgColor("P", bIfPoint ? GREEN : RED);
				break;
			case 'f':
			case 'F':
				bIfFixed = !bIfFixed;
				infoBox.setArg("F", bIfFixed ? "Fixed" : "Unfixed", true);
				infoBox.setArgColor("F", bIfFixed ? GREEN : RED);
				if (bIfFixed)
				{
					infoBox.x += originPos.x;
					infoBox.y += originPos.y;
				}
				else
				{
					infoBox.x -= originPos.x;
					infoBox.y -= originPos.y;
				}
				break;
			case 'g':
			case 'G':
				bIfGrid = !bIfGrid;
				infoBox.setArg("G", bIfGrid ? "Show" : "Hide");
				infoBox.setArgColor("G", bIfGrid ? GREEN : RED);
				break;
			case 'm':
			case 'M':
				bIfMeasure = !bIfMeasure;
				infoBox.setArg("M", bIfMeasure ? "Show" : "Hide");
				infoBox.setArgColor("M", bIfMeasure ? GREEN : RED);
				break;
			case 'r':
			case 'R':
				scale = 1;
				originPos.x = getwidth() / 2;
				originPos.y = getheight() / 2;
				setaspectratio(scale, scale);
				setorigin(originPos.x, originPos.y);
				infoBox.setArg("Scale", scale, true, 0, 4);
				break;
			case 'q':
			case 'Q':
				goto closeJmp;
			default:
				break;
			}
		}
		cleardevice();

		if (bIfBg)
		{

			// 网格
			if (bIfGrid)
			{
				setlinecolor(0XB4A8A8);
				constexpr int gridSize = 1000;
				for (int i = 0; i <= 10000 / gridSize; i++)
				{
					for (int j = 0; j <= 10000 / gridSize; j++)
					{
						line(-i * gridSize, -j * gridSize, i * gridSize, -j * gridSize);
						line(-i * gridSize, j * gridSize, i * gridSize, j * gridSize);
						line(-i * gridSize, -j * gridSize, -i * gridSize, j * gridSize);
						line(i * gridSize, -j * gridSize, i * gridSize, j * gridSize);
					}
				}
				constexpr int smallGridSize = 100;
				constexpr int smallAreaSize = 1000;
				for (int i = 0; i <= smallAreaSize / smallGridSize; i++)
				{
					for (int j = 0; j <= smallAreaSize / smallGridSize; j++)
					{
						line(-i * smallGridSize, -j * smallGridSize, i * smallGridSize, -j * smallGridSize);
						line(-i * smallGridSize, j * smallGridSize, i * smallGridSize, j * smallGridSize);
						line(-i * smallGridSize, -j * smallGridSize, -i * smallGridSize, j * smallGridSize);
						line(i * smallGridSize, -j * smallGridSize, i * smallGridSize, j * smallGridSize);
					}
				}

				constexpr int littleGridSize = 10;
				constexpr int littleAreaSize = 100;
				for (int i = 0; i <= littleAreaSize / littleGridSize; i++)
				{
					for (int j = 0; j <= littleAreaSize / littleGridSize; j++)
					{
						line(-i * littleGridSize, -j * littleGridSize, i * littleGridSize, -j * littleGridSize);
						line(-i * littleGridSize, j * littleGridSize, i * littleGridSize, j * littleGridSize);
						line(-i * littleGridSize, -j * littleGridSize, -i * littleGridSize, j * littleGridSize);
						line(i * littleGridSize, -j * littleGridSize, i * littleGridSize, j * littleGridSize);
					}
				}
			}

			setlinecolor(0X423E3E);
			setlinestyle(PS_SOLID, 3);
			line(-10000, 0, 10000, 0);
			line(0, -10000, 0, 0);

		}


		if (bIfArea || bIfLine)
		{
			setlinecolor(0X00F000);
			setfillcolor(0XF37E31);
			if (!points.empty())
			{
				for (auto& [x, y] : points)
					line(x, y, x, 400);
			}
		}
		// 中心点
		setfillcolor(RGB(0XF0, 0, 0));
		solidcircle(0, 0, 3);


		if (bIfInfo)
		{
			double fps = ::fps();

			infoBox.setArg("FPS", fps, false, 0, 4);

			if (fps < 5)
			{
				bIfGrid = false;
				infoBox.setArg("G", "Hide");
				infoBox.setArgColor("G", RED);
				infoBox.showInfo("grid", true);
				shutDownGridnfoFrameCount = 500;
			}
			if (shutDownGridnfoFrameCount > 0 && --shutDownGridnfoFrameCount == 1)
				infoBox.showInfo("grid", false);
			else
				infoBox.showInfo("fpsWarn", fps < 20);

			if (bIfFixed)
				infoBox.drawInfo(infoBox.x - originPos.x, infoBox.y - originPos.y);
			else
				infoBox.drawInfo();
		}

		FlushBatchDraw();
		using namespace std::chrono;
		std::this_thread::sleep_for(5ms);
	}

closeJmp:
	EndBatchDraw();
	closegraph();
	return 0;
}
