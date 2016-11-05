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

#pragma comment(lib,"Bthprops")
using namespace std;

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
char ChangeStr(char str) {

		if ('0' <= str && '9' >= str) {
			str = str + 48;
		}
		else {
			str = str;
		}
	return str;
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

int SetWinCenter(HWND hWnd)
{
	HWND hDeskWnd;
	RECT deskrc, rc;
	int x, y;

	hDeskWnd = GetDesktopWindow();
	GetWindowRect(hDeskWnd, (LPRECT)&deskrc);
	GetWindowRect(hWnd, (LPRECT)&rc);
	x = (deskrc.right - (rc.right - rc.left)) / 2;
	y = (deskrc.bottom - (rc.bottom - rc.top)) / 2;
	SetWindowPos(hWnd, HWND_TOP, x, y, (rc.right - rc.left), (rc.bottom - rc.top), SWP_SHOWWINDOW);
	return 0;
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
	switch (msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_CREATE:
		CreateWindow(
			TEXT("EDIT"), TEXT(""),
			WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
			50, 50, 200, 50, hwnd, (HMENU)1,
			((LPCREATESTRUCT)(lp))->hInstance, NULL
		);
		SetWinCenter(hwnd);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wp, lp);
}



int MakeEditWindow(HINSTANCE hInstance) {
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
	winc.lpszClassName = TEXT("KITTY");

	if (!RegisterClass(&winc)) return -1;

	hwnd = CreateWindow(
		TEXT("KITTY"), TEXT("Flexible Keyboard"),
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		100, 100, 400, 300, NULL, NULL,
		hInstance, NULL
	);

	if (hwnd == NULL) return -1;
	SetWinCenter(hwnd);
}

//###################################################3



int APIENTRY WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	//window作成
	MSG windowMsg;
	MakeEditWindow(hInstance);
	while (GetMessage(&windowMsg, NULL, 0, 0)) {
		TranslateMessage(&windowMsg);
		DispatchMessage(&windowMsg);
	}
	
	char COMName[64] = "COM5";
	TCHAR TCHAR_COMName[64];
	size_t ret_com;
	setlocale(LC_ALL, "ja_JP");
	mbstowcs_s(&ret_com,TCHAR_COMName, 64,COMName,_TRUNCATE);
	
	//接続処理
	CSerialPortProcessor* processor = new CSerialPortProcessor();
	DCB portConfig;
	portConfig.BaudRate = 9600;
	// 処理開始
	processor->Start(TCHAR_COMName, &portConfig);
	BYTE buffer[256] = { 0 };
	DWORD readBytes;

	readBytes = processor->recvRead(buffer);
	while (1) {
		readBytes = processor->recvRead(buffer);
		//TCHAR msg[30];
		//wsprintf(msg, TEXT("%lu"), readBytes);
		//MessageBox(NULL, msg, _T("BluetoothDevice"), MB_OK);
		if (readBytes > 0) {
			char output = buffer[0];
			output = ChangeStr(output);
			SendKey(output);
		}
	}
	processor->End();
}

