
// LidarHeightTestDialogDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "LidarHeightTestDialog.h"
#include "LidarHeightTestDialogDlg.h"
#include "afxdialogex.h"
#include "../MultipleDeviceClient/LimDevice.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CLidarHeightTestDialogDlg 对话框



CLidarHeightTestDialogDlg::CLidarHeightTestDialogDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_LIDARHEIGHTTESTDIALOG_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CLidarHeightTestDialogDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CLidarHeightTestDialogDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTNSHOT, &CLidarHeightTestDialogDlg::OnBnClickedBtnshot)
	ON_BN_CLICKED(IDC_BTNCONNECT, &CLidarHeightTestDialogDlg::OnBnClickedBtnconnect)
END_MESSAGE_MAP()


// CLidarHeightTestDialogDlg 消息处理程序

BOOL CLidarHeightTestDialogDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CLidarHeightTestDialogDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CLidarHeightTestDialogDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CLidarHeightTestDialogDlg::OnBnClickedBtnshot()
{
	constexpr int Y_STEP = 5;
	constexpr int X_STEP = 5;
	constexpr int X_NUM = 6;
	constexpr int Y_NUM = 1800;

	auto& device = LimDevice::DeviceList.begin()->second;
	if (device.negHeightCoord.empty())
		return;
	device.LockCoord();

	int begin = ceil(device.negHeightCoord.front().x / (double)Y_STEP) * Y_STEP;
	int end = floor(device.negHeightCoord.back().x / (double)Y_STEP) * Y_STEP;


	CString str, coordLst;

	str.Format("%llu: {%d", std::chrono::system_clock::now().time_since_epoch().count(), device.negHeightCoord[begin - device.negHeightCoord.front().x].y);
	for (int x = begin + Y_STEP; x <= end; x += Y_STEP)
		str.Format("%s, %d", str.GetString(), device.negHeightCoord[x - device.negHeightCoord.front().x].y);
	str += "}\r\n";
	GetDlgItemText(IDC_COORD, coordLst);
	coordLst += str;
	SetDlgItemText(IDC_COORD, coordLst);
	((CEdit*)GetDlgItem(IDC_COORD))->SendMessage(WM_VSCROLL, MAKEWPARAM(SB_BOTTOM, 0), NULL);
	device.UnlockCoord();
}


void CLidarHeightTestDialogDlg::OnBnClickedBtnconnect()
{
	CString ip;
	LimDevice::InitEquipment();	

	GetDlgItemText(IDC_IPLIDAR, ip);

	LimDevice::OpenEquipment(ip.GetString()/*"192.168.1.210"*/);
	LimDevice::WaitFirstDeviceTryConnected();
	if (LimDevice::OnlineDeviceNumber > 0)
	{
		LimDevice::StartLMDData();
		GetDlgItem(IDC_BTNSHOT)->EnableWindow();
	}
}
