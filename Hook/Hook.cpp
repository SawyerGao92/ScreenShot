// KeyHook.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
//#include <fstream>

//using namespace std;

#define DLLEXPORT extern "C" __declspec(dllimport)

//����
HHOOK        hhkHook = NULL; //���平�Ӿ��
HINSTANCE    hInstance = NULL; //����ʵ��
//fstream        chOut;                //����ļ�
int keytime = 1;
//��������
DLLEXPORT    int        add(int a, int b);    //����
DLLEXPORT    BOOL    InstallHook();        //��װ����
DLLEXPORT    BOOL    UnInstallHook();    //ж�ع���

//���庯��
LRESULT CALLBACK KeyboardProc(
	int code,       // hook code
	WPARAM wParam,  // virtual-key code
	LPARAM lParam   // keystroke-message information
	);

//
////���
//BOOL APIENTRY DllMain(HANDLE hModule,
//	DWORD  ul_reason_for_call,
//	LPVOID lpReserved
//	)
//{
//	hInstance = (HINSTANCE)hModule;
//	return TRUE;
//}

//���ӵ�ʵ�ֲ���
LRESULT CALLBACK KeyboardProc(int code, WPARAM wParam, LPARAM lParam)
{
	KBDLLHOOKSTRUCT *Key_Info = (KBDLLHOOKSTRUCT*)lParam;
	if (HC_ACTION == code)
	{
		if (WM_KEYDOWN == wParam || WM_SYSKEYDOWN)  //�������Ϊ����״̬
		{
			//if (Key_Info->vkCode == VK_LWIN || Key_Info->vkCode == VK_RWIN) //���� WIN(����) ��
			//{
			//	return TRUE;
			//}
			//if (Key_Info->vkCode == 0x4D && ((GetKeyState(VK_LWIN) & 0x8000) ||
			//	(GetKeyState(VK_RWIN) & 0x8000))) //���� WIN+D ��ϼ�(����)
			//{
			//	return TRUE;
			//}
			//if (Key_Info->vkCode == 0x44 && ((GetKeyState(VK_LWIN) & 0x8000) ||
			//	(GetKeyState(VK_LWIN) & 0x8000)))  //���� WIN+M ��ϼ�(����)
			//{
			//	return TRUE;
			//}
			//if (Key_Info->vkCode == 0x1b && GetKeyState(VK_CONTROL) & 0x8000) //���� CTRL + ESC ��ϼ� 
			//{
			//	return TRUE;
			//}
			//if (Key_Info->vkCode == VK_TAB && Key_Info->flags & LLKHF_ALTDOWN) //���� ATL + TAB ��ϼ�
			//{
			//	return TRUE;
			//}
			//if (Key_Info->vkCode == VK_ESCAPE && Key_Info->flags & LLKHF_ALTDOWN) //���� ATL + ESC ��ϼ�
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
	return CallNextHookEx(0, code, wParam, lParam); //�ص�





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