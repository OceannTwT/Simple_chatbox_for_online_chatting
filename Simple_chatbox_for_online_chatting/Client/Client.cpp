#include "pch.h"
#include <afxwin.h> 
//#include "stdafx.h"
#include<fstream>
#include "iostream"
#include "ChatRoom_KBChatRoomDlg.h"
#include "ChatRoom_KBOTOChatDlg.h"
#include "resource.h"
#include "chat.h"
//#include "afxdialogex.h"
using namespace std;
#define MAX_BUF_SIZE 2052



BOOL SOCKET_Select(SOCKET hSocket, int nTimeOut, BOOL bRead)
{
	fd_set fdset;
	timeval tv;
	FD_ZERO(&fdset); //清除套接字集合变量
	FD_SET(hSocket, &fdset); //将套接字 s 添加到 fdread集合
	nTimeOut = nTimeOut > 1000 ? 1000 : nTimeOut;
	tv.tv_sec = 0;
	tv.tv_usec = nTimeOut;
	int iRet = 0;
	if (bRead) {
		iRet = select(0, &fdset, NULL, NULL, &tv);
	}
	else {
		iRet = select(0, NULL, &fdset, NULL, &tv);
	}
	if (iRet <= 0) {
		return FALSE;
	}
	else if (FD_ISSET(hSocket, &fdset)) {
		return TRUE;
	}
	return FALSE;
}

UINT ClientThreadCon(LPVOID pParam) {
	//建立socket
	ChatRoom_KBChatRoomDlg* pChatRoom = (ChatRoom_KBChatRoomDlg*)pParam;
	ASSERT(pChatRoom != NULL);
	pChatRoom->m_ConnectSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (pChatRoom->m_ConnectSocket == INVALID_SOCKET) {
		AfxMessageBox(_T("Socket建立失败"));
		return false;
	}
	char IpAddr[16] = { 0 };
	sockaddr_in server;
	memset((void*)&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(short(5567));
	inet_pton(AF_INET, "192.168.43.15", &server.sin_addr);
	if (connect(pChatRoom->m_ConnectSocket, (struct sockaddr*) & server, sizeof(server)) == SOCKET_ERROR) {
		AfxMessageBox(_T("连接失败"));
		closesocket(pChatRoom->m_ConnectSocket);
		return false;
	}
	MyMsgStruct mymsgs;
	//向服务器发送建立消息信息，告诉有用户登陆
	mymsgs.type = _clientAdd;
	mymsgs.clientnum = 0;
	string msg = CT2A(pChatRoom->m_Name.GetBuffer(0));
	strncpy_s(mymsgs.myClientName, msg.c_str(), sizeof(mymsgs.myClientName));
	send(pChatRoom->m_ConnectSocket, (char*)&mymsgs, sizeof(MyMsgStruct), 0);
	pChatRoom->m_Name.ReleaseBuffer();
	Sleep(100);
	//遍历私聊对话框，为每个私聊框添加客户端socket信息
	list<ChatRoom_KBOTOChatDlg*>::iterator iter;  //定义迭代器
	for (iter = pChatRoom->m_OTOChatList.begin(); iter != pChatRoom->m_OTOChatList.end(); ) {
		(*iter)->m_OTOConnectSocket = pChatRoom->m_ConnectSocket;
		(*iter)->Mname = pChatRoom->m_Name;
		(*iter)->m_IsReveived = TRUE;
		iter++;
	}
	ifcon = 1;


	while (true) {
		if (SOCKET_Select(pChatRoom->m_ConnectSocket)) {
			TCHAR szBUF[MAX_BUF_SIZE + 1] = { 0 };
			int iRet = recv(pChatRoom->m_ConnectSocket, (char*)szBUF, MAX_BUF_SIZE, 0);
			//成功收到信息
			if (iRet > 0) {
				MyMsgStruct myrecv;
				memcpy(&myrecv, szBUF, sizeof(MyMsgStruct));
				//群聊消息直接在聊天框显示
				if (myrecv.type == _sendMessage) {
					CString showmsg;
					string smsg;
					smsg = myrecv.Message;
					showmsg = smsg.c_str();
					pChatRoom->ShowMessage(showmsg);
				}
				else	if (myrecv.type == _clientAdd) {   //其他客户端建立消息则添加进当前在线成员列表
					CString MemberName;
					string sname;
					sname = myrecv.myClientName;
					MemberName = sname.c_str();
					pChatRoom->AddListMember(MemberName);
				}
				else	if (myrecv.type == _privateMessage) {  //私聊消息处理
					int flag = 0;//flag判断与发送者私聊窗口是否已创建
					//发送者名字
					CString SName;
					string sSname;
					sSname = myrecv.myClientPwd;
					SName = sSname.c_str();
					//接收者名字
					CString TName;
					string sTname;
					sTname = myrecv.myClientName;
					TName = sTname.c_str();
					//发送信息
					CString showmsg;
					string smsg;
					smsg = myrecv.Message;
					showmsg = smsg.c_str();
					Sleep(100);
					list<ChatRoom_KBOTOChatDlg*>::iterator iter;  //定义迭代器
					//检索与发送者私聊窗口是否创建
					for (iter = pChatRoom->m_OTOChatList.begin(); iter != pChatRoom->m_OTOChatList.end(); ) {
						//如果窗口已创建
						if ((*iter)->Tname == SName) {
							(*iter)->ShowMessage(showmsg);
							//如果与发送者私聊窗口没显示
							if (!(*iter)->IsWindowVisible()) {
								int row = pChatRoom->L_MemberList.FindStringExact(0, SName);
								//设置成员列表发送者名称颜色为红色
								::SendMessage(AfxGetMainWnd()->GetSafeHwnd(), ChangeColor, (WPARAM)RGB(255, 0, 0), (LPARAM)row);
								pChatRoom->L_MemberList.SetCurSel(row);
								pChatRoom->L_MemberList.SetCurSel(-1);
								Sleep(1000);
							}
							flag = 1;
							break;
						}
						iter++;
					}
					//如果没有创建发送者私聊窗口
					if (!flag) {
						//设置成员列表发送者名称颜色为红色
						int row = pChatRoom->L_MemberList.FindStringExact(0, SName);
						::SendMessage(AfxGetMainWnd()->GetSafeHwnd(), ChangeColor, (WPARAM)RGB(255, 0, 0), (LPARAM)row);
						Sleep(1000);
						//创建发送者私聊窗口
						ChatRoom_KBOTOChatDlg* pDlg = new ChatRoom_KBOTOChatDlg();
						::SendMessage(AfxGetMainWnd()->GetSafeHwnd(), TESTMESSAGE, (WPARAM)pDlg, NULL);
						Sleep(10);
						pDlg->Mname = pChatRoom->m_Name;
						pDlg->Tname = SName;
						pDlg->ShowWindow(SW_SHOW);
						Sleep(10);
						pDlg->ShowMessage(showmsg);
						Sleep(10);
						pDlg->ShowWindow(SW_HIDE);
						Sleep(10);
						pDlg->ShowWindow(SW_SHOW);
						Sleep(10);
						pDlg->ShowWindow(SW_HIDE);
						Sleep(10);
						pDlg->m_OTOConnectSocket = pChatRoom->m_ConnectSocket;
						pChatRoom->m_OTOChatList.push_back(pDlg);
						pDlg->m_IsReveived = TRUE;
					}
				}
				else if (myrecv.type == _privateFileLen) {  //私聊文件请求处理

				   //原发送者（对方）名字
					CString SName;
					string sSname;
					sSname = myrecv.myClientPwd;
					SName = sSname.c_str();
					//原接收者（我）名字
					CString TName;
					string sTname;
					sTname = myrecv.myClientName;
					TName = sTname.c_str();
					//发送信息：文件名/内容
					CString FileName;
					string strfile;
					strfile = myrecv.Message;
					FileName = strfile.c_str();
					int pos = FileName.ReverseFind('\\');
					CString name = FileName.Mid(pos + 1, FileName.GetLength());
					//接收文件的长度
					int filesize = myrecv.myfilesize;

					//获取对应的OTO对象

					list<ChatRoom_KBOTOChatDlg*>::iterator iter;  //定义迭代器
					//检索与发送者私聊窗口是否创建
					for (iter = pChatRoom->m_OTOChatList.begin(); iter != pChatRoom->m_OTOChatList.end(); ) {
						//如果窗口已创建
						if ((*iter)->Tname == SName) {
							break;
						}
						iter++;
					}

					ChatRoom_KBOTOChatDlg* pDlg = new ChatRoom_KBOTOChatDlg();
					if (iter == pChatRoom->m_OTOChatList.end()) {//如果没有对应的私信区

						//创建发送者私聊窗口
						::SendMessage(AfxGetMainWnd()->GetSafeHwnd(), TESTMESSAGE, (WPARAM)pDlg, NULL);
						Sleep(10);
						pDlg->Mname = pChatRoom->m_Name;
						pDlg->Tname = SName;
						pDlg->m_OTOConnectSocket = pChatRoom->m_ConnectSocket;
						pChatRoom->m_OTOChatList.push_back(pDlg);
						pDlg->m_IsReveived = TRUE;
					}
					else {
						pDlg = *iter;
					}
					Sleep(100);

					if (pDlg->m_IsReveived) //如果是第一次接收数据,获取接收的文件信息
					{
						BROWSEINFOA bInfo;
						bInfo.hwndOwner = AfxGetMainWnd()->m_hWnd;
						bInfo.pidlRoot = NULL;
						char filename[MAX_PATH];
						char dir[MAX_PATH];
						memset(filename, 0, MAX_PATH);
						memset(dir, 0, MAX_PATH);

						////WINDOWS的数据结构，bInfo存储了用户选中目录的重要信
						bInfo.pszDisplayName = filename;
						bInfo.lpszTitle = "请选择目的路径";
						bInfo.ulFlags = BIF_BROWSEINCLUDEFILES | BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS | BIF_BROWSEFORCOMPUTER;
						bInfo.lParam = NULL;
						bInfo.lpfn = NULL;
						bInfo.iImage = 0;

						LPITEMIDLIST lpList = SHBrowseForFolderA(&bInfo);////WINDOWS选择目录

						CString filedir;
						if (SHGetPathFromIDListA(lpList, dir))////如果成功把项目标志符列表lpList转换为文档系统路径dir
						{
							//pDlg->m_File.Abort();
							filedir = dir;////dir即filedir\\是所选路径，
							filedir += "\\";

							pDlg->of.open(filedir + name, ios::app);
							//pDlg->m_File.Open(filedir + name, CFile::modeCreate | CFile::modeWrite);
							pDlg->m_IsReveived = FALSE;  //文件信息接收结束
						}

						pDlg->m_FileWholeLen = filesize;

						//告诉对方可以发文件了
						MyMsgStruct smsg;
						smsg.type = _privateFileReply;
						strncpy_s(smsg.myClientPwd, myrecv.myClientName, sizeof(smsg.myClientPwd));
						strncpy_s(smsg.myClientName, myrecv.myClientPwd, sizeof(smsg.myClientName));
						strncpy_s(smsg.Message, "start", sizeof(smsg.Message));
						send(pDlg->m_OTOConnectSocket, (char*)&smsg, sizeof(MyMsgStruct), 0);

						//
					}
					else //开始接收数据，而非info
					{
						CString FileContent = FileName;
						//int filelen = m_ServerSock->Receive(buffer, MAXSENDNUM + 1);////buffer
						pDlg->m_FileLen += filesize;////更新：已接收的文件大小

						for (int i = 0; i < filesize; i++)
						{
							pDlg->of << (char)FileName[i];
						}
						//pDlg->m_File.Write(FileName, filesize);////把filelen个单位的buffer写入文件m_File
						if (pDlg->m_FileLen >= pDlg->m_FileWholeLen)  //接收完成
						{
							MyMsgStruct smsg;
							smsg.type = _privateFileReply;
							strncpy_s(smsg.myClientPwd, myrecv.myClientName, sizeof(smsg.myClientPwd));
							strncpy_s(smsg.myClientName, myrecv.myClientPwd, sizeof(smsg.myClientName));
							strncpy_s(smsg.Message, "finish", sizeof(smsg.Message));
							send(pDlg->m_OTOConnectSocket, (char*)&smsg, sizeof(MyMsgStruct), 0);
							//pDlg->m_File.Close();
							pDlg->m_IsReveived = TRUE;
							pDlg->of.close();
							AfxMessageBox(_T("接收完成"));
							
						}
						else////继续接收
						{
							MyMsgStruct smsg;
							smsg.type = _privateFileReply;
							strncpy_s(smsg.myClientPwd, myrecv.myClientName, sizeof(smsg.myClientPwd));
							strncpy_s(smsg.myClientName, myrecv.myClientPwd, sizeof(smsg.myClientName));
							strncpy_s(smsg.Message, "unfin", sizeof(smsg.Message));
							send(pDlg->m_OTOConnectSocket, (char*)&smsg, sizeof(MyMsgStruct), 0);
						}
					}
				}
				else if (myrecv.type == _privateFileReply) {  //私聊文件内容的发送

				  //原发送者（对方）名字
					CString SName;
					string sSname;
					sSname = myrecv.myClientPwd;
					SName = sSname.c_str();
					//原接收者（我）名字
					CString TName;
					string sTname;
					sTname = myrecv.myClientName;
					TName = sTname.c_str();
					//发送信息：文件名/内容
					CString FileName;
					string strfile;
					strfile = myrecv.Message;
					FileName = strfile.c_str();
					int pos = FileName.ReverseFind('\\');
					CString name = FileName.Mid(pos + 1, FileName.GetLength());
					//接收文件的长度
					int filesize = myrecv.myfilesize;

					//获取对应的OTO对象

					list<ChatRoom_KBOTOChatDlg*>::iterator iter;  //定义迭代器

					//检索与发送者私聊窗口是否创建
					for (iter = pChatRoom->m_OTOChatList.begin(); iter != pChatRoom->m_OTOChatList.end(); ) {
						//如果窗口已创建
						if ((*iter)->Tname == SName) {

							break;
						}
						iter++;
					}
					ChatRoom_KBOTOChatDlg* pDlg = *iter;
					//发文件的内容
					MyMsgStruct smsg;
					smsg.type = _privateFileLen;
					strncpy_s(smsg.myClientPwd, myrecv.myClientName, sizeof(smsg.myClientPwd));
					strncpy_s(smsg.myClientName, myrecv.myClientPwd, sizeof(smsg.myClientName));

					if (strfile == "start" || strfile == "unfin") {

						int result;

				

						if ((pDlg->m_SendLen < pDlg->m_SendFileLen) && ((pDlg->m_SendLen + MAXSENDNUM) > pDlg->m_SendFileLen))//最后一次发送
						{
							for (int i = pDlg->m_SendLen; i < pDlg->m_SendFileLen; i++)
							{
								smsg.Message[i - pDlg->m_SendLen] = pDlg->contexts[i];
							}
							smsg.Message[pDlg->m_SendFileLen - pDlg->m_SendLen] = '\0'; 
							/*pDlg->m_SendFile.Read(smsg.Message, pDlg->m_SendFileLen - pDlg->m_SendLen);
							smsg.Message[pDlg->m_SendFileLen - pDlg->m_SendLen] = '\0';*/


							//result = pDlg->m_pSendSock->Send(membuf, pDlg->m_SendFileLen - pDlg->m_SendLen);
							//strncpy_s(smsg.Message, "start", sizeof(smsg.Message));
							smsg.myfilesize = pDlg->m_SendFileLen - pDlg->m_SendLen;
							send(pDlg->m_OTOConnectSocket, (char*)&smsg, sizeof(MyMsgStruct), 0);
							/*
							if (result == SOCKET_ERROR)
							{
								pDlg->m_Send = FALSE;
								pDlg->m_SendNext = FALSE;
								pDlg->m_SendFile.Close();
								m_pSendSock->Close();
								m_pSendSock->Create(CLIENTPORT);
								LocalFree(membuf);
								KillTimer(1);
								MessageBox("发送失败");
								return;
							}*/
							pDlg->m_SendLen = pDlg->m_SendFileLen;

							AfxMessageBox(_T("发送完成"));
						}
						else
						{
						/*	pDlg->m_SendFile.Read(smsg.Message, MAXSENDNUM);
							smsg.Message[pDlg->m_SendFileLen - pDlg->m_SendLen] = '\0';
						*/	//result = m_pSendSock->Send(membuf, MAXSENDNUM);

							for (int i = pDlg->m_SendLen; i < pDlg->m_SendLen + MAXSENDNUM; i++)
							{
								smsg.Message[i - pDlg->m_SendLen] = pDlg->contexts[i];
							}
							smsg.Message[MAXSENDNUM] = '\0';
							smsg.myfilesize = MAXSENDNUM;
							send(pDlg->m_OTOConnectSocket, (char*)&smsg, sizeof(MyMsgStruct), 0);
							/*if (result == SOCKET_ERROR)
							{
								m_Send = FALSE;
								m_SendNext = FALSE;
								m_SendFile.Close();
								m_pSendSock->Close();
								m_pSendSock->Create(CLIENTPORT);

								LocalFree(membuf);
								KillTimer(1);
								MessageBox("发送失败");
								return;
							}
							LocalFree(membuf);*/
							pDlg->m_SendLen += MAXSENDNUM;
						}
					}
					else if (strfile == "finish") {
						AfxMessageBox(_T("对方已接收"));
					}
				}
				else  if (myrecv.type == _clientExit) {//在线成员离开消息处理
					//显示离开信息
					CString showmsg;
					string smsg;
					smsg = myrecv.Message;
					showmsg = smsg.c_str();
					pChatRoom->ShowMessage(showmsg);
					//删除成员列表中成员
					CString MemberName;
					string sname;
					sname = myrecv.myClientName;
					MemberName = sname.c_str();
					if (MemberName != "") {
						int row = pChatRoom->L_MemberList.FindStringExact(0, MemberName);
						pChatRoom->L_MemberList.DeleteString(row);
					}
				}
			}
			else {
				pChatRoom->ShowMessage(_T("聊天室服务器已停止，请重新连接！\r\n"));
				break;
			}
		}
		Sleep(500);
	}
	closesocket(pChatRoom->m_ConnectSocket);
	return 0;
}

BOOL ClientRegister(MyMsgStruct* mymsgs) {
	//新建socket
	SOCKET reg = socket(AF_INET, SOCK_STREAM, 0);
	if (reg == INVALID_SOCKET) {
		AfxMessageBox(_T("Socket建立失败"));
		return false;
	}
	char IpAddr[16] = { 0 };
	sockaddr_in server;
	memset((void*)&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(short(5567));
	inet_pton(AF_INET, "192.168.43.15", &server.sin_addr);
	//与服务器进行连接
	if (connect(reg, (struct sockaddr*) & server, sizeof(server)) == SOCKET_ERROR) {
		AfxMessageBox(_T("连接失败"));
		closesocket(reg);
		return false;
	}
	//发送注册信息
	send(reg, (char*)mymsgs, sizeof(MyMsgStruct), 0);
	//监听等待结果
	while (true) {
		if (SOCKET_Select(reg)) {
			TCHAR szBUF[MAX_BUF_SIZE + 1] = { 0 };
			int iRet = recv(reg, (char*)szBUF, MAX_BUF_SIZE + 1, 0);
			Sleep(100); 
			if (iRet > 0) {
				MyMsgStruct myrecv;
				memcpy(&myrecv, szBUF, sizeof(MyMsgStruct));
				if (myrecv.type == _register)
				{
					CString showmsg;
					string smsg;
					smsg = myrecv.Message;
					showmsg = smsg.c_str();
					AfxMessageBox(showmsg);
					break;
				}
			}
			else {
				AfxMessageBox(_T("连接失败"));
				closesocket(reg);
				return false;
			}
		}
		Sleep(500);
	}
	closesocket(reg);
	return true;
}

BOOL ClientLogin(MyMsgStruct* mymsgs, ChatRoom_KBChatRoomDlg* ChatRoom) {  //mymsgs为要发送的信息结构体，ChatRoom为聊天室主界面类
	//建立socket
	SOCKET reg = socket(AF_INET, SOCK_STREAM, 0);
	if (reg == INVALID_SOCKET) {
		AfxMessageBox(_T("Socket建立失败"));
		return false;
	}
	char IpAddr[16] = { 0 };
	sockaddr_in server;
	memset((void*)&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(short(5567));
	inet_pton(AF_INET, "192.168.43.15", &server.sin_addr);
	//进行连接
	if (connect(reg, (struct sockaddr*) & server, sizeof(server)) == SOCKET_ERROR) {
		AfxMessageBox(_T("连接失败"));
		closesocket(reg);
		return false;
	}
	//发送信息
	send(reg, (char*)mymsgs, sizeof(MyMsgStruct), 0);
	while (true) {
		//监听消息
		if (SOCKET_Select(reg)) {
			TCHAR szBUF[MAX_BUF_SIZE + 1] = { 0 };
			int iRet = recv(reg, (char*)szBUF, MAX_BUF_SIZE + 1, 0);
			if (iRet > 0) {
				MyMsgStruct myrecv;
				memcpy(&myrecv, szBUF, sizeof(MyMsgStruct));
				//如果登录成功
				if (myrecv.type == _login) {
					char uname[20];
					//服务器回传递当前在线成员信息，将当前在线成员名字添加到在线列表中，并为它们都建立私聊窗口
					for (int i = 0; i < myrecv.clientnum; i++) {
						strncpy_s(uname, sizeof(uname), myrecv.Message + i * 20, 20);
						CString myClientName;
						string susername;
						susername = uname;
						myClientName = susername.c_str();
						if (myClientName != "") {
							ChatRoom->AddListMember(myClientName);
							if (myClientName != ChatRoom->m_Name) {
								ChatRoom_KBOTOChatDlg* pDlg = new ChatRoom_KBOTOChatDlg();
								pDlg->Create(IDD_CHATROOM_OTO_DIALOG, ChatRoom);
								pDlg->Tname = myClientName;
								ChatRoom->m_OTOChatList.push_back(pDlg);
							}
							//ChatRoom
						}
					}
					break;
					//如果登录失败
				}
				else if (myrecv.type == _fail) {
					CString showmsg;
					string smsg;
					smsg = myrecv.Message;
					showmsg = smsg.c_str();
					AfxMessageBox(showmsg);
					closesocket(reg);
					return false;
				}
				else {
					closesocket(reg);
					return false;
				}
			}
			else {
				AfxMessageBox(_T("连接失败"));
				closesocket(reg);
				return false;
			}
		}
		Sleep(500);
	}
	closesocket(reg);
	return true;
}
