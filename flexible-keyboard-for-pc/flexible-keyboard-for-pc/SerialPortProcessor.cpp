#include <windows.h>
#include "SerialPortProcessor.h"
#include <tchar.h>

#define MAXCSerialBYTE 2

CSerialPortProcessor::CSerialPortProcessor(void)
	: myHComPort(NULL), myReceivedData(NULL), myReceivedDataLength(0)
{
	myMutex = CreateMutex(NULL, TRUE, NULL);
}

CSerialPortProcessor::~CSerialPortProcessor(void)
{
	CloseHandle(myMutex);
	if (myReceivedData != NULL) {
		delete myReceivedData;
		myReceivedData = NULL;
	}
	myMutex = NULL;
}

void CSerialPortProcessor::Start(LPCTSTR  comPortName, DCB* portConfig)
{
	// 指定ポートを開く 
	myHComPort = CreateFile(
		(LPCTSTR)comPortName,
		GENERIC_READ | GENERIC_WRITE,
		0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	//comポートのエラー表示
	if (myHComPort == INVALID_HANDLE_VALUE) {
		TCHAR msg[30];
		wsprintf(msg, TEXT("%d"), GetLastError());
		MessageBox(NULL, msg, _T("error"), MB_OK);
	}

	//送受信バッファ初期化
	int Ret;
	Ret = SetupComm(myHComPort, MAXCSerialBYTE, MAXCSerialBYTE);
	if (Ret == FALSE) {
		MessageBox(NULL, _T("SetupComm failed.\n"), _T("BluetoothDevice"), MB_OK);
		End();
		exit(0);
	}

	if ((Ret = deletebuffer()) != 0) {
		exit(0);
	}

	// ポートのボーレート、パリティ等を設定 
	DCBInit(portConfig);
	Ret = SetCommState(myHComPort, portConfig);
	if (Ret == FALSE)  // 失敗した場合
	{
		MessageBox(NULL, _T("SetCommState failed.\n"), _T("BluetoothDevice"), MB_OK);
		End();
		exit(0);
	}
	myHThread = CreateThread(NULL, 0,
		ThreadFunc, (LPVOID)this,
		CREATE_SUSPENDED, &myThreadId);
}
void CSerialPortProcessor::DCBInit(DCB* portConfig) {
	portConfig->DCBlength = sizeof(portConfig);
	portConfig->BaudRate = 9600;
	portConfig->fBinary = TRUE;
	portConfig->ByteSize = 8;
	portConfig->fParity = NOPARITY;
	portConfig->StopBits = ONESTOPBIT;
	portConfig->fOutxCtsFlow = FALSE;
	portConfig->fOutxDsrFlow = FALSE;
	portConfig->fDtrControl = DTR_CONTROL_DISABLE;
	portConfig->fRtsControl = RTS_CONTROL_DISABLE;
	//ソフトウェア制御
	portConfig->fOutX = FALSE;
	portConfig->fInX = FALSE;
	portConfig->fTXContinueOnXoff = TRUE;
	portConfig->XonLim = MAXCSerialBYTE / 2;
	portConfig->XoffLim = MAXCSerialBYTE / 2;
	portConfig->XonChar = 0x11;
	portConfig->XoffChar = 0x13;
	//その他
	portConfig->fNull = TRUE;
	portConfig->fAbortOnError = TRUE;
	portConfig->fErrorChar = FALSE;
	portConfig->ErrorChar = 0x00;
	portConfig->EofChar = 0x03;
	portConfig->EvtChar = 0x02;
}

int CSerialPortProcessor::deletebuffer() {
	int Ret;
	Ret = PurgeComm(myHComPort, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

	if (Ret == FALSE) {
		MessageBox(NULL, _T("PurgeComm failed.\n"), _T("BluetoothDevice"), MB_OK);
		CloseHandle(myHComPort);
		return -1;
	}
	return 0;
}



void CSerialPortProcessor::End()
{
	if (myHComPort != NULL) {
		CloseHandle(myHComPort);
	}
}

DWORD WINAPI CSerialPortProcessor::ThreadFunc(LPVOID lpParameter)
{
	return ((CSerialPortProcessor*)lpParameter)->ReceiveData();
}


DWORD WINAPI  CSerialPortProcessor::recvRead(LPVOID buffer) {
	DWORD toReadBytes = 1;
	DWORD readBytes;
	ReadFile(myHComPort, buffer, toReadBytes, &readBytes, NULL);
	return readBytes;
}


DWORD WINAPI CSerialPortProcessor::ReceiveData()
{
	BYTE buffer[MAXCSerialBYTE];
	DWORD toReadBytes = MAXCSerialBYTE;
	DWORD readBytes;
	while (ReadFile(myHComPort, buffer, toReadBytes, &readBytes, NULL)) {
		if (readBytes > 0) {
			WaitForSingleObject(myMutex, 0);
			// 受信したデータは、myReceivedData に受信済みで取り出されていない 
			// データに続けて保持しておく 
			BYTE* tmpBuf = new BYTE[myReceivedDataLength + readBytes];
			if (myReceivedData != NULL) {
				memcpy(tmpBuf, myReceivedData, myReceivedDataLength);
				delete[] myReceivedData;
			}
			memcpy(tmpBuf, buffer, readBytes);
			myReceivedDataLength += readBytes;
			myReceivedData = tmpBuf;
			ReleaseMutex(myMutex);
		}
	}
	return S_OK;
}










