
// ChatRoom_ServerDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "ChatRoom_Server.h"
#include "ChatRoom_ServerDlg.h"
#include "afxdialogex.h"
#include "iostream"
#include "fstream"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define MAX_BUF_SIZE 2048
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


// CChatRoomServerDlg 对话框



CChatRoomServerDlg::CChatRoomServerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CHATROOM_SERVER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_ListenSocket = INVALID_SOCKET;
	m_hListenThread = NULL;
}

void CChatRoomServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, o_MessageWind);
}

BEGIN_MESSAGE_MAP(CChatRoomServerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	////单击IDC_OpenServer时，，调用OnBnClickedOpenserver
	ON_BN_CLICKED(IDC_OpenServer, &CChatRoomServerDlg::OnBnClickedOpenserver)
	////单击IDC_LIST2时，调用OnLbnSelchangeList2
	ON_LBN_SELCHANGE(IDC_LIST2, &CChatRoomServerDlg::OnLbnSelchangeList2)
	////单击IDC_BUTTON2时，调用OnBnClickedButton2
	ON_BN_CLICKED(IDC_BUTTON2, &CChatRoomServerDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CChatRoomServerDlg::OnBnClickedButton3)
END_MESSAGE_MAP()


// CChatRoomServerDlg 消息处理程序

BOOL CChatRoomServerDlg::OnInitDialog()
{ 
	CDialogEx::OnInitDialog();

	//// 将“关于...”菜单项添加到系统菜单中。

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

void CChatRoomServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CChatRoomServerDlg::OnPaint()
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
HCURSOR CChatRoomServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CChatRoomServerDlg::ShowMessage(CString strMsg)
{
	o_MessageWind.SetSel(-1, -1);
	o_MessageWind.ReplaceSel(strMsg + _T("\r\n"));
}

void CChatRoomServerDlg::RemoveClientFromList(ClientStruct in_Item)
{
	list<ClientStruct>::iterator iter;  //定义迭代器
	for (iter = m_ClientList.begin(); iter != m_ClientList.end(); ) {
		if (iter->hThread == in_Item.hThread && iter->strIp == in_Item.strIp && iter->Socket == in_Item.Socket) {
			//closesocket(iter->Socket);
			iter = m_ClientList.erase(iter);
		}
		else {
			iter++;
		}
	}
}

void CChatRoomServerDlg::SendClientMsg(MyMsgStruct* sendMsg, ClientStruct* pSend, int ifms)	//sendMsg：发送的信息结构体，pSend：目的客户端， ifms：1表示向所有客户端发送，0表示向pSend发送
{	
		list<ClientStruct>::iterator iter;  //定义迭代器
		//向所有客户端发送
		if (ifms) {
			//遍历客户端链表
			for (iter = m_ClientList.begin(); iter != m_ClientList.end(); ) {
				if (pSend != NULL){
					send(iter->Socket, (char*)sendMsg, sizeof(MyMsgStruct), 0);
				}
				iter++;
			}
		}
		//向目的客户端发送
		else {
			for (iter = m_ClientList.begin(); iter != m_ClientList.end(); ) {
				//寻找与目的客户端信息一致的链表节点
				if (pSend != NULL && iter->hThread == pSend->hThread && iter->strIp == pSend->strIp && iter->Socket == pSend->Socket){
					send(iter->Socket, (char*)sendMsg, sizeof(MyMsgStruct), 0);
					break;
				}
				iter++;
			}
		}
}
       
void CChatRoomServerDlg::SendClientList(MyMsgStruct* sendMsg, ClientStruct* pSend)		//sendMsg：发送的信息结构体，pSend：目的客户端
{
	list<ClientStruct>::iterator iter;  //定义迭代器
	int i = 0;//i表示在线成员数量
	//遍历客户端链表
	for (iter = m_ClientList.begin(); iter != m_ClientList.end(); ) {
		//寻找与目的客户端信息一致的链表节点
		if (pSend != NULL && iter->hThread == pSend->hThread && iter->strIp == pSend->strIp && iter->Socket == pSend->Socket) {
			list<ClientStruct>::iterator iter2;  //定义迭代器
			//把客户端链表的每一个成员名字存放在消息结构体的消息位置
			for (iter2 = m_ClientList.begin(); iter2 != m_ClientList.end(); ) {
				if (iter2->myClientName != NULL && iter2->myClientName!="") {   
					strncpy_s(sendMsg->Message+i*20,  sizeof(iter2->myClientName), iter2->myClientName, sizeof(iter2->myClientName));
				}
				iter2++;
				i++;
			}
			sendMsg->type = _login;
			sendMsg->clientnum = i;
			//发送
			send(iter->Socket, (char*)sendMsg, sizeof(MyMsgStruct), 0);
			break;
		}
		iter++;
	}
}

void CChatRoomServerDlg::SetClientList(ClientStruct* pSet,char myClientName[20])
{
	list<ClientStruct>::iterator iter;  //定义迭代器
	for (iter = m_ClientList.begin(); iter != m_ClientList.end(); ) {
		if (pSet != NULL && iter->hThread == pSet->hThread && iter->strIp == pSet->strIp && iter->Socket == pSet->Socket) {
			strncpy_s(iter->myClientName, myClientName, sizeof(iter->myClientName));
			break;
		}
		iter++;
	}
}

void CChatRoomServerDlg::SendClientMsgOTO(MyMsgStruct* sendMsg, ClientStruct* pSend)	//sendMsg：发送的信息结构体，pSend：目的客户端
{
	int flag = 0;//flag表示目的客户端的状态，0代表目的客户端不在线，1为在线
	list<ClientStruct>::iterator iter;  //定义迭代器
	for (iter = m_ClientList.begin(); iter != m_ClientList.end(); ) {
		//搜索与目的客户端名字相同的链表节点
		if (pSend != NULL && strcmp(iter->myClientName, sendMsg->myClientName) == 0) {
			flag = 1;
			break;
		}
		iter++;
	}
	//如果目的客户端在线
	if (flag) {
		send(iter->Socket, (char*)sendMsg, sizeof(MyMsgStruct), 0);
		ClientStruct u;
		m_ClientList.push_front(*iter);
		m_ClientList.erase(iter);
	}
	else {
		//如果目的客户端不在线
		if (pSend != NULL) {
			//传回发送客户端目标不在线信息
			strncpy_s(sendMsg->myClientPwd, sendMsg->myClientName, sizeof(sendMsg->myClientPwd));
			strncpy_s(sendMsg->Message, "对方当前不在线", sizeof(sendMsg->Message));
			send(pSend->Socket, (char*)sendMsg, sizeof(MyMsgStruct), 0);
		}
	}
}

void CChatRoomServerDlg::OnBnClickedOpenserver()
{
	
	m_hListenThread = CreateThread(NULL, 0, ListenThreadFunc, this, 0, NULL);
}


void CChatRoomServerDlg::OnLbnSelchangeList2()
{
	// TODO: 在此添加控件通知处理程序代码
}


void CChatRoomServerDlg::OnBnClickedButton2()
{
	MyMsgStruct smsg;
	CString strMsg;
	GetDlgItemText(IDC_EDIT2, strMsg);
	string text = CT2A(strMsg.GetBuffer(0));////0的意思是缓冲区至少有0个字符串。即缓冲区使用CString原本的内存大小
	strMsg.ReleaseBuffer();
	if (text == "") {
		return;
	}

	////查询是否已在黑名单
	string forbidText;
	ifstream instuf("forbidText.txt", ios::in);
	////从instuf中循环获取已有的 forbidText
	if (instuf) {
		while (instuf >> forbidText) {                                                                                
			if (forbidText == text) {
				AfxMessageBox(_T("该词已在黑名单，请勿重复添加！"));
				instuf.close();
				SetDlgItemText(IDC_EDIT2, _T(""));
				return;
			}
		}
		instuf.close();
	}
	////如果不在黑名单，就加入黑名单
	ofstream outstuf;
	outstuf.open("forbidText.txt", ios::app);
	outstuf << text << "\r\n";
	outstuf.close();
	SetDlgItemText(IDC_EDIT2, _T(""));
	return;
}


void CChatRoomServerDlg::OnBnClickedButton3()
{
	
	return;
}
