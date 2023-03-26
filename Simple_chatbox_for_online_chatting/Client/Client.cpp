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
	FD_ZERO(&fdset); //����׽��ּ��ϱ���
	FD_SET(hSocket, &fdset); //���׽��� s ��ӵ� fdread����
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
	//����socket
	ChatRoom_KBChatRoomDlg* pChatRoom = (ChatRoom_KBChatRoomDlg*)pParam;
	ASSERT(pChatRoom != NULL);
	pChatRoom->m_ConnectSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (pChatRoom->m_ConnectSocket == INVALID_SOCKET) {
		AfxMessageBox(_T("Socket����ʧ��"));
		return false;
	}
	char IpAddr[16] = { 0 };
	sockaddr_in server;
	memset((void*)&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(short(5567));
	inet_pton(AF_INET, "192.168.43.15", &server.sin_addr);
	if (connect(pChatRoom->m_ConnectSocket, (struct sockaddr*) & server, sizeof(server)) == SOCKET_ERROR) {
		AfxMessageBox(_T("����ʧ��"));
		closesocket(pChatRoom->m_ConnectSocket);
		return false;
	}
	MyMsgStruct mymsgs;
	//����������ͽ�����Ϣ��Ϣ���������û���½
	mymsgs.type = _clientAdd;
	mymsgs.clientnum = 0;
	string msg = CT2A(pChatRoom->m_Name.GetBuffer(0));
	strncpy_s(mymsgs.myClientName, msg.c_str(), sizeof(mymsgs.myClientName));
	send(pChatRoom->m_ConnectSocket, (char*)&mymsgs, sizeof(MyMsgStruct), 0);
	pChatRoom->m_Name.ReleaseBuffer();
	Sleep(100);
	//����˽�ĶԻ���Ϊÿ��˽�Ŀ���ӿͻ���socket��Ϣ
	list<ChatRoom_KBOTOChatDlg*>::iterator iter;  //���������
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
			//�ɹ��յ���Ϣ
			if (iRet > 0) {
				MyMsgStruct myrecv;
				memcpy(&myrecv, szBUF, sizeof(MyMsgStruct));
				//Ⱥ����Ϣֱ�����������ʾ
				if (myrecv.type == _sendMessage) {
					CString showmsg;
					string smsg;
					smsg = myrecv.Message;
					showmsg = smsg.c_str();
					pChatRoom->ShowMessage(showmsg);
				}
				else	if (myrecv.type == _clientAdd) {   //�����ͻ��˽�����Ϣ����ӽ���ǰ���߳�Ա�б�
					CString MemberName;
					string sname;
					sname = myrecv.myClientName;
					MemberName = sname.c_str();
					pChatRoom->AddListMember(MemberName);
				}
				else	if (myrecv.type == _privateMessage) {  //˽����Ϣ����
					int flag = 0;//flag�ж��뷢����˽�Ĵ����Ƿ��Ѵ���
					//����������
					CString SName;
					string sSname;
					sSname = myrecv.myClientPwd;
					SName = sSname.c_str();
					//����������
					CString TName;
					string sTname;
					sTname = myrecv.myClientName;
					TName = sTname.c_str();
					//������Ϣ
					CString showmsg;
					string smsg;
					smsg = myrecv.Message;
					showmsg = smsg.c_str();
					Sleep(100);
					list<ChatRoom_KBOTOChatDlg*>::iterator iter;  //���������
					//�����뷢����˽�Ĵ����Ƿ񴴽�
					for (iter = pChatRoom->m_OTOChatList.begin(); iter != pChatRoom->m_OTOChatList.end(); ) {
						//��������Ѵ���
						if ((*iter)->Tname == SName) {
							(*iter)->ShowMessage(showmsg);
							//����뷢����˽�Ĵ���û��ʾ
							if (!(*iter)->IsWindowVisible()) {
								int row = pChatRoom->L_MemberList.FindStringExact(0, SName);
								//���ó�Ա�б�����������ɫΪ��ɫ
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
					//���û�д���������˽�Ĵ���
					if (!flag) {
						//���ó�Ա�б�����������ɫΪ��ɫ
						int row = pChatRoom->L_MemberList.FindStringExact(0, SName);
						::SendMessage(AfxGetMainWnd()->GetSafeHwnd(), ChangeColor, (WPARAM)RGB(255, 0, 0), (LPARAM)row);
						Sleep(1000);
						//����������˽�Ĵ���
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
				else if (myrecv.type == _privateFileLen) {  //˽���ļ�������

				   //ԭ�����ߣ��Է�������
					CString SName;
					string sSname;
					sSname = myrecv.myClientPwd;
					SName = sSname.c_str();
					//ԭ�����ߣ��ң�����
					CString TName;
					string sTname;
					sTname = myrecv.myClientName;
					TName = sTname.c_str();
					//������Ϣ���ļ���/����
					CString FileName;
					string strfile;
					strfile = myrecv.Message;
					FileName = strfile.c_str();
					int pos = FileName.ReverseFind('\\');
					CString name = FileName.Mid(pos + 1, FileName.GetLength());
					//�����ļ��ĳ���
					int filesize = myrecv.myfilesize;

					//��ȡ��Ӧ��OTO����

					list<ChatRoom_KBOTOChatDlg*>::iterator iter;  //���������
					//�����뷢����˽�Ĵ����Ƿ񴴽�
					for (iter = pChatRoom->m_OTOChatList.begin(); iter != pChatRoom->m_OTOChatList.end(); ) {
						//��������Ѵ���
						if ((*iter)->Tname == SName) {
							break;
						}
						iter++;
					}

					ChatRoom_KBOTOChatDlg* pDlg = new ChatRoom_KBOTOChatDlg();
					if (iter == pChatRoom->m_OTOChatList.end()) {//���û�ж�Ӧ��˽����

						//����������˽�Ĵ���
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

					if (pDlg->m_IsReveived) //����ǵ�һ�ν�������,��ȡ���յ��ļ���Ϣ
					{
						BROWSEINFOA bInfo;
						bInfo.hwndOwner = AfxGetMainWnd()->m_hWnd;
						bInfo.pidlRoot = NULL;
						char filename[MAX_PATH];
						char dir[MAX_PATH];
						memset(filename, 0, MAX_PATH);
						memset(dir, 0, MAX_PATH);

						////WINDOWS�����ݽṹ��bInfo�洢���û�ѡ��Ŀ¼����Ҫ��
						bInfo.pszDisplayName = filename;
						bInfo.lpszTitle = "��ѡ��Ŀ��·��";
						bInfo.ulFlags = BIF_BROWSEINCLUDEFILES | BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS | BIF_BROWSEFORCOMPUTER;
						bInfo.lParam = NULL;
						bInfo.lpfn = NULL;
						bInfo.iImage = 0;

						LPITEMIDLIST lpList = SHBrowseForFolderA(&bInfo);////WINDOWSѡ��Ŀ¼

						CString filedir;
						if (SHGetPathFromIDListA(lpList, dir))////����ɹ�����Ŀ��־���б�lpListת��Ϊ�ĵ�ϵͳ·��dir
						{
							//pDlg->m_File.Abort();
							filedir = dir;////dir��filedir\\����ѡ·����
							filedir += "\\";

							pDlg->of.open(filedir + name, ios::app);
							//pDlg->m_File.Open(filedir + name, CFile::modeCreate | CFile::modeWrite);
							pDlg->m_IsReveived = FALSE;  //�ļ���Ϣ���ս���
						}

						pDlg->m_FileWholeLen = filesize;

						//���߶Է����Է��ļ���
						MyMsgStruct smsg;
						smsg.type = _privateFileReply;
						strncpy_s(smsg.myClientPwd, myrecv.myClientName, sizeof(smsg.myClientPwd));
						strncpy_s(smsg.myClientName, myrecv.myClientPwd, sizeof(smsg.myClientName));
						strncpy_s(smsg.Message, "start", sizeof(smsg.Message));
						send(pDlg->m_OTOConnectSocket, (char*)&smsg, sizeof(MyMsgStruct), 0);

						//
					}
					else //��ʼ�������ݣ�����info
					{
						CString FileContent = FileName;
						//int filelen = m_ServerSock->Receive(buffer, MAXSENDNUM + 1);////buffer
						pDlg->m_FileLen += filesize;////���£��ѽ��յ��ļ���С

						for (int i = 0; i < filesize; i++)
						{
							pDlg->of << (char)FileName[i];
						}
						//pDlg->m_File.Write(FileName, filesize);////��filelen����λ��bufferд���ļ�m_File
						if (pDlg->m_FileLen >= pDlg->m_FileWholeLen)  //�������
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
							AfxMessageBox(_T("�������"));
							
						}
						else////��������
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
				else if (myrecv.type == _privateFileReply) {  //˽���ļ����ݵķ���

				  //ԭ�����ߣ��Է�������
					CString SName;
					string sSname;
					sSname = myrecv.myClientPwd;
					SName = sSname.c_str();
					//ԭ�����ߣ��ң�����
					CString TName;
					string sTname;
					sTname = myrecv.myClientName;
					TName = sTname.c_str();
					//������Ϣ���ļ���/����
					CString FileName;
					string strfile;
					strfile = myrecv.Message;
					FileName = strfile.c_str();
					int pos = FileName.ReverseFind('\\');
					CString name = FileName.Mid(pos + 1, FileName.GetLength());
					//�����ļ��ĳ���
					int filesize = myrecv.myfilesize;

					//��ȡ��Ӧ��OTO����

					list<ChatRoom_KBOTOChatDlg*>::iterator iter;  //���������

					//�����뷢����˽�Ĵ����Ƿ񴴽�
					for (iter = pChatRoom->m_OTOChatList.begin(); iter != pChatRoom->m_OTOChatList.end(); ) {
						//��������Ѵ���
						if ((*iter)->Tname == SName) {

							break;
						}
						iter++;
					}
					ChatRoom_KBOTOChatDlg* pDlg = *iter;
					//���ļ�������
					MyMsgStruct smsg;
					smsg.type = _privateFileLen;
					strncpy_s(smsg.myClientPwd, myrecv.myClientName, sizeof(smsg.myClientPwd));
					strncpy_s(smsg.myClientName, myrecv.myClientPwd, sizeof(smsg.myClientName));

					if (strfile == "start" || strfile == "unfin") {

						int result;

				

						if ((pDlg->m_SendLen < pDlg->m_SendFileLen) && ((pDlg->m_SendLen + MAXSENDNUM) > pDlg->m_SendFileLen))//���һ�η���
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
								MessageBox("����ʧ��");
								return;
							}*/
							pDlg->m_SendLen = pDlg->m_SendFileLen;

							AfxMessageBox(_T("�������"));
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
								MessageBox("����ʧ��");
								return;
							}
							LocalFree(membuf);*/
							pDlg->m_SendLen += MAXSENDNUM;
						}
					}
					else if (strfile == "finish") {
						AfxMessageBox(_T("�Է��ѽ���"));
					}
				}
				else  if (myrecv.type == _clientExit) {//���߳�Ա�뿪��Ϣ����
					//��ʾ�뿪��Ϣ
					CString showmsg;
					string smsg;
					smsg = myrecv.Message;
					showmsg = smsg.c_str();
					pChatRoom->ShowMessage(showmsg);
					//ɾ����Ա�б��г�Ա
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
				pChatRoom->ShowMessage(_T("�����ҷ�������ֹͣ�����������ӣ�\r\n"));
				break;
			}
		}
		Sleep(500);
	}
	closesocket(pChatRoom->m_ConnectSocket);
	return 0;
}

BOOL ClientRegister(MyMsgStruct* mymsgs) {
	//�½�socket
	SOCKET reg = socket(AF_INET, SOCK_STREAM, 0);
	if (reg == INVALID_SOCKET) {
		AfxMessageBox(_T("Socket����ʧ��"));
		return false;
	}
	char IpAddr[16] = { 0 };
	sockaddr_in server;
	memset((void*)&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(short(5567));
	inet_pton(AF_INET, "192.168.43.15", &server.sin_addr);
	//���������������
	if (connect(reg, (struct sockaddr*) & server, sizeof(server)) == SOCKET_ERROR) {
		AfxMessageBox(_T("����ʧ��"));
		closesocket(reg);
		return false;
	}
	//����ע����Ϣ
	send(reg, (char*)mymsgs, sizeof(MyMsgStruct), 0);
	//�����ȴ����
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
				AfxMessageBox(_T("����ʧ��"));
				closesocket(reg);
				return false;
			}
		}
		Sleep(500);
	}
	closesocket(reg);
	return true;
}

BOOL ClientLogin(MyMsgStruct* mymsgs, ChatRoom_KBChatRoomDlg* ChatRoom) {  //mymsgsΪҪ���͵���Ϣ�ṹ�壬ChatRoomΪ��������������
	//����socket
	SOCKET reg = socket(AF_INET, SOCK_STREAM, 0);
	if (reg == INVALID_SOCKET) {
		AfxMessageBox(_T("Socket����ʧ��"));
		return false;
	}
	char IpAddr[16] = { 0 };
	sockaddr_in server;
	memset((void*)&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(short(5567));
	inet_pton(AF_INET, "192.168.43.15", &server.sin_addr);
	//��������
	if (connect(reg, (struct sockaddr*) & server, sizeof(server)) == SOCKET_ERROR) {
		AfxMessageBox(_T("����ʧ��"));
		closesocket(reg);
		return false;
	}
	//������Ϣ
	send(reg, (char*)mymsgs, sizeof(MyMsgStruct), 0);
	while (true) {
		//������Ϣ
		if (SOCKET_Select(reg)) {
			TCHAR szBUF[MAX_BUF_SIZE + 1] = { 0 };
			int iRet = recv(reg, (char*)szBUF, MAX_BUF_SIZE + 1, 0);
			if (iRet > 0) {
				MyMsgStruct myrecv;
				memcpy(&myrecv, szBUF, sizeof(MyMsgStruct));
				//�����¼�ɹ�
				if (myrecv.type == _login) {
					char uname[20];
					//�������ش��ݵ�ǰ���߳�Ա��Ϣ������ǰ���߳�Ա������ӵ������б��У���Ϊ���Ƕ�����˽�Ĵ���
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
					//�����¼ʧ��
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
				AfxMessageBox(_T("����ʧ��"));
				closesocket(reg);
				return false;
			}
		}
		Sleep(500);
	}
	closesocket(reg);
	return true;
}
