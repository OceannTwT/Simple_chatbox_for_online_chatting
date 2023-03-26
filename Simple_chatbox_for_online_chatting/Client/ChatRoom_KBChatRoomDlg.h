#pragma once
#include<list>
#include "ChatRoom_KBOTOChatDlg.h"
#include "CColorListBox.h"
using namespace std;
// ChatRoom_KBChatRoomDlg 对话框

class ChatRoom_KBChatRoomDlg : public CDialogEx
{ 
	DECLARE_DYNAMIC(ChatRoom_KBChatRoomDlg)

public:
	ChatRoom_KBChatRoomDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~ChatRoom_KBChatRoomDlg();
	SOCKET m_ConnectSocket;
	HANDLE m_hConnectThread;
	CString m_Name;
	list<ChatRoom_KBOTOChatDlg*> m_OTOChatList;
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CHATROOM_KBCHATROOM_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	DECLARE_MESSAGE_MAP()
public:
	CEdit o_MessageWind;
	CColorListBox L_MemberList;
	void ShowMessage(CString strMsg);
	afx_msg void OnBnClickedBsendmsg();
	void AddListMember(CString uname, COLORREF itemColor = RGB(0,0,0));
	void OnOTOChat();
	afx_msg void OnClose();
	
protected:
//	afx_msg LRESULT OnOnnewotodlg(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnNewOTODlg(WPARAM wParam, LPARAM lParam);
};
