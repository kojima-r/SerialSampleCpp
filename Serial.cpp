
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
	//�V���A���|�[�g�����D��M����~����D
	CloseHandle( m_hCom );
	
}
int Serial::init(const TCHAR* com,int baud){
	// ��M�X���b�h���I������
	/*
	m_isReceiving = false;
	
	if (m_receiveThread != NULL) {
		WaitForSingleObject(m_receiveThread, 4000);
		m_receiveThread = NULL;
	}
	*/
	// �ȑO�Ƀ|�[�g���I�[�v�����Ă�����N���[�Y����
	if(m_hCom != NULL) {
		CloseHandle(m_hCom);
	}
	// �V���A���|�[�g�̃I�[�v��
    m_hCom = CreateFile(com, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if(m_hCom == INVALID_HANDLE_VALUE){
		return false;
	}

	// �V���A���|�[�g�̐ݒ�
	DCB dcb;
	GetCommState(m_hCom, &dcb);	// DCB ���擾
	dcb.BaudRate = baud;		// baudrate
	dcb.ByteSize = 8;			// �o�C�g�T�C�Y
	dcb.Parity = NOPARITY;		// �p���e�B(�g�p���Ȃ�,���ɂ�EVENPARITY,ODDPARITY)
	dcb.fParity = FALSE;		// �p���e�B���g�p���邩
	dcb.StopBits = ONESTOPBIT;	// �X�g�b�v�r�b�g
	dcb.fOutxCtsFlow = FALSE;	// ���M���ɁACTS ���Ď����邩�ǂ�����
	dcb.fOutxDsrFlow = FALSE;	// ���M���ɁADSR ���Ď����邩�ǂ�����
	dcb.fDsrSensitivity = FALSE;// DSR ��OFF�̊Ԃ͎�M�f�[�^�𖳎����邩
	SetCommState(m_hCom, &dcb);	// DCB ��ݒ�

	// ��M�X���b�h�J�n
	m_isReceiving = true;
	//�^�C���A�E�g
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