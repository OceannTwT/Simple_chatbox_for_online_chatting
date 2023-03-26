 
// ChatRoom_ServerDlg.h: 头文件
//
#include<list>
#include"chat.h"
using namespace std;
#pragma once



// CChatRoomServerDlg 对话框
class CChatRoomServerDlg : public CDialogEx
{
// 构造
public:
	CChatRoomServerDlg(CWnd* pParent = nullptr);	// 标准构造函数
	SOCKET m_ListenSocket;
	HANDLE m_hListenThread;
	list<ClientStruct> m_ClientList;

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CHATROOM_SERVER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	
public:
	afx_msg void OnBnClickedOpenserver();
	void ShowMessage(CString strMsg);
	void RemoveClientFromList(ClientStruct in_Item);
	void SendClientMsg(MyMsgStruct* sendMsg, ClientStruct* pSend, int ifms);
	void SendClientList(MyMsgStruct* sendMsg, ClientStruct* pSend);
	void SetClientList(ClientStruct* pSet, char myClientName[20]);
	void SendClientMsgOTO(MyMsgStruct* sendMsg, ClientStruct* pSend);
	CEdit o_MessageWind;
	afx_msg void OnLbnSelchangeList2();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
};


 
