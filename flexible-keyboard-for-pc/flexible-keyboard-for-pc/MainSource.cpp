#include <WinSock2.h>
#include<Windows.h>

#include<BluetoothAPIs.h>

#include <ws2bth.h>
#include <tchar.h>
#include <iostream>
#include "SerialPortProcessor.h"
#include <SetupAPI.h>
#include <stdio.h>
#include <locale.h>


#define BUTTON1 100

#pragma comment(lib,"Bthprops")
using namespace std;

bool CloseFlag = false;

void SendKey(unsigned char code)
{
	keybd_event(code, 0x00, KEYEVENTF_EXTENDEDKEY | 0, 0);
	keybd_event(code, 0x00, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
}

void SendCtrlKey(unsigned char code, unsigned char ctrl) {
	keybd_event(ctrl, 0x00, KEYEVENTF_EXTENDEDKEY | 0, 0);
	SendKey(code);
	keybd_event(ctrl, 0x00, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
}

//仮想キーコードに変換
void ChangeStr(char *str) {
		if ('0' <= *str && '9' >= *str) {
			*str = *str + 48;
		}
		else if(*str >= 17 && *str <= 20){
			switch (*str)
			{
			case 17:
				*str = VK_LEFT;
				break;
			case 18:
				*str = VK_UP;
				break;
			case 19:
				*str = VK_RIGHT;
				break;
			case 20:
				*str = VK_DOWN;
				break;
			default:
				break;
			}
		}
		else if ('.' == *str) {
			*str = *str + (110 - 46);
		}
		else if (*str == 10) {
			*str = *str + 3;
		}
		else if(*str == 21){
			CloseFlag = true;
		}
}

void SearchBlueTooth() {
	BLUETOOTH_DEVICE_SEARCH_PARAMS params = { 0 };
	params.fReturnAuthenticated = TRUE;
	params.fReturnRemembered = TRUE;
	params.fReturnUnknown = TRUE;
	params.fReturnConnected = TRUE;
	params.fIssueInquiry = TRUE;
	params.cTimeoutMultiplier = 7;
	params.dwSize = sizeof(BLUETOOTH_DEVICE_SEARCH_PARAMS);

	BLUETOOTH_DEVICE_INFO_STRUCT BlueToothinfo = { 0 };
	BlueToothinfo.dwSize = sizeof(BLUETOOTH_DEVICE_INFO_STRUCT);

	HBLUETOOTH_DEVICE_FIND find = BluetoothFindFirstDevice(&params, &BlueToothinfo);

	if (find) {
		do {
			MessageBox(NULL, BlueToothinfo.szName, _T("BluetoothDevice"), MB_OK);
		} while (BluetoothFindNextDevice(find, &BlueToothinfo));
		BluetoothFindDeviceClose(find);
	}
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%window用

int SetWinCenter(HWND hWnd,int Xsplit,int Ysplit,int Yheight)
{
	HWND hDeskWnd;
	RECT deskrc, rc;
	int x, y;

	hDeskWnd = GetDesktopWindow();
	GetWindowRect(hDeskWnd, (LPRECT)&deskrc);
	GetWindowRect(hWnd, (LPRECT)&rc);
	x = (deskrc.right - (rc.right - rc.left)) / Xsplit;
	y = (deskrc.bottom - (rc.bottom - rc.top)) / Ysplit;
	SetWindowPos(hWnd, HWND_TOP, x, y, (rc.right - rc.left), (rc.bottom - rc.top), SWP_SHOWWINDOW);
	return 0;
}


LRESULT CALLBACK WndProc(HWND hwnd2, UINT msg, WPARAM wp, LPARAM lp) {
	HDC hdc;
	switch (msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_CREATE:
		return 0;
	case WM_KEYDOWN:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd2, msg, wp, lp);
}



HWND MakeEditWindow(HINSTANCE hInstance,LPCWSTR type) {
	HWND hwnd;

	WNDCLASS winc;

	winc.style = CS_HREDRAW | CS_VREDRAW;
	winc.lpfnWndProc = WndProc;
	winc.cbClsExtra = winc.cbWndExtra = 0;
	winc.hInstance = hInstance;
	winc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	winc.hCursor = LoadCursor(NULL, IDC_ARROW);
	winc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	winc.lpszMenuName = NULL;
	winc.lpszClassName = type;// TEXT("KITTY");

	if (!RegisterClass(&winc)) return NULL;

	hwnd = CreateWindow(
		type, TEXT("Flexible Keyboard"),
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		100, 100, 400, 300, NULL, NULL,
		hInstance, NULL
	);

	if (hwnd == NULL) return NULL;
	SetWinCenter(hwnd,2,2,1);
	return hwnd;
}

//###################################################3



int APIENTRY WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	HWND hwnd;
	//window作成
	/*
	MSG windowMsg;
	hwnd = MakeEditWindow(hInstance,_T("KITTY"));
	while (GetMessage(&windowMsg, NULL, 0, 0)) {
		DispatchMessage(&windowMsg);
	}
	*/
	char COMName[64] = "COM5";
	TCHAR TCHAR_COMName[64];
	size_t ret_com;
	setlocale(LC_ALL, "ja_JP");
	mbstowcs_s(&ret_com, TCHAR_COMName, 64, COMName, _TRUNCATE);
	MessageBox(NULL, _T("Bluetooth接続準備を開始します。"), _T("Flexible Keyboard"), MB_OK);
	//接続処理
	CSerialPortProcessor* processor = new CSerialPortProcessor();
	DCB portConfig;
	portConfig.BaudRate = 9600;
	// 処理開始
	processor->Start(TCHAR_COMName, &portConfig);
	BYTE buffer[256] = { 0 };
	DWORD readBytes;
	//DestroyWindow();
	MessageBox(NULL, _T("Bluetooth接続可能です"), _T("Flexible Keyboard"), MB_OK);

	while (!CloseFlag) {
		readBytes = processor->recvRead(buffer);
		if (readBytes > 0) {
			char output = buffer[0];
			ChangeStr(&output);
			SendKey(output);
		}
	}
	processor->End();
}


