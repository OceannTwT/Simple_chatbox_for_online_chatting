// ChatRoom_KBOTOChatDlg.cpp: 实现文件
//

#include "pch.h"
#include "ChatRoom_KBChat.h"
#include "ChatRoom_KBOTOChatDlg.h"
#include "afxdialogex.h"
#include "chat.h"
#include<sstream>
#include "iostream"
using namespace std;

#include "CColorListBox.h"
// ChatRoom_KBOTOChatDlg 对话框
const int max_read_len_ = 10000;


IMPLEMENT_DYNAMIC(ChatRoom_KBOTOChatDlg, CDialogEx)

ChatRoom_KBOTOChatDlg::ChatRoom_KBOTOChatDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CHATROOM_OTO_DIALOG, pParent)
{
	
	m_OTOConnectSocket = INVALID_SOCKET;
}

ChatRoom_KBOTOChatDlg::~ChatRoom_KBOTOChatDlg()
{
}

void ChatRoom_KBOTOChatDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT2, o_OTOMessageWind);
}

void ChatRoom_KBOTOChatDlg::ShowMessage(CString strMsg)
{
	o_OTOMessageWind.SetSel(-1, -1);
	o_OTOMessageWind.ReplaceSel(strMsg + _T("\r\n"));
	return;
}


BEGIN_MESSAGE_MAP(ChatRoom_KBOTOChatDlg, CDialogEx)

	ON_BN_CLICKED(IDC_BUTTON1, &ChatRoom_KBOTOChatDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &ChatRoom_KBOTOChatDlg::OnBnClickedButton2)

	ON_WM_DROPFILES()
	ON_BN_CLICKED(IDC_BUTTON3, &ChatRoom_KBOTOChatDlg::OnBnClickedButton3)
END_MESSAGE_MAP()


// ChatRoom_KBOTOChatDlg 消息处理程序





void ChatRoom_KBOTOChatDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	MyMsgStruct smsg;
	CString strMsg;
	CString ti;
	CTime tm; tm = CTime::GetCurrentTime();
	ti = tm.Format("%Y/%m/%d %X");
	GetDlgItemText(IDC_EDIT1, strMsg);
	string text = CT2A(strMsg.GetBuffer(0));
	strMsg.ReleaseBuffer();
	if (text == "") {
		return;
	}
	strMsg = ti + _T("  ") + Mname + _T(":") + strMsg;
	string msg = CT2A(strMsg.GetBuffer(0));
	strMsg.ReleaseBuffer();
	//检查是否有违规内容
	if (msg.find("独立台湾") != string::npos || msg.find("淫秽视频") != string::npos || msg.find("敏感词") != string::npos) {
		//不存在
		AfxMessageBox(_T("存在敏感信息，已屏蔽！"));
		SetDlgItemText(IDC_EDIT1, _T(""));
		return;
	}else {
		strMsg.ReleaseBuffer();
		string tname = CT2A(Tname.GetBuffer(0));
		Tname.ReleaseBuffer();
		smsg.type = _privateMessage;
		smsg.clientnum = 0;
		strncpy_s(smsg.myClientName, tname.c_str(), sizeof(smsg.myClientName));
		strncpy_s(smsg.myClientPwd, "", sizeof(smsg.myClientPwd));
		strncpy_s(smsg.Message, msg.c_str(), sizeof(smsg.Message));
		int iSend = send(m_OTOConnectSocket, (char*)&smsg, sizeof(MyMsgStruct), 0);
		//strMsg.ReleaseBuffer();
		SetDlgItemText(IDC_EDIT1, _T(""));
		ShowMessage(strMsg);
		return;
	}
}

////选择文件
void ChatRoom_KBOTOChatDlg::OnBnClickedButton2()
{
	m_FileName = "";
	CFileDialog fDlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, this);////选择文件
	if (IDCANCEL == fDlg.DoModal())
	{
		return;
	}
	if (fDlg.DoModal() == IDOK)
	{
		m_FileName = fDlg.GetPathName();////获取m_FileName
		SetDlgItemText(IDC_EDIT3, m_FileName);
	}

}


void ChatRoom_KBOTOChatDlg::OnLvnItemchangedList1(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
}


void ChatRoom_KBOTOChatDlg::OnBnClickedButton3()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_Send == FALSE)////如果当前没有正在发送别的文件
	{
		if (m_FileName=="")
		{
			AfxMessageBox(_T("请选择一个文件！"));
			return;
		}
		//m_SendFile.Abort();////与对象m_SendFile相关联的文件,并使文件不可读写
		//m_SendFile.Open(m_FileName, CFile::modeRead);
		
		//m_SendFileLen = m_SendFile.GetLength();////获取m_SendFileLen

		ifstream is;
		is.open(m_FileName);
		stringstream buffer;
		buffer << is.rdbuf();
		contexts = buffer.str();
		m_SendFileLen = contexts.size();
		

		char* fileinfo = (char*)LocalAlloc(LMEM_FIXED, MAX_PATH + 4);
		memset(fileinfo, 0, MAX_PATH + 4);
		//memcpy(fileinfo, m_FileName.GetBuffer(0), m_FileName.GetLength());////获取fileinfo：1是m_FileName

		CString filesize;
		filesize.Format(_T("%i"), m_SendFileLen);
		//memcpy(fileinfo + m_FileName.GetLength() + 1, filesize.GetBuffer(0), filesize.GetLength());////获取fileinfo：2是filesize
		memcpy(fileinfo, filesize.GetBuffer(0), filesize.GetLength());////获取fileinfo：2是filesize

		SetTimer(1, 2000, NULL);
		//发送文件信息
		m_SendNum = 0;
		//m_pSendSock->Send(fileinfo, MAX_PATH + 4);////发送fileinfo

		MyMsgStruct smsg;
		smsg.type = _privateFileLen;
		smsg.clientnum = 0; 

		string tname = CT2A(Tname.GetBuffer(0));
		Tname.ReleaseBuffer();
		strncpy_s(smsg.myClientName, tname.c_str(), sizeof(smsg.myClientName));
		strncpy_s(smsg.myClientPwd, "", sizeof(smsg.myClientPwd));

		/*正确
		string msg = CT2A(filesize.GetBuffer(0));

		strncpy_s(smsg.Message, msg.c_str(), sizeof(smsg.Message));
		
		filesize.ReleaseBuffer();*/

		//修改：filesize
		smsg.myfilesize = m_SendFileLen;
		string msg = CT2A(m_FileName.GetBuffer(0));
		strncpy_s(smsg.Message, msg.c_str(), sizeof(smsg.Message));
		m_FileName.ReleaseBuffer();
		send(m_OTOConnectSocket, (char*)&smsg, sizeof(MyMsgStruct), 0);//发文件名和大小

		m_SendLen = 0;
		LocalFree(fileinfo);
	}
	else
	{
		AfxMessageBox(_T("尚有其他文件正在发送"));
	}
	
}
