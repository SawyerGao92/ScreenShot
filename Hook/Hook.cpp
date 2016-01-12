// KeyHook.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
//#include <fstream>

//using namespace std;

#define DLLEXPORT extern "C" __declspec(dllimport)

//变量
HHOOK        hhkHook = NULL; //定义钩子句柄
HINSTANCE    hInstance = NULL; //程序实例
//fstream        chOut;                //输出文件
int keytime = 1;
//导出函数
DLLEXPORT    int        add(int a, int b);    //测试
DLLEXPORT    BOOL    InstallHook();        //安装钩子
DLLEXPORT    BOOL    UnInstallHook();    //卸载钩子

//定义函数
LRESULT CALLBACK KeyboardProc(
	int code,       // hook code
	WPARAM wParam,  // virtual-key code
	LPARAM lParam   // keystroke-message information
	);

//
////入口
//BOOL APIENTRY DllMain(HANDLE hModule,
//	DWORD  ul_reason_for_call,
//	LPVOID lpReserved
//	)
//{
//	hInstance = (HINSTANCE)hModule;
//	return TRUE;
//}

//钩子的实现部分
LRESULT CALLBACK KeyboardProc(int code, WPARAM wParam, LPARAM lParam)
{
	KBDLLHOOKSTRUCT *Key_Info = (KBDLLHOOKSTRUCT*)lParam;
	if (HC_ACTION == code)
	{
		if (WM_KEYDOWN == wParam || WM_SYSKEYDOWN)  //如果按键为按下状态
		{
			//if (Key_Info->vkCode == VK_LWIN || Key_Info->vkCode == VK_RWIN) //屏敝 WIN(左右) 键
			//{
			//	return TRUE;
			//}
			//if (Key_Info->vkCode == 0x4D && ((GetKeyState(VK_LWIN) & 0x8000) ||
			//	(GetKeyState(VK_RWIN) & 0x8000))) //屏敝 WIN+D 组合键(左右)
			//{
			//	return TRUE;
			//}
			//if (Key_Info->vkCode == 0x44 && ((GetKeyState(VK_LWIN) & 0x8000) ||
			//	(GetKeyState(VK_LWIN) & 0x8000)))  //屏敝 WIN+M 组合键(左右)
			//{
			//	return TRUE;
			//}
			//if (Key_Info->vkCode == 0x1b && GetKeyState(VK_CONTROL) & 0x8000) //屏敝 CTRL + ESC 组合键 
			//{
			//	return TRUE;
			//}
			//if (Key_Info->vkCode == VK_TAB && Key_Info->flags & LLKHF_ALTDOWN) //屏敝 ATL + TAB 组合键
			//{
			//	return TRUE;
			//}
			//if (Key_Info->vkCode == VK_ESCAPE && Key_Info->flags & LLKHF_ALTDOWN) //屏敝 ATL + ESC 组合键
			//{
			//	return TRUE;
			//}
			if (Key_Info->vkCode == VK_F12) // && Key_Info->flags & VK_MENU)
			{
				if (keytime == 1)
				{//MessageBoxA(NULL, "f12", "success", MB_OK);
					HWND Screenshot_hwnd = FindWindowA(NULL, "ScreenShot");
					::SendMessage(Screenshot_hwnd, WM_COPYDATA, -1, -1);
					keytime++;
				}
				else
				{
					keytime = 1;
					return TRUE;
				}
			}
			//if (Key_Info->vkCode == VK_ESCAPE)
			//{
			//	HWND Screenshot_hwnd = FindWindowA(NULL, "ScreenShot");
			//	;; SendMessage(Screenshot_hwnd, SWP_HIDEWINDOW, -2, -2);
			//}

		}

	}
	return CallNextHookEx(0, code, wParam, lParam); //回调





}

BOOL InstallHook()
{
	//chOut.open("c:\\keyhook.log", ios::app | ios::out);
	hhkHook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)KeyboardProc, hInstance, 0);

	if (hhkHook != NULL)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL UnInstallHook()
{
	//if (chOut.is_open)
	//{
	//chOut.close();
	//}

	return UnhookWindowsHookEx(hhkHook);
}