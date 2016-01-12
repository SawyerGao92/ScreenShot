// ScreenCapture.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "ScreenShot.h"
#include "shellapi.h"
#include <io.h>		//access函数头文件
#define SHOT_NAME "ScreenShot"
#define SHOT_TYPE ".bmp"

#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;								// 当前实例
TCHAR szTitle[MAX_LOADSTRING];					// 标题栏文本
TCHAR szWindowClass[MAX_LOADSTRING];			// 主窗口类名

// 此代码模块中包含的函数的前向声明:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

HDC g_srcMemDc;
HDC g_grayMemDc;
int screenW;
int screenH;

RECT rect = { 0 };   //画图的矩形区域
bool isDrawing = false;
bool isSelect = false;


void GetScreenCapture();
void CovertToGrayBitmap(HBITMAP hSourceBmp, HDC sourceDc);
void WriteDatatoClipBoard();

HMODULE    m_hDll = NULL;
typedef BOOL(*INSPROC)(void);
typedef BOOL(*UNINSPROC)(void);
HBITMAP hBmp, hOldBmp;
BOOL  SaveBmp(HBITMAP  hBitmap, CString  FileName);
//#include <windows.h> 
//#include "resource.h" 

#define WM_TRAY WM_USER+100
#define WM_TASKBAR_CREATED RegisterWindowMessage(TEXT("TaskbarCreated")) 

//#define L"托盘程序"    TEXT(L"托盘程序") 
//#define L"Win32 API 实现系统托盘程序"     TEXT(L"Win32 API 实现系统托盘程序") 

NOTIFYICONDATA nid;		//托盘属性 
HMENU hMenu;			//托盘菜单 

void ShowTrayMsg();
void InitTray(HINSTANCE hInstance, HWND hWnd);
HWND hWnd;
int rown = 0;

int timess = 0;
#define TIMESS 50					//颜色变动的快慢

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: 在此放置代码。
	MSG msg;
	HACCEL hAccelTable;

	// 初始化全局字符串
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_SCREENSHOT, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 执行应用程序初始化:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SCREENSHOT));

	InitTray(hInstance, hWnd);			//实例化托盘 
	SetTimer(hWnd, 3, 1000, NULL);		//定时发消息，演示气泡提示功能 




	// 主消息循环:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目的: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDC_SCREENSHOT));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = 0;//MAKEINTRESOURCE(IDC_SCREENCAPTURE);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目的: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	//findwindow 是否已经存在，只打开一次
	HWND Screenshot_hwnd = FindWindowA(NULL, "ScreenShot");
	if (Screenshot_hwnd != 0)
	{
		MessageBoxA(hWnd, "程序已经在后台运行中...", "ScreenShot", MB_OK);
		exit(0);
	}


	hInst = hInstance; // 将实例句柄存储在全局变量中

	hWnd = CreateWindow(szWindowClass, szTitle, WS_POPUP,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, SW_MAXIMIZE);
	ShowWindow(hWnd, SW_HIDE);
	UpdateWindow(hWnd);


	//初始化HOOK
	m_hDll = LoadLibrary(L"Hook.dll");

	if (m_hDll != NULL)
	{
		INSPROC    instproc;
		instproc = (INSPROC)GetProcAddress(m_hDll, "InstallHook");
		BOOL bResult = instproc();
	}
	else
	{
		exit(1);
	}


	return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的: 处理主窗口的消息。
//
//  WM_COMMAND	- 处理应用程序菜单
//  WM_PAINT	- 绘制主窗口
//  WM_DESTROY	- 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	LOGBRUSH brush;
	brush.lbStyle = BS_NULL;
	HBRUSH hBrush = CreateBrushIndirect(&brush);

	LOGPEN pen;
	POINT penWidth;
	penWidth.x = 2;
	penWidth.y = 2;

	int color[7][3] = { { 254, 67, 101 }, { 217, 104, 49 }, { 250, 227, 113 }, { 131, 175, 155 }, { 69, 137, 148 }, { 1, 77, 103 }, { 89, 61, 67 } };
	pen.lopnColor = RGB(color[rown][0], color[rown][1], color[rown][2]);
	timess = timess + 1;
	if (timess == TIMESS)
	{
		rown = rown + 1;
		timess = 0;
	}
	if (rown == 6)
		rown = -0;

	pen.lopnStyle = PS_SOLID;
	pen.lopnWidth = penWidth;
	HPEN hPen = CreatePenIndirect(&pen);


	switch (message)
	{
	case WM_TRAY:
		switch (lParam)
		{
		case WM_RBUTTONDOWN:
		{
			//获取鼠标坐标 
			POINT pt; GetCursorPos(&pt);

			//解决在菜单外单击左键菜单不消失的问题 
			SetForegroundWindow(hWnd);

			//使菜单某项变灰 
			//EnableMenuItem(hMenu, ID_SHOW, MF_GRAYED);     

			//显示并获取选中的菜单 
			int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD, pt.x, pt.y, NULL, hWnd,
				NULL);
			if (cmd == ID_SHOW)
				MessageBox(hWnd, L"Win32 API 实现系统托盘程序", L"托盘程序", MB_OK);
			if (cmd == ID_EXIT)
				PostMessage(hWnd, WM_DESTROY, NULL, NULL);
		}
		break;
		case WM_LBUTTONDOWN:
			MessageBox(hWnd, L"Win32 API 实现系统托盘程序", L"托盘程序", MB_OK);
			break;
		case WM_LBUTTONDBLCLK:
			break;
		}
		break;

	case WM_TIMER:
		ShowTrayMsg();
		KillTimer(hWnd, wParam);
		break;

	case WM_COPYDATA:
		if (wParam == -1 && lParam == -1)
		{
			GetScreenCapture();
			ShowWindow(hWnd, SW_SHOW);
			UpdateWindow(hWnd);
		}


	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// 分析菜单选择:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

	case WM_CREATE:
		//GetScreenCapture();
		break;
	case WM_PAINT:
	{
		hdc = BeginPaint(hWnd, &ps);

		HDC memDc = CreateCompatibleDC(hdc);
		HBITMAP bmp = CreateCompatibleBitmap(hdc, screenW, screenH);
		SelectObject(memDc, bmp);

		BitBlt(memDc, 0, 0, screenW, screenH, g_grayMemDc, 0, 0, SRCCOPY);
		SelectObject(memDc, hBrush);
		SelectObject(memDc, hPen);

		if (isDrawing || isSelect)
		{

			BitBlt(memDc, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, g_srcMemDc, rect.left, rect.top, SRCCOPY);
			Rectangle(memDc, rect.left, rect.top, rect.right, rect.bottom);
		}

		BitBlt(hdc, 0, 0, screenW, screenH, memDc, 0, 0, SRCCOPY);

		DeleteObject(bmp);
		DeleteObject(memDc);

		EndPaint(hWnd, &ps);
	}
	break;

	case WM_LBUTTONDOWN:
	{
		if (!isSelect)
		{
			POINT pt;
			GetCursorPos(&pt);
			rect.left = pt.x;
			rect.top = pt.y;
			rect.right = pt.x;
			rect.bottom = pt.y;

			isDrawing = true;
			InvalidateRgn(hWnd, 0, false);
		}
	}
	break;

	case WM_LBUTTONUP:
	{
		if (isDrawing && !isSelect)
		{
			isDrawing = false;
			POINT pt;
			GetCursorPos(&pt);
			rect.right = pt.x;
			rect.bottom = pt.y;

			isSelect = true;

			InvalidateRgn(hWnd, 0, false);
		}
	}
	break;

	case WM_MOUSEMOVE:
	{
		if (isDrawing&& !isSelect)
		{
			POINT pt;
			GetCursorPos(&pt);
			rect.right = pt.x;
			rect.bottom = pt.y;
			InvalidateRgn(hWnd, 0, false);
		}
	}
	break;
	case WM_LBUTTONDBLCLK:
	{
		if (isSelect)
		{
			WriteDatatoClipBoard();
			InvalidateRgn(hWnd, 0, false);
			ShowWindow(hWnd, SW_HIDE);
			//ShowWindow(hWnd, SW_MINIMIZE);



			TCHAR MyDir[_MAX_PATH];
			SHGetSpecialFolderPath(hWnd, MyDir, CSIDL_DESKTOP, 0);	//获取特殊路径，获取桌面路径
			CString  str;
			str.Format(_T("%s"), MyDir);
			CString depart = "\\";
			CString lkuohao = "(";
			CString rkuohao = ")";
			CString name = SHOT_NAME;
			CString cstype = SHOT_TYPE;
			//cstring to char*
			CString strPath = str + depart + name + cstype;
			//char *path = (LPSTR)(LPCTSTR)cstr;

			//CString strPath = L"C:\\Users\\Wayne\\Desktop\\2015年最新软件进展\\call_log.csv";


			int nLength = strPath.GetLength();
			int nBytes = WideCharToMultiByte(CP_ACP, 0, strPath, nLength, NULL, 0, NULL, NULL);
			char* VoicePath = new char[nBytes + 1];
			memset(VoicePath, 0, nLength + 1);
			WideCharToMultiByte(CP_OEMCP, 0, strPath, nLength, VoicePath, nBytes, NULL, NULL);
			VoicePath[nBytes] = 0;

			//strPath    *VoicePath

			int woc = 2;
			CString woca;
			CString cstrn = strPath;

			while (_access(VoicePath, 0) == 0)
			{
				woca.Format(_T("%d"), woc);
				cstrn = strPath + lkuohao + woca + rkuohao + cstype;
				//判断有没有~
				int n2Length = cstrn.GetLength();
				int n2Bytes = WideCharToMultiByte(CP_ACP, 0, cstrn, n2Length, NULL, 0, NULL, NULL);
				VoicePath = new char[n2Bytes + 1];
				memset(VoicePath, 0, n2Length + 1);
				WideCharToMultiByte(CP_OEMCP, 0, cstrn, n2Length, VoicePath, n2Bytes, NULL, NULL);
				VoicePath[n2Bytes] = 0;
				woc++;
			}

			SaveBmp(hBmp, cstrn);	//使用下面的函数

		}
		isSelect = false;

	}
	break;
	case WM_DESTROY:
	{//窗口销毁时删除托盘 
		Shell_NotifyIcon(NIM_DELETE, &nid);

		UNINSPROC    uninstproc;
		uninstproc = (UNINSPROC)GetProcAddress(m_hDll, "UnInstallHook");
		BOOL bResult = uninstproc();

		::FreeLibrary(m_hDll);
		m_hDll = NULL;

		PostQuitMessage(0);
		break;
	}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}


void GetScreenCapture()
{
	HDC disDc = ::CreateDC(L"DISPLAY", 0, 0, 0);  //创建屏幕相关的DC
	screenW = GetDeviceCaps(disDc, HORZRES);//水平分辨率
	screenH = GetDeviceCaps(disDc, VERTRES);//垂直分辨率

	g_srcMemDc = CreateCompatibleDC(disDc);  //创建于屏幕兼容的DC（内存DC）
	HBITMAP hbMap = CreateCompatibleBitmap(disDc, screenW, screenH);  //模拟一张画布，其中是没有数据的
	SelectObject(g_srcMemDc, hbMap);   //将画图选入内存DC，其中还是没有数据的
	BitBlt(g_srcMemDc, 0, 0, screenW, screenH, disDc, 0, 0, SRCCOPY);  //将屏幕的dc中的画图，拷贝至内存DC中

	//获取屏幕的灰度图片
	g_grayMemDc = CreateCompatibleDC(disDc);
	HBITMAP grayMap = CreateCompatibleBitmap(disDc, screenW, screenH);  //模拟一张画布，其中是没有数据的
	SelectObject(g_grayMemDc, grayMap);   //将画图选入内存DC，其中还是没有数据的
	BitBlt(g_grayMemDc, 0, 0, screenW, screenH, disDc, 0, 0, SRCCOPY);  //将屏幕的dc中的画图，拷贝至内存DC中

	CovertToGrayBitmap(grayMap, g_grayMemDc);  //将彩色图片转换灰度图片

	DeleteObject(hbMap);
	DeleteObject(grayMap);
	DeleteObject(disDc);
}

void CovertToGrayBitmap(HBITMAP hSourceBmp, HDC sourceDc)
{
	HBITMAP retBmp = hSourceBmp;
	BITMAPINFO bmpInfo;
	ZeroMemory(&bmpInfo, sizeof(BITMAPINFO));
	bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	GetDIBits(sourceDc, retBmp, 0, 0, NULL, &bmpInfo, DIB_RGB_COLORS);

	BYTE* bits = new BYTE[bmpInfo.bmiHeader.biSizeImage];
	GetBitmapBits(retBmp, bmpInfo.bmiHeader.biSizeImage, bits);

	int bytePerPixel = 4;//默认32位
	if (bmpInfo.bmiHeader.biBitCount == 24)
	{
		bytePerPixel = 3;
	}
	for (DWORD i = 0; i < bmpInfo.bmiHeader.biSizeImage; i += bytePerPixel)
	{
		BYTE r = *(bits + i);
		BYTE g = *(bits + i + 1);
		BYTE b = *(bits + i + 2);
		*(bits + i) = *(bits + i + 1) = *(bits + i + 2) = (r + b + g) / 3;
	}
	SetBitmapBits(hSourceBmp, bmpInfo.bmiHeader.biSizeImage, bits);
	delete[] bits;
}

void WriteDatatoClipBoard()
{
	HDC hMemDc, hScrDc;

	int width, height;
	width = rect.right - rect.left;
	height = rect.bottom - rect.top;

	hScrDc = CreateDC(L"DISPLAY", NULL, NULL, NULL);
	hMemDc = CreateCompatibleDC(hScrDc);
	hBmp = CreateCompatibleBitmap(hScrDc, width, height);

	hOldBmp = (HBITMAP)SelectObject(hMemDc, hBmp);
	BitBlt(hMemDc, 0, 0, width, height, g_srcMemDc, rect.left, rect.top, SRCCOPY);
	hBmp = (HBITMAP)SelectObject(hMemDc, hOldBmp);
	DeleteDC(hMemDc);
	DeleteDC(hScrDc);
	//复制到剪贴板
	if (OpenClipboard(0))
	{
		EmptyClipboard();
		SetClipboardData(CF_BITMAP, hBmp);
		CloseClipboard();
	}

	DeleteObject(hBmp);
	DeleteObject(hMemDc);
	DeleteObject(hScrDc);

}







//VC下把HBITMAP保存为bmp图片   
BOOL  SaveBmp(HBITMAP     hBitmap, CString     FileName)
{
	HDC     hDC;
	//当前分辨率下每象素所占字节数         
	int     iBits;
	//位图中每象素所占字节数         
	WORD     wBitCount;
	//定义调色板大小，     位图中像素字节大小     ，位图文件大小     ，     写入文件字节数             
	DWORD     dwPaletteSize = 0, dwBmBitsSize = 0, dwDIBSize = 0, dwWritten = 0;
	//位图属性结构             
	BITMAP     Bitmap;
	//位图文件头结构         
	BITMAPFILEHEADER     bmfHdr;
	//位图信息头结构             
	BITMAPINFOHEADER     bi;
	//指向位图信息头结构                 
	LPBITMAPINFOHEADER     lpbi;
	//定义文件，分配内存句柄，调色板句柄             
	HANDLE     fh, hDib, hPal, hOldPal = NULL;

	//计算位图文件每个像素所占字节数             
	hDC = CreateDC(L"DISPLAY", NULL, NULL, NULL);
	iBits = GetDeviceCaps(hDC, BITSPIXEL)     *     GetDeviceCaps(hDC, PLANES);
	DeleteDC(hDC);
	if (iBits <= 1)
		wBitCount = 1;
	else  if (iBits <= 4)
		wBitCount = 4;
	else if (iBits <= 8)
		wBitCount = 8;
	else
		wBitCount = 24;

	GetObject(hBitmap, sizeof(Bitmap), (LPSTR)&Bitmap);
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = Bitmap.bmWidth;
	bi.biHeight = Bitmap.bmHeight;
	bi.biPlanes = 1;
	bi.biBitCount = wBitCount;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrImportant = 0;
	bi.biClrUsed = 0;

	dwBmBitsSize = ((Bitmap.bmWidth *wBitCount + 31) / 32) * 4 * Bitmap.bmHeight;

	//为位图内容分配内存             
	hDib = GlobalAlloc(GHND, dwBmBitsSize + dwPaletteSize + sizeof(BITMAPINFOHEADER));
	lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);
	*lpbi = bi;

	//     处理调色板                 
	hPal = GetStockObject(DEFAULT_PALETTE);
	if (hPal)
	{
		hDC = ::GetDC(NULL);
		hOldPal = ::SelectPalette(hDC, (HPALETTE)hPal, FALSE);
		RealizePalette(hDC);
	}

	//     获取该调色板下新的像素值             
	GetDIBits(hDC, hBitmap, 0, (UINT)Bitmap.bmHeight,
		(LPSTR)lpbi + sizeof(BITMAPINFOHEADER) + dwPaletteSize,
		(BITMAPINFO *)lpbi, DIB_RGB_COLORS);

	//恢复调色板                 
	if (hOldPal)
	{
		::SelectPalette(hDC, (HPALETTE)hOldPal, TRUE);
		RealizePalette(hDC);
		::ReleaseDC(NULL, hDC);
	}


	//创建位图文件                 
	fh = CreateFile(FileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

	if (fh == INVALID_HANDLE_VALUE)         return     FALSE;

	//     设置位图文件头             
	bmfHdr.bfType = 0x4D42;     //     "BM"             
	dwDIBSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwPaletteSize + dwBmBitsSize;
	bmfHdr.bfSize = dwDIBSize;
	bmfHdr.bfReserved1 = 0;
	bmfHdr.bfReserved2 = 0;
	bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER) + dwPaletteSize;
	//     写入位图文件头             
	WriteFile(fh, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);
	//     写入位图文件其余内容             
	WriteFile(fh, (LPSTR)lpbi, dwDIBSize, &dwWritten, NULL);
	//清除                 
	GlobalUnlock(hDib);
	GlobalFree(hDib);
	CloseHandle(fh);

	return     TRUE;
}


//实例化托盘 
void InitTray(HINSTANCE hInstance, HWND hWnd)
{
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = hWnd;
	nid.uID = IDI_TRAY;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_INFO;
	nid.uCallbackMessage = WM_TRAY;
	nid.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TRAY));
	lstrcpy(nid.szTip, L"ScreenShot");

	hMenu = CreatePopupMenu();//生成托盘菜单 
	//为托盘菜单添加两个选项 
	AppendMenu(hMenu, MF_STRING, ID_MARK, TEXT("简介"));
	AppendMenu(hMenu, MF_STRING, ID_SHOW, TEXT("提示"));
	AppendMenu(hMenu, MF_STRING, ID_EXIT, TEXT("退出"));

	Shell_NotifyIcon(NIM_ADD, &nid);
}

//演示托盘气泡提醒 
void ShowTrayMsg()
{
	lstrcpy(nid.szInfoTitle, L"ScreenShot");
	lstrcpy(nid.szInfo, TEXT("后台运行中..."));
	nid.uTimeout = 1000;
	Shell_NotifyIcon(NIM_MODIFY, &nid);

}