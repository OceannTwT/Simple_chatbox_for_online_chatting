#pragma once
#include<fstream>
// ChatRoom_KBOTOChatDlg 对话框

class ChatRoom_KBOTOChatDlg : public CDialogEx
{
	DECLARE_DYNAMIC(ChatRoom_KBOTOChatDlg)
	 
public:
	ChatRoom_KBOTOChatDlg(CWnd* pParent = nullptr);   // 标准构造函数

	CString m_FileName;    //传送的文件名
	BOOL           m_Send;        //是否开始发送数据
///	CFile          m_SendFile;    //发送的文件
	DWORD          m_SendFileLen; //发送文件的长度
	DWORD          m_SendLen;     //记录已经发送的文件长度
	DWORD          m_SendNum;     //记录当前发送的次数

	CFile          m_File;        //保存接收的文件
	BOOL           m_IsReveived;  //是否接收数据
	DWORD          m_FileWholeLen;    //接收的文件的总大小
	DWORD          m_FileLen;    //已经接收的文件大小

	virtual ~ChatRoom_KBOTOChatDlg();
	std::string contexts; 
	std::ofstream of;

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CHATROOM_OTO_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CString Tname;
	CString Mname;
	//CString Mname;
	//HANDLE OTOconnect;
	CEdit o_OTOMessageWind;
	SOCKET m_OTOConnectSocket;
	void ShowMessage(CString strMsg);
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();

	//CListCtrl m_List;////上传的文件列表
	afx_msg void OnLvnItemchangedList1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedButton3();

};
