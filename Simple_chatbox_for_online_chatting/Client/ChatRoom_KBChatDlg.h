
// ChatRoom_KBChatDlg.h: 头文件
//

#pragma once
#include "ChatRoom_KBChatRoomDlg.h"
#include "ChatRoom_KBOTOChatDlg.h"
// CChatRoomKBChatDlg 对话框
class CChatRoomKBChatDlg : public CDialogEx
{ 
// 构造
public:
	CChatRoomKBChatDlg(CWnd* pParent = nullptr);	// 标准构造函数

	~CChatRoomKBChatDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CHATROOM_KBCHAT_DIALOG };
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
	CEdit account;
	CEdit Password;
	ChatRoom_KBChatRoomDlg* ChatRoom;

	afx_msg void OnBnClickedregister();
	afx_msg void OnBnClickedlogin();
protected:
//	afx_msg LRESULT OnNewOODlg(WPARAM wParam, LPARAM lParam);
//	afx_msg LRESULT OnTestmessage(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnTestmessage(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnChangecolor(WPARAM wParam, LPARAM lParam);
};
