#pragma once
#define _fail 0
#define _register 1
#define _login 2
#define _sendMessage 3
#define _clientAdd 4
#define _clientExit 5
#define _privateMessage 6
#define _privateFileLen 7
#define _privateFileReply 8
#define _privateFile 9
#define MAXSENDNUM 1999
 
#define WM_MY_MESSAGE (WM_USER+200) 
#define TESTMESSAGE (WM_USER+201) 
#define ChangeColor (WM_USER+202)
extern int ifcon;
class ChatRoom_KBChatRoomDlg;
class ChatRoom_KBOTOChatDlg;
struct MyMsgStruct {
	int type;
	int clientnum;
	int myfilesize;
	CHAR myClientName[20];
	CHAR myClientPwd[20];
	CHAR Message[2000];

};


class ClientStruct {
public:
	CString strIp;
	SOCKET  Socket;
	HANDLE  hThread;
	ChatRoom_KBChatRoomDlg* pMainWind;
	ClientStruct() {
		Socket = INVALID_SOCKET;
		hThread = NULL;
		pMainWind = NULL;
	}
};

//DWORD WINAPI ListenThreadFunc(LPVOID pParam);
UINT ClientThreadCon(LPVOID pParam);
//DWORD WINAPI ClientThreadCon(LPVOID pParam);
BOOL SOCKET_Select(SOCKET hSocket, int nTimeOut = 100, BOOL bRead = FALSE);
BOOL ClientRegister(MyMsgStruct* mymsgs);
BOOL ClientLogin(MyMsgStruct* mymsgs, ChatRoom_KBChatRoomDlg* ChatRoom);
//DWORD WINAPI OTOClientThreadCon(LPVOID pParam);
//DWORD WINAPI ListenThreadFunc(LPVOID pParam);
//DWORD WINAPI ClientThreadCre(LPVOID pParam);