#pragma once
#include <windows.h>
#include <tchar.h>

class Serial
{
public:
	Serial(void);
	~Serial(void);
	int init(const TCHAR* com,int baud);
	int receive();
	int getc();
	int send();
	int send(char* buf);
	int resetRecvBuffer();
	void startReceiver();
	void stopReceiver();
	void runningThread();

private:
	bool m_isReceiving;
	HANDLE m_hCom; // COM�|�[�g�̃n���h��
	//CWinThread *m_receiveThread; // RS232C��M�X���b�h
	char send_buffer[256];
	char recv_buffer[256];

	int recv_buffer_num;
	int waiting;
};	
