#pragma once

class CSerialPortProcessor
{
public:
	CSerialPortProcessor(void);
	~CSerialPortProcessor(void);

	// 指定ポートをオープンし受信開始 
	void Start(LPCTSTR comPortName, DCB* portConfig);

	int deletebuffer();
	void CSerialPortProcessor::DCBInit(DCB* portConfig);
	// COM ポートを閉じ処理終了 
	void End();

	DWORD WINAPI  CSerialPortProcessor::recvRead(LPVOID buffer);
	

private:
	static DWORD WINAPI ThreadFunc(LPVOID lpParametr);
	DWORD WINAPI ReceiveData();

	HANDLE myHComPort;

	HANDLE myHThread;    // スレッド用ハンドル 
	DWORD  myThreadId;   // スレッド ID 

	HANDLE myMutex;      // myReceivedData,myReceivedDataLength を排他するための Mutex 

	BYTE* myReceivedData;  // 受信用内部データ バッファ 
	DWORD myReceivedDataLength;  // myReceivedData に格納されている受信データのバイト数 
};


