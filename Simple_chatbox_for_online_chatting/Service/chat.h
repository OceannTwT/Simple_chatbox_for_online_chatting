#pragma once
#include <string>
using namespace std;
#define _fail 0
#define _register 1
#define _login 2
#define _sendMessage 3
#define _clientAdd 4
#define _clientExit 5
#define _privateMessage 6
#define _privateFileLen 7
#define _privateFileReply 8
#define MAXSENDNUM 1999

class CChatRoomServerDlg;
struct MyMsgStruct {
	int type;					//��Ϣ���ͣ��ݴ��жϱ�����Ϣִ�й���
	int clientnum;					//�ͻ��˵�ǰ���߳�Ա
	int myfilesize;
	CHAR myClientName[20];			//�������˻���
	CHAR myClientPwd[20];		//����������
	CHAR Message[2000];			//���͵���Ϣ
};
class ClientStruct {
public:
	CHAR myClientName[20];
	CString strIp;
	SOCKET  Socket;
	HANDLE  hThread;
	CChatRoomServerDlg* pMainWind;
	ClientStruct() {
		memset(myClientName, 0, sizeof(myClientName));
		Socket = INVALID_SOCKET;
		hThread = NULL;
		pMainWind = NULL;
	}
};
DWORD WINAPI ListenThreadFunc(LPVOID pParam);
BOOL SOCKET_Select(SOCKET hSocket, int nTimeOut = 100, BOOL bRead = FALSE);
DWORD WINAPI ListenThreadFunc(LPVOID pParam);
DWORD WINAPI ClientThreadCre(LPVOID pParam);