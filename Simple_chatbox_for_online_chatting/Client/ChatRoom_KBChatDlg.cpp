
// ChatRoom_KBChatDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h" 
#include "ChatRoom_KBChat.h"
#include "ChatRoom_KBChatDlg.h"
#include "afxdialogex.h"
#include "iostream"
#include "fstream"
#include <string>
#include <cstring>
#include "chat.h"
using namespace std;
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CChatRoomKBChatDlg 对话框



CChatRoomKBChatDlg::CChatRoomKBChatDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CHATROOM_KBCHAT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	ChatRoom = NULL;
}

CChatRoomKBChatDlg::~CChatRoomKBChatDlg(){
		if (ChatRoom != NULL) {
		delete ChatRoom;
		ChatRoom = NULL;
	}
}


void CChatRoomKBChatDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_Account, account);
	DDX_Control(pDX, IDC_Password, Password);
}

BEGIN_MESSAGE_MAP(CChatRoomKBChatDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_register, &CChatRoomKBChatDlg::OnBnClickedregister)
	ON_BN_CLICKED(IDC_login, &CChatRoomKBChatDlg::OnBnClickedlogin)
//	ON_MESSAGE(WM_USER + 201, &CChatRoomKBChatDlg::OnNewOODlg)
//ON_MESSAGE(TestMessage, &CChatRoomKBChatDlg::OnTestmessage)
	ON_MESSAGE(TESTMESSAGE, &CChatRoomKBChatDlg::OnTestmessage)
	ON_MESSAGE(ChangeColor, &CChatRoomKBChatDlg::OnChangecolor)
END_MESSAGE_MAP()


// CChatRoomKBChatDlg 消息处理程序

BOOL CChatRoomKBChatDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CChatRoomKBChatDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CChatRoomKBChatDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CChatRoomKBChatDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}




void CChatRoomKBChatDlg::OnBnClickedregister()
{
	CString myClientName,myClientPwd;
	account.GetWindowText(myClientName);
	Password.GetWindowText(myClientPwd);
	string UserN = CT2A(myClientName.GetString());
	string UserP = CT2A(myClientPwd.GetString());
	string Name, Password;
	if (UserN == "" || UserP == "") {
		AfxMessageBox(_T("用户名和密码格式不对"));
		return;
	}
	MyMsgStruct mymsgs;
	//char temp[2048];
	memset(&mymsgs, 0, sizeof(MyMsgStruct));
	mymsgs.type = _register;
	mymsgs.clientnum = 0;
	strncpy_s(mymsgs.myClientName, UserN.c_str(), sizeof(mymsgs.myClientName));
	strncpy_s(mymsgs.myClientPwd, UserP.c_str(), sizeof(mymsgs.myClientPwd));
	strncpy_s(mymsgs.Message, "", sizeof(mymsgs.Message));
	//memcpy(temp, &mymsgs, sizeof(MyMsgStruct));
	ClientRegister(&mymsgs);
	return;
}


void CChatRoomKBChatDlg::OnBnClickedlogin()
{	
	ifcon = 0;
	CString myClientName, myClientPwd;
	account.GetWindowText(myClientName);
	Password.GetWindowText(myClientPwd);
	string UserN = CT2A(myClientName.GetBuffer(0));
	myClientName.ReleaseBuffer();
	string UserP = CT2A(myClientPwd.GetBuffer(0));
	myClientPwd.ReleaseBuffer();
	string Name, Password;
	if (UserN == "" || UserP == "") {
		AfxMessageBox(_T("用户名和密码格式不对"));
		return;
	}
	MyMsgStruct mymsgs;
	memset(&mymsgs, 0, sizeof(MyMsgStruct));
	mymsgs.type = _login;
	mymsgs.clientnum = 0;
	strncpy_s(mymsgs.myClientName, UserN.c_str(), sizeof(mymsgs.myClientName));
	strncpy_s(mymsgs.myClientPwd, UserP.c_str(), sizeof(mymsgs.myClientPwd));
	strncpy_s(mymsgs.Message, "", sizeof(mymsgs.Message));
	if (ChatRoom == NULL) {//判定对话框是否有所指向
		ChatRoom = new ChatRoom_KBChatRoomDlg();//指向一个非模对话框示例
		ChatRoom->Create(IDD_CHATROOM_KBCHATROOM_DIALOG, this);//创建
	}
	if (ClientLogin(&mymsgs,ChatRoom)) {
		ChatRoom->m_Name = myClientName;
		//ChatRoom->AddListMember(myClientName);
		//ChatRoom->m_hConnectThread = CreateThread(NULL, 0, ClientThreadCon, ChatRoom, 0, NULL);	
		AfxBeginThread(ClientThreadCon, (VOID*)ChatRoom, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
		Sleep(500);
		if (ifcon != 0) {
			this->ShowWindow(SW_HIDE);
			ChatRoom->ShowWindow(SW_SHOW);//显示
		}
	}
	return;
}



afx_msg LRESULT CChatRoomKBChatDlg::OnTestmessage(WPARAM wParam, LPARAM lParam)
{
	ChatRoom_KBOTOChatDlg* pDlg = (ChatRoom_KBOTOChatDlg*)wParam;
	pDlg->Create(IDD_CHATROOM_OTO_DIALOG);
	//pDlg->ShowWindow(SW_SHOW);
	return 0;
}


afx_msg LRESULT CChatRoomKBChatDlg::OnChangecolor(WPARAM wParam, LPARAM lParam)
{
	COLORREF itemColor = (COLORREF) wParam;
	int n = (int)lParam;
	ChatRoom->L_MemberList.SetColour(n,itemColor);
	return 0;
}
