#include "pch.h"
//#include "stdafx.h"
#include "chat.h"
#include "ChatRoom_ServerDlg.h"
#include "iostream"
#include "fstream"
#define MAX_BUF_SIZE 2052


BOOL SOCKET_Select(SOCKET hSocket, int nTimeOut, BOOL bRead)////�������ȴ�
{
	fd_set fdset;
	timeval tv;
	FD_ZERO(&fdset); //����׽��ּ��ϱ���
	FD_SET(hSocket, &fdset); //���׽��� hSocket ��ӵ� fdset����
	nTimeOut = nTimeOut > 1000 ? 1000 : nTimeOut;
	tv.tv_sec = 0;
	tv.tv_usec = nTimeOut;
	int iRet = 0;
	if (bRead) {////����ǿɶ��Ľӿڣ��������tvʱ����û�����Է�����Ϣ���ͽ�������������
		iRet = select(0, &fdset, NULL, NULL, &tv);
	}
	else {////����ǿ�д��
		iRet = select(0, NULL, &fdset, NULL, &tv);
	}
	if (iRet <= 0) {////��tvʱ����û���
		return FALSE;
	}
	else if (FD_ISSET(hSocket, &fdset)) {////��tvʱ���������
		return TRUE;
	}
	return FALSE; 
}

DWORD WINAPI ListenThreadFunc(LPVOID pParam) { //pParam�Ƿ���������
	CChatRoomServerDlg* pChatRoom = (CChatRoomServerDlg*)pParam;////��CChatRoom_ServerDlg.h�Զ���
	
	ASSERT(pChatRoom != NULL);
	////1.������ʽ�׽���socket�������׽��ֺ���pChatRoom->m_ListenSocket
	pChatRoom->m_ListenSocket = socket(AF_INET, SOCK_STREAM, 0); 
	if (pChatRoom->m_ListenSocket == INVALID_SOCKET) {
		AfxMessageBox(_T("SOCKET����ʧ��"));
		goto err;
	}
	////������ʽ�׽���socket�������׽��ֺ���sockSrv
	SOCKET sockSrv = socket(AF_INET, SOCK_STREAM, 0);
	if (sockSrv == INVALID_SOCKET) {
		AfxMessageBox(_T("SOCKET����ʧ��"));
		goto err;
	}
	////2.���׽��֡����׽���sockSrv�� IP ��ַaddrSrv�ڶ˿� 5567 ��
	SOCKADDR_IN addrSrv;
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(short(5567));
	inet_pton(AF_INET, "192.168.43.15", &addrSrv.sin_addr);////IP��ַת������,������ı���IP��ַת��Ϊ�����������ֽ��򡱵�IP��ַ
	
	if (bind(sockSrv, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR)) == SOCKET_ERROR) {
		AfxMessageBox(_T("socket��ʧ��"));
		goto err;
	}
	////3.�׽��� sockSrv ���ڼ���״̬������ͻ�����������Ϊ 5������ͻ������ǵ���connect()�����������󣬷������ͻ��յ��������
	if (listen(sockSrv, 5) == SOCKET_ERROR) {
		AfxMessageBox(_T("����ʧ��"));
		goto err;
	}
	
		
	////��������ʾ
	
	pChatRoom->ShowMessage(_T("�����������ɹ�"));
	
	while (1) {
		if (SOCKET_Select(sockSrv, 100, TRUE)) {
			SOCKADDR_IN addrClient;
			int len = sizeof(SOCKADDR);
			////4.�������ӣ��ȴ��ͻ��˵����ӡ��õ��µ�������sockConn��
			////���������̴���һ���µ��׽��� sockConn ����������׽��� sockSrv ���ܵ�һ
			////���ͻ���������Զ�̿ͻ����̵��׽��ֵ�ַ���� addrClient �洢��
			////accept������������Ӿͽ������ˡ�֮��Ϳ��Կ�ʼ����I/O������
			SOCKET sockConn = accept(sockSrv, (SOCKADDR*)&addrClient, &len);
			if (sockConn == INVALID_SOCKET) {
				AfxMessageBox(_T("1��"));
				continue;
			}
			////����һ��ClientStruct�Ķ���client����¼һ���ͻ��˵���Ϣ
			ClientStruct client;
			client.Socket = sockConn;
			char str[INET_ADDRSTRLEN];
			client.strIp = inet_ntop(AF_INET, &addrClient.sin_addr, str, sizeof(str));////�ͻ��˵�IP������ֵ��ʽaddrClient.sin_addr ת��Ϊ ���ʮ���Ƶ�ip��ַ��ʽstr
			client.pMainWind = pChatRoom;
			pChatRoom->m_ClientList.push_back(client);////�¿ͻ��˼���pChatRoom->m_ClientList����
			////����һ��ClientThreadCre�߳���client�ͻ���ͨ��
			client.hThread = CreateThread(NULL, 0, ClientThreadCre, &(pChatRoom->m_ClientList.back()), CREATE_SUSPENDED, NULL);
			pChatRoom->m_ClientList.back().hThread = client.hThread;
			ResumeThread(client.hThread);////�̵߳��ȣ��ø��̲߳�������
			
			Sleep(100);
		}
		
	}
err:////���ǰ��ĳ��ʧ����
	closesocket(pChatRoom->m_ListenSocket);
	return true;
}

DWORD WINAPI ClientThreadCre(LPVOID pParam) {
	////����һ��MyMsgStruct�Ķ���sendMsg�����ڴ洢����Ҫ��ͻ��ˡ�����������Ϣ����chat.h�Զ��塣
	MyMsgStruct sendMsg;
	sendMsg.clientnum = 0;////�ͻ��˵�ǰ���߳�Ա��Ϊ0
	////����һ��ClientStruct�Ķ���m_ClientItem����¼��ǰ��һ���ͻ��˵���Ϣ
	ClientStruct m_ClientItem = *(ClientStruct*)pParam;
	strncpy_s(m_ClientItem.myClientName, "", sizeof(m_ClientItem.myClientName));////������
	while (1) {
		if (SOCKET_Select(m_ClientItem.Socket, 100, TRUE)) {////�������ȴ������m_ClientItem.Socket�ɹ���ȡ���ͻ��˷���������
			////�ʹ���һ������szBUF���洢�յ�����Ϣ
			TCHAR szBUF[MAX_BUF_SIZE + 1] = { 0 };
			////����szBUF���洢m_ClientItem.Socket�յ�����Ϣ
			int iRet = recv(m_ClientItem.Socket, (char*)&szBUF, MAX_BUF_SIZE+1, 0);
			Sleep(100);
			////����ɹ�recv�Ļ�
			if (iRet > 0) {
				////����һ��MyMsgStruct�Ķ���mymsgs�����ڴ洢 �ӿͻ��ˡ��յ�������Ϣ����chat.h�Զ��塣
				MyMsgStruct mymsgs;
				memcpy(&mymsgs, szBUF, sizeof(MyMsgStruct));

				////�ͻ�������ע��
				if (mymsgs.type == _register) {
					string Name;
					string Password;
					ifstream instuf("data.txt", ios::in);////���ļ�data.txt���洢��ifstream�Ķ���instuf��
					sendMsg.type = _register;
					strncpy_s(sendMsg.myClientName, "", sizeof(sendMsg.myClientName));
					strncpy_s(sendMsg.myClientPwd, "", sizeof(sendMsg.myClientPwd));
					////��instuf��ѭ����ȡ���е� Name �� Password
					if (instuf) {
						while (instuf >> Name >> Password) {
							////���ע��������������                                                                                         
							if (Name == mymsgs.myClientName) {
								strncpy_s(sendMsg.Message, "���û������ڣ���ֱ�ӵ�¼", sizeof(sendMsg.Message));
								m_ClientItem.pMainWind->SendClientMsg(&sendMsg, &m_ClientItem,0);////����������CChatRoom_ServerDlg����Ŀ���û�m_ClientItem����sendMsg
								Sleep(100);
								m_ClientItem.pMainWind->RemoveClientFromList(m_ClientItem);////�ӵ�ǰ�������ҵ��û��б���ɾ�����û���Ϣ
								instuf.close();
								return true;
							}
						}
						instuf.close();
					}

					////���ע��������û������������������д��data.txt,��ע�����ڡ�ip���˻�д��register.txt
					ofstream outstuf;
					outstuf.open("data.txt", ios::app);
					outstuf << mymsgs.myClientName << " " << mymsgs.myClientPwd << "\r\n";
					outstuf.close();

					CTime tm; tm = CTime::GetCurrentTime();
					CString ti = tm.Format("%Y/%m/%d %X");					
					outstuf.open("register.txt", ios::app);
					string msg = CT2A(ti.GetBuffer(0));
					ti.ReleaseBuffer();
					string ip = CT2A(m_ClientItem.strIp.GetBuffer(0));
					m_ClientItem.strIp.ReleaseBuffer();
					outstuf << msg << " ��ipΪ" << ip << "ע�����û�" << mymsgs.myClientName << "\r\n";
					outstuf.close();

					////���߿ͻ���ע��ɹ�
					strncpy_s(sendMsg.Message, "�ɹ����ע��", sizeof(sendMsg.Message));
					m_ClientItem.pMainWind->SendClientMsg(&sendMsg, &m_ClientItem,0);
					Sleep(100);
					m_ClientItem.pMainWind->RemoveClientFromList(m_ClientItem);
					break;
				}

				////�ͻ��������½
				else if (mymsgs.type == _login) {
					////��data.txt���ж��˻������Ƿ���ȷ����ȷ��flag=1
					string Name;
					string Password;
					int flag = 0;		
					ifstream instuf("data.txt", ios::in);
					instuf.seekg(0, ios::beg);
					while (instuf >> Name >> Password) {
						if (Name == mymsgs.myClientName && Password == mymsgs.myClientPwd) {
							flag = 1;
							break;
						}
					}
					instuf.close();
					////����˺�������ȷ
					if (flag) {		
						////��login.txt��д���¼ʱ�䡢ip���˺�
						CTime tm; tm = CTime::GetCurrentTime();
						CString ti = tm.Format("%Y/%m/%d %X");
						ofstream outstuf;
						string msg = CT2A(ti.GetBuffer(0));
						ti.ReleaseBuffer();
						string ip = CT2A(m_ClientItem.strIp.GetBuffer(0));
						m_ClientItem.strIp.ReleaseBuffer();
						outstuf.open("login.txt", ios::app);
						outstuf << msg << "��ipΪ" << ip << "����¼���û�" << mymsgs.myClientName << "\r\n";
						outstuf.close();

						////��SendClientList���߿ͻ��˵�¼�ɹ��͵�ǰ�������ӵ��û���Ȼ��ɾ�������ӡ�
						sendMsg.type = _login;
						strncpy_s(sendMsg.myClientName, mymsgs.myClientName, sizeof(sendMsg.myClientName));
						strncpy_s(sendMsg.myClientPwd, "", sizeof(sendMsg.myClientPwd));
						//�ش�ͻ��ˣ�ע��ɹ���������sendMsg��
						m_ClientItem.pMainWind->SendClientList(&sendMsg, &m_ClientItem);////��������������Ϣ��������CChatRoomServerDlg
						m_ClientItem.pMainWind->RemoveClientFromList(m_ClientItem);
						break;
					}
					////�˺��������,��SendClientMsg���߿ͻ��˵�¼ʧ�ܣ�Ȼ��ɾ�������ӡ�
					else {  
						sendMsg.type = _fail;
						strncpy_s(sendMsg.myClientName, "", sizeof(sendMsg.myClientName));
						strncpy_s(sendMsg.myClientPwd, "", sizeof(sendMsg.myClientPwd));
						strncpy_s(sendMsg.Message, "��������ȷ���û�������", sizeof(sendMsg.Message));
						m_ClientItem.pMainWind->SendClientMsg(&sendMsg, &m_ClientItem, 0);
						Sleep(100);
						m_ClientItem.pMainWind->RemoveClientFromList(m_ClientItem);
						break;
					}
				}

				////�ͻ�������Ⱥ��
				else if (mymsgs.type == _sendMessage) {
					////�鿴�Ƿ������δ�
					string forbidText;
					int flag = 0;
					ifstream instuf("forbidText.txt", ios::in);
					instuf.seekg(0, ios::beg);
					string msg = mymsgs.Message;
					while (instuf >> forbidText) {
						if (msg.find(forbidText) != string::npos) {
							flag = 1;
							break;
						}
					}
					instuf.close();
					
					sendMsg.type =  _sendMessage;
					strncpy_s(sendMsg.myClientName, "", sizeof(sendMsg.myClientName));
					strncpy_s(sendMsg.myClientPwd, "", sizeof(sendMsg.myClientPwd));
					strncpy_s(sendMsg.Message, mymsgs.Message, sizeof(sendMsg.Message));
					////��chat.txt��¼������ip,�û�����Ⱥ����Ϣ
					ofstream outstuf;
					outstuf.open("chat.txt", ios::app);
					string ip = CT2A(m_ClientItem.strIp.GetBuffer(0));
					m_ClientItem.strIp.ReleaseBuffer();
					outstuf <<  "(ip:" << ip << ")�û�" << mymsgs.myClientName << "Ⱥ����Ϣ:"<< mymsgs.Message <<"\r\n";
					outstuf.close();

					////ShowMessage�ڷ�������Ϣ�����ʾȺ������Ϣ��SendClientMsg�����пͻ��˷���Ϣ
					CString showmsg;
					string smsg;
					if (flag) {
						////��������ʾ���д�
						smsg = mymsgs.Message;
						showmsg = smsg.c_str();
						m_ClientItem.pMainWind->ShowMessage(_T("���д�:") +showmsg);
						m_ClientItem.pMainWind->SendClientMsg(&sendMsg, &m_ClientItem, 0);
						////���ͷ��ͻ�����ʾ���д���ʾ�������ͻ����ղ�������Ϣ
						CString strMsg = _T("���������д�:");
						string msg = CT2A(strMsg.GetBuffer(0));
						strMsg.ReleaseBuffer();
						msg += forbidText;
						strncpy_s(sendMsg.Message, msg.c_str(), sizeof(sendMsg.Message));
						m_ClientItem.pMainWind->SendClientMsg(&sendMsg, &m_ClientItem, 0);
					}
					else{
						smsg = mymsgs.Message;
						showmsg = smsg.c_str();
						m_ClientItem.pMainWind->ShowMessage(showmsg);
						m_ClientItem.pMainWind->SendClientMsg(&sendMsg, &m_ClientItem, 1);
					}
}

				////�ͻ��˳ɹ���¼��������������û��ĳ�Ա�б�
				else if (mymsgs.type == _clientAdd) {
					////��ClientList�����¼����ͻ��˵��û���
					strncpy_s(m_ClientItem.myClientName, mymsgs.myClientName, sizeof(m_ClientItem.myClientName));
					m_ClientItem.pMainWind->SetClientList(&m_ClientItem, mymsgs.myClientName);
					////��������AddListMember
					sendMsg.type = _clientAdd;
					strncpy_s(sendMsg.myClientName, mymsgs.myClientName, sizeof(sendMsg.myClientName));
					strncpy_s(sendMsg.myClientPwd, "", sizeof(sendMsg.myClientPwd));
					strncpy_s(sendMsg.Message, "", sizeof(sendMsg.Message));
					m_ClientItem.pMainWind->SendClientMsg(&sendMsg, &m_ClientItem, 1);
					Sleep(100);

					////��������Ϣ����ʾ
					string loginname = mymsgs.myClientName;
					string logintext = loginname + "�����˾�����������";
					CString showmsg;
					string smsg;
					smsg = logintext;
					showmsg = smsg.c_str();
					m_ClientItem.pMainWind->ShowMessage(showmsg);

					////�����˵���Ϣ����ʾ����û���¼��
					sendMsg.type = _sendMessage;
					strncpy_s(sendMsg.myClientName, mymsgs.myClientName, sizeof(sendMsg.myClientName));
					strncpy_s(sendMsg.myClientPwd, "", sizeof(sendMsg.myClientPwd));
					strncpy_s(sendMsg.Message, logintext.c_str(), sizeof(sendMsg.Message));
					m_ClientItem.pMainWind->SendClientMsg(&sendMsg, &m_ClientItem, 1);
				}

				////�ͻ�������˽��
				else if (mymsgs.type == _privateMessage) {
					////��������ʾ������ͻ��˷�˽��
					sendMsg.type = _privateMessage;
					strncpy_s(sendMsg.myClientName, mymsgs.myClientName, sizeof(sendMsg.myClientName));
					strncpy_s(sendMsg.myClientPwd, m_ClientItem.myClientName, sizeof(sendMsg.myClientPwd));
					strncpy_s(sendMsg.Message, mymsgs.Message, sizeof(sendMsg.Message));
					CString showmsg;
					string smsg;
					smsg = mymsgs.Message;
					showmsg = smsg.c_str();
					m_ClientItem.pMainWind->ShowMessage(showmsg);
					m_ClientItem.pMainWind->SendClientMsgOTO(&sendMsg, &m_ClientItem);

					////��chat.txtд��˽�ŵ�ip���û���������
					ofstream outstuf;
					string ip = CT2A(m_ClientItem.strIp.GetBuffer(0));
					m_ClientItem.strIp.ReleaseBuffer();
					outstuf.open("chat.txt", ios::app);
					outstuf  << "(ip" << ip << ")�û�" << m_ClientItem.myClientName << "˽��" << mymsgs.myClientName <<":"<<mymsgs.Message << "\r\n";
					outstuf.close();
				}

				////�ͻ�������˽�ŵ��ļ�
				else if (mymsgs.type == _privateFileLen) {
				////��������ʾ������ͻ��˷�˽��
				sendMsg.type = _privateFileLen;
				strncpy_s(sendMsg.myClientName, mymsgs.myClientName, sizeof(sendMsg.myClientName));//���ܷ�
				strncpy_s(sendMsg.myClientPwd, m_ClientItem.myClientName, sizeof(sendMsg.myClientPwd));//���ͷ�
				strncpy_s(sendMsg.Message, mymsgs.Message, sizeof(sendMsg.Message));//����
				sendMsg.myfilesize = mymsgs.myfilesize;
				m_ClientItem.pMainWind->SendClientMsgOTO(&sendMsg, &m_ClientItem);

				////��chat.txtд��˽�ŵ�ip���û���������
				ofstream outstuf;
				string ip = CT2A(m_ClientItem.strIp.GetBuffer(0));
				m_ClientItem.strIp.ReleaseBuffer();
				outstuf.open("chat.txt", ios::app);
				outstuf << "(ip" << ip << ")�û�" << m_ClientItem.myClientName << "˽��" << mymsgs.myClientName << "ϣ�����͵��ļ�:" << mymsgs.Message << "\r\n";
				outstuf.close();
				}
				////�ͻ�������˽�ŵ��ļ�
				else if (mymsgs.type == _privateFileReply) {
				//////��������ʾ������ͻ��˷�˽���ļ�
				sendMsg.type = mymsgs.type;
				strncpy_s(sendMsg.myClientName, mymsgs.myClientName, sizeof(sendMsg.myClientName));//���ܷ�
				strncpy_s(sendMsg.myClientPwd, m_ClientItem.myClientName, sizeof(sendMsg.myClientPwd));//���ͷ�
				strncpy_s(sendMsg.Message, mymsgs.Message, sizeof(sendMsg.Message));
				sendMsg.myfilesize = mymsgs.myfilesize;
				m_ClientItem.pMainWind->SendClientMsgOTO(&sendMsg, &m_ClientItem);

				}
			}

			////���������ֹ��recv���� 0
			else { 
				////��������ʾ�����뿪��Ϣ�����пͻ��ˣ�ɾ������ͻ��˵�����
				if (m_ClientItem.myClientName != "") {
					sendMsg.type = _clientExit;
					strncpy_s(sendMsg.myClientName, m_ClientItem.myClientName, sizeof(sendMsg.myClientName));
					strncpy_s(sendMsg.myClientPwd, "", sizeof(sendMsg.myClientPwd));
					string uname = m_ClientItem.myClientName;
					string msg = uname + "�˳��˾�����������";
					strcpy_s(sendMsg.Message, sizeof(sendMsg.Message), msg.c_str());
					CString showmsg;
					showmsg = msg.c_str();
					m_ClientItem.pMainWind->ShowMessage(showmsg);
					m_ClientItem.pMainWind->RemoveClientFromList(m_ClientItem);
					m_ClientItem.pMainWind->SendClientMsg(&sendMsg, &m_ClientItem, 1);
					////��login.txtд���뿪��ʱ�䣬ip���û���
					CTime tm; tm = CTime::GetCurrentTime();
					CString ti = tm.Format("%Y/%m/%d %X");
					ofstream outstuf;
					string time = CT2A(ti.GetBuffer(0));
					ti.ReleaseBuffer();
					string ip = CT2A(m_ClientItem.strIp.GetBuffer(0));
					m_ClientItem.strIp.ReleaseBuffer();
					outstuf.open("login.txt", ios::app);
					outstuf << time << "(ip" << ip << ")�û�" << m_ClientItem.myClientName << "�˳��˾�����������"<< "\r\n";
					outstuf.close();
				}
				m_ClientItem.pMainWind->RemoveClientFromList(m_ClientItem);
				break;
			}
		}
		Sleep(500);
	}
	return true;
}

