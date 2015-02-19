
#include "Serial.h"
#include <tchar.h>
#include <boost/thread.hpp>

Serial::Serial(void)
{
	m_hCom = NULL;
	//m_receiveThread = NULL;
	//m_receiveEvent = receiveEvent;
	recv_buffer_num=0;
	memset(recv_buffer,'\0',255);
	
}

Serial::~Serial(void)
{
	//シリアルポートを閉じる．受信も停止する．
	CloseHandle( m_hCom );
	
}
int Serial::init(const TCHAR* com,int baud){
	// 受信スレッドを終了する
	/*
	m_isReceiving = false;
	
	if (m_receiveThread != NULL) {
		WaitForSingleObject(m_receiveThread, 4000);
		m_receiveThread = NULL;
	}
	*/
	// 以前にポートをオープンしていたらクローズする
	if(m_hCom != NULL) {
		CloseHandle(m_hCom);
	}
	// シリアルポートのオープン
    m_hCom = CreateFile(com, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if(m_hCom == INVALID_HANDLE_VALUE){
		return false;
	}

	// シリアルポートの設定
	DCB dcb;
	GetCommState(m_hCom, &dcb);	// DCB を取得
	dcb.BaudRate = baud;		// baudrate
	dcb.ByteSize = 8;			// バイトサイズ
	dcb.Parity = NOPARITY;		// パリティ(使用しない,他にはEVENPARITY,ODDPARITY)
	dcb.fParity = FALSE;		// パリティを使用するか
	dcb.StopBits = ONESTOPBIT;	// ストップビット
	dcb.fOutxCtsFlow = FALSE;	// 送信時に、CTS を監視するかどうかを
	dcb.fOutxDsrFlow = FALSE;	// 送信時に、DSR を監視するかどうかを
	dcb.fDsrSensitivity = FALSE;// DSR がOFFの間は受信データを無視するか
	SetCommState(m_hCom, &dcb);	// DCB を設定

	// 受信スレッド開始
	m_isReceiving = true;
	//タイムアウト
	COMMTIMEOUTS tmo;
	tmo.ReadTotalTimeoutConstant = 5000;
	tmo.ReadTotalTimeoutMultiplier = 0;
	tmo.WriteTotalTimeoutConstant = 5000;
	tmo.WriteTotalTimeoutMultiplier = 0;
	SetCommTimeouts(m_hCom, &tmo);
	
	//m_receiveThread = AfxBeginThread(ReceiveRs232cFunc, this);

	return 1;
}
int Serial::resetRecvBuffer(){
	for(;recv_buffer_num>0;recv_buffer_num--){
		recv_buffer[recv_buffer_num]='\0';
	}
	recv_buffer_num=0;
	recv_buffer[recv_buffer_num]='\0';
	return 1;
}
int Serial::receive(){
	DWORD dwRead;
	if(m_hCom == INVALID_HANDLE_VALUE){
		return 0;
	}
	ReadFile(m_hCom,&recv_buffer[recv_buffer_num],1,&dwRead,NULL);
	recv_buffer_num++;
	return 1;
}
int Serial::getc(){
	DWORD dwRead;
	if(m_hCom == INVALID_HANDLE_VALUE){
		return -1;
	}
	char buf[2];
	ReadFile(m_hCom,buf,1,&dwRead,NULL);
	return buf[0];
}
int Serial::send(){
	DWORD wbyte;
	WriteFile(m_hCom,send_buffer,strlen(send_buffer), &wbyte, NULL);
	return wbyte;
}
int Serial::send(char* buf){
	DWORD wbyte;
	WriteFile(m_hCom,buf,strlen(buf), &wbyte, NULL);
	return wbyte;
}

void Serial::startReceiver(){
	waiting=1;
	boost::thread thr_seri(&Serial::runningThread,this);
	//thr_seri.join();
	return;
}
void Serial::stopReceiver(){
	waiting=0;
	return;
}
void Serial::runningThread(){
	while(waiting==1){
		char c=this->getc();
		printf("%c",c);
	}
	return;
}