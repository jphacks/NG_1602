#pragma once

class CSerialPortProcessor
{
public:
	CSerialPortProcessor(void);
	~CSerialPortProcessor(void);

	// �w��|�[�g���I�[�v������M�J�n 
	void Start(LPCTSTR comPortName, DCB* portConfig);

	int deletebuffer();
	void CSerialPortProcessor::DCBInit(DCB* portConfig);
	// COM �|�[�g��������I�� 
	void End();

	DWORD WINAPI  CSerialPortProcessor::recvRead(LPVOID buffer);
	

private:
	static DWORD WINAPI ThreadFunc(LPVOID lpParametr);
	DWORD WINAPI ReceiveData();

	HANDLE myHComPort;

	HANDLE myHThread;    // �X���b�h�p�n���h�� 
	DWORD  myThreadId;   // �X���b�h ID 

	HANDLE myMutex;      // myReceivedData,myReceivedDataLength ��r�����邽�߂� Mutex 

	BYTE* myReceivedData;  // ��M�p�����f�[�^ �o�b�t�@ 
	DWORD myReceivedDataLength;  // myReceivedData �Ɋi�[����Ă����M�f�[�^�̃o�C�g�� 
};


