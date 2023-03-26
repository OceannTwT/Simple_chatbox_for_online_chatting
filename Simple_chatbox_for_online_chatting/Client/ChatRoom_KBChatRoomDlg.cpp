// ChatRoom_KBChatRoomDlg.cpp: 实现文件
//

#include "pch.h"
#include "ChatRoom_KBChat.h"
#include "ChatRoom_KBChatRoomDlg.h"
#include "afxdialogex.h"
#include "chat.h"
#include <fstream>
#include<unordered_set> 
#include<regex>
#include "iostream"
using namespace std;
#define MAX_BUF_SIZE 2048

// ChatRoom_KBChatRoomDlg 对话框

IMPLEMENT_DYNAMIC(ChatRoom_KBChatRoomDlg, CDialogEx)

ChatRoom_KBChatRoomDlg::ChatRoom_KBChatRoomDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CHATROOM_KBCHATROOM_DIALOG, pParent)
{
	m_ConnectSocket = INVALID_SOCKET;
	m_hConnectThread = NULL;

}

ChatRoom_KBChatRoomDlg::~ChatRoom_KBChatRoomDlg()
{
}

void ChatRoom_KBChatRoomDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, o_MessageWind);
	DDX_Control(pDX, IDC_LIST1, L_MemberList);
	return;
}

void ChatRoom_KBChatRoomDlg::ShowMessage(CString strMsg)
{
	o_MessageWind.SetSel(-1, -1);
	o_MessageWind.ReplaceSel(strMsg + _T("\r\n"));
	return;
}

void ChatRoom_KBChatRoomDlg::OnBnClickedBsendmsg()
{
	MyMsgStruct smsg;
	CString strMsg;
	CString ti;
	CTime tm; tm = CTime::GetCurrentTime();
	const int limit_range_ = 6;
	ti = tm.Format("%Y/%m/%d %X");
	GetDlgItemText(IDC_EDIT2, strMsg);
	string text = CT2A(strMsg.GetBuffer(0));
	strMsg.ReleaseBuffer();
	if (text == "") {
		return;
	}
	strMsg = ti + _T("  ") + m_Name + _T(":") + strMsg;
	string msg = CT2A(strMsg.GetBuffer(0)), ban_list;
	ofstream o_file;
	o_file.open("ban.txt", ios::app);
	o_file.close();
	ifstream is_file;
	is_file.open("ban.txt");
	unordered_set<string> pattenString;
	while (is_file >> ban_list)
	{
		string tmp = ban_list;
		pattenString.insert(ban_list);
	}
	vector<regex> regPatten;
	for (char i = 0 + '0'; i <= limit_range_ + '0'; i++)
	{
		for (auto str : pattenString)
		{
			string Pat = str.substr(0, 4) + "(.{" + i + "})" + str.substr(4, str.size() - 4);
			regex nextPatten(Pat.c_str());
			regPatten.push_back(nextPatten);
		}
	}
	strMsg.ReleaseBuffer();
	for (auto reg : regPatten)
	{
		if (regex_search(msg, reg))
		{
			AfxMessageBox(_T("存在敏感信息，已屏蔽！"));
			SetDlgItemText(IDC_EDIT2, _T(""));
			return;
		}
	}
	smsg.type = 3;
	smsg.clientnum = 0;
	strncpy_s(smsg.myClientName, "", sizeof(smsg.myClientName));
	strncpy_s(smsg.myClientPwd, "", sizeof(smsg.myClientPwd));
	strncpy_s(smsg.Message, msg.c_str(), sizeof(smsg.Message));
	int iSend = send(m_ConnectSocket, (char*)&smsg, sizeof(MyMsgStruct), 0);
	//strMsg.ReleaseBuffer();
	SetDlgItemText(IDC_EDIT2, _T(""));
	return;
}

void ChatRoom_KBChatRoomDlg::AddListMember(CString uname, COLORREF itemColor) {
	//CString uname;
	L_MemberList.InsertString(0, uname);
}

void ChatRoom_KBChatRoomDlg::OnOTOChat()
{
	CString text;
	int i = L_MemberList.GetCurSel();
	int flag = 0;
	L_MemberList.GetText(i, text); // 写错了，不要&
	//L_MemberList.DeleteString(i);
	//AddListMember(text, RGB(0, 0, 0));
	//int row = pChatRoom->L_MemberList.FindStringExact(0, SName);
	L_MemberList.SetColour(i, RGB(0, 0, 0));
	if (text != this->m_Name) {
		list<ChatRoom_KBOTOChatDlg*>::iterator iter;  //定义迭代器
		for (iter = this->m_OTOChatList.begin(); iter != this->m_OTOChatList.end(); ) {
			if ((*iter)->Tname == text) {
				(*iter)->ShowWindow(SW_SHOW);
				(*iter)->SetWindowText(text);
				flag = 1;
				break;
			}
			iter++;
		}
		if (!flag) {
			ChatRoom_KBOTOChatDlg* pDlg = new ChatRoom_KBOTOChatDlg();
			pDlg->Create(IDD_CHATROOM_OTO_DIALOG, this);
			pDlg->ShowWindow(SW_SHOW);
			pDlg->SetWindowText(text);
			pDlg->Mname = this->m_Name;
			pDlg->Tname = text;
			pDlg->m_OTOConnectSocket = this->m_ConnectSocket;
			this->m_OTOChatList.push_back(pDlg);
		}
	}
}

BEGIN_MESSAGE_MAP(ChatRoom_KBChatRoomDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BSendMsg, &ChatRoom_KBChatRoomDlg::OnBnClickedBsendmsg)
	ON_LBN_DBLCLK(IDC_LIST1, &ChatRoom_KBChatRoomDlg::OnOTOChat)
	ON_WM_CLOSE()
	//	ON_MESSAGE(OnNewOTODlg, &ChatRoom_KBChatRoomDlg::OnOnnewotodlg)
	ON_MESSAGE(WM_USER + 200, &ChatRoom_KBChatRoomDlg::OnNewOTODlg)
END_MESSAGE_MAP()


// ChatRoom_KBChatRoomDlg 消息处理程序



void ChatRoom_KBChatRoomDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialogEx::OnClose();
	GetParent()->DestroyWindow();
}



afx_msg LRESULT ChatRoom_KBChatRoomDlg::OnNewOTODlg(WPARAM wParam, LPARAM lParam)
{
	ChatRoom_KBOTOChatDlg* pDlg = (ChatRoom_KBOTOChatDlg*)wParam;
	pDlg->Create(IDD_CHATROOM_OTO_DIALOG);
	pDlg->ShowWindow(SW_SHOW);
	return 0;
}
