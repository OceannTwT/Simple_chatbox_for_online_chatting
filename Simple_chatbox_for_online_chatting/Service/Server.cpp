#include "pch.h"
//#include "stdafx.h"
#include "chat.h"
#include "ChatRoom_ServerDlg.h"
#include "iostream"
#include "fstream"
#define MAX_BUF_SIZE 2052


BOOL SOCKET_Select(SOCKET hSocket, int nTimeOut, BOOL bRead)////非阻塞等待
{
	fd_set fdset;
	timeval tv;
	FD_ZERO(&fdset); //清除套接字集合变量
	FD_SET(hSocket, &fdset); //把套接字 hSocket 添加到 fdset集合
	nTimeOut = nTimeOut > 1000 ? 1000 : nTimeOut;
	tv.tv_sec = 0;
	tv.tv_usec = nTimeOut;
	int iRet = 0;
	if (bRead) {////如果是可读的接口，那如果在tv时间内没读到对方的消息，就结束，不阻塞。
		iRet = select(0, &fdset, NULL, NULL, &tv);
	}
	else {////如果是可写的
		iRet = select(0, NULL, &fdset, NULL, &tv);
	}
	if (iRet <= 0) {////在tv时间内没完成
		return FALSE;
	}
	else if (FD_ISSET(hSocket, &fdset)) {////在tv时间内完成了
		return TRUE;
	}
	return FALSE; 
}

DWORD WINAPI ListenThreadFunc(LPVOID pParam) { //pParam是服务器窗口
	CChatRoomServerDlg* pChatRoom = (CChatRoomServerDlg*)pParam;////在CChatRoom_ServerDlg.h自定义
	
	ASSERT(pChatRoom != NULL);
	////1.建立流式套接字socket，返回套接字号在pChatRoom->m_ListenSocket
	pChatRoom->m_ListenSocket = socket(AF_INET, SOCK_STREAM, 0); 
	if (pChatRoom->m_ListenSocket == INVALID_SOCKET) {
		AfxMessageBox(_T("SOCKET建立失败"));
		goto err;
	}
	////建立流式套接字socket，返回套接字号在sockSrv
	SOCKET sockSrv = socket(AF_INET, SOCK_STREAM, 0);
	if (sockSrv == INVALID_SOCKET) {
		AfxMessageBox(_T("SOCKET建立失败"));
		goto err;
	}
	////2.绑定套接字。把套接字sockSrv与 IP 地址addrSrv在端口 5567 绑定
	SOCKADDR_IN addrSrv;
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(short(5567));
	inet_pton(AF_INET, "192.168.43.15", &addrSrv.sin_addr);////IP地址转换函数,将点分文本的IP地址转换为二进制网络字节序”的IP地址
	
	if (bind(sockSrv, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR)) == SOCKET_ERROR) {
		AfxMessageBox(_T("socket绑定失败"));
		goto err;
	}
	////3.套接字 sockSrv 置于监听状态，处理客户请求队列最大为 5。如果客户端这是调用connect()发出连接请求，服务器就会收到这个请求。
	if (listen(sockSrv, 5) == SOCKET_ERROR) {
		AfxMessageBox(_T("监听失败"));
		goto err;
	}
	
		
	////服务器显示
	
	pChatRoom->ShowMessage(_T("服务器开启成功"));
	
	while (1) {
		if (SOCKET_Select(sockSrv, 100, TRUE)) {
			SOCKADDR_IN addrClient;
			int len = sizeof(SOCKADDR);
			////4.接受连接，等待客户端的连接。得到新的连接字sockConn。
			////服务器进程创建一个新的套接字 sockConn 来处理监听套接字 sockSrv 接受的一
			////个客户连接请求，远程客户进程的套接字地址放入 addrClient 存储。
			////accept接收请求后，连接就建立好了。之后就可以开始网络I/O操作。
			SOCKET sockConn = accept(sockSrv, (SOCKADDR*)&addrClient, &len);
			if (sockConn == INVALID_SOCKET) {
				AfxMessageBox(_T("1！"));
				continue;
			}
			////创建一个ClientStruct的对象client来记录一个客户端的信息
			ClientStruct client;
			client.Socket = sockConn;
			char str[INET_ADDRSTRLEN];
			client.strIp = inet_ntop(AF_INET, &addrClient.sin_addr, str, sizeof(str));////客户端的IP：将数值格式addrClient.sin_addr 转化为 点分十进制的ip地址格式str
			client.pMainWind = pChatRoom;
			pChatRoom->m_ClientList.push_back(client);////新客户端加入pChatRoom->m_ClientList链表
			////创建一个ClientThreadCre线程与client客户端通信
			client.hThread = CreateThread(NULL, 0, ClientThreadCre, &(pChatRoom->m_ClientList.back()), CREATE_SUSPENDED, NULL);
			pChatRoom->m_ClientList.back().hThread = client.hThread;
			ResumeThread(client.hThread);////线程调度，让该线程不被挂起
			
			Sleep(100);
		}
		
	}
err:////如果前面某步失败了
	closesocket(pChatRoom->m_ListenSocket);
	return true;
}

DWORD WINAPI ClientThreadCre(LPVOID pParam) {
	////创建一个MyMsgStruct的对象sendMsg，用于存储即将要向客户端“发出”的消息。在chat.h自定义。
	MyMsgStruct sendMsg;
	sendMsg.clientnum = 0;////客户端当前在线成员数为0
	////创建一个ClientStruct的对象m_ClientItem来记录当前这一个客户端的信息
	ClientStruct m_ClientItem = *(ClientStruct*)pParam;
	strncpy_s(m_ClientItem.myClientName, "", sizeof(m_ClientItem.myClientName));////空名字
	while (1) {
		if (SOCKET_Select(m_ClientItem.Socket, 100, TRUE)) {////非阻塞等待，如果m_ClientItem.Socket成功读取到客户端发来的内容
			////就创建一个数组szBUF，存储收到的消息
			TCHAR szBUF[MAX_BUF_SIZE + 1] = { 0 };
			////数组szBUF，存储m_ClientItem.Socket收到的消息
			int iRet = recv(m_ClientItem.Socket, (char*)&szBUF, MAX_BUF_SIZE+1, 0);
			Sleep(100);
			////如果成功recv的话
			if (iRet > 0) {
				////创建一个MyMsgStruct的对象mymsgs，用于存储 从客户端“收到”的消息。在chat.h自定义。
				MyMsgStruct mymsgs;
				memcpy(&mymsgs, szBUF, sizeof(MyMsgStruct));

				////客户端请求注册
				if (mymsgs.type == _register) {
					string Name;
					string Password;
					ifstream instuf("data.txt", ios::in);////读文件data.txt，存储在ifstream的对象instuf中
					sendMsg.type = _register;
					strncpy_s(sendMsg.myClientName, "", sizeof(sendMsg.myClientName));
					strncpy_s(sendMsg.myClientPwd, "", sizeof(sendMsg.myClientPwd));
					////从instuf中循环获取已有的 Name 和 Password
					if (instuf) {
						while (instuf >> Name >> Password) {
							////如果注册新名字重名了                                                                                         
							if (Name == mymsgs.myClientName) {
								strncpy_s(sendMsg.Message, "该用户名存在，请直接登录", sizeof(sendMsg.Message));
								m_ClientItem.pMainWind->SendClientMsg(&sendMsg, &m_ClientItem,0);////函数定义在CChatRoom_ServerDlg，向目的用户m_ClientItem发送sendMsg
								Sleep(100);
								m_ClientItem.pMainWind->RemoveClientFromList(m_ClientItem);////从当前在聊天室的用户列表中删除该用户信息
								instuf.close();
								return true;
							}
						}
						instuf.close();
					}

					////如果注册新名字没重名，把新名字密码写入data.txt,把注册日期、ip、账户写入register.txt
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
					outstuf << msg << " ，ip为" << ip << "注册了用户" << mymsgs.myClientName << "\r\n";
					outstuf.close();

					////告诉客户端注册成功
					strncpy_s(sendMsg.Message, "成功完成注册", sizeof(sendMsg.Message));
					m_ClientItem.pMainWind->SendClientMsg(&sendMsg, &m_ClientItem,0);
					Sleep(100);
					m_ClientItem.pMainWind->RemoveClientFromList(m_ClientItem);
					break;
				}

				////客户端请求登陆
				else if (mymsgs.type == _login) {
					////读data.txt，判断账户密码是否正确，正确则flag=1
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
					////如果账号密码正确
					if (flag) {		
						////在login.txt中写入登录时间、ip、账号
						CTime tm; tm = CTime::GetCurrentTime();
						CString ti = tm.Format("%Y/%m/%d %X");
						ofstream outstuf;
						string msg = CT2A(ti.GetBuffer(0));
						ti.ReleaseBuffer();
						string ip = CT2A(m_ClientItem.strIp.GetBuffer(0));
						m_ClientItem.strIp.ReleaseBuffer();
						outstuf.open("login.txt", ios::app);
						outstuf << msg << "，ip为" << ip << "，登录了用户" << mymsgs.myClientName << "\r\n";
						outstuf.close();

						////用SendClientList告诉客户端登录成功和当前所有连接的用户，然后删除该连接。
						sendMsg.type = _login;
						strncpy_s(sendMsg.myClientName, mymsgs.myClientName, sizeof(sendMsg.myClientName));
						strncpy_s(sendMsg.myClientPwd, "", sizeof(sendMsg.myClientPwd));
						//回答客户端：注册成功，并更新sendMsg的
						m_ClientItem.pMainWind->SendClientList(&sendMsg, &m_ClientItem);////发送所有连接信息。定义在CChatRoomServerDlg
						m_ClientItem.pMainWind->RemoveClientFromList(m_ClientItem);
						break;
					}
					////账号密码错误,用SendClientMsg告诉客户端登录失败，然后删除该连接。
					else {  
						sendMsg.type = _fail;
						strncpy_s(sendMsg.myClientName, "", sizeof(sendMsg.myClientName));
						strncpy_s(sendMsg.myClientPwd, "", sizeof(sendMsg.myClientPwd));
						strncpy_s(sendMsg.Message, "请输入正确的用户名密码", sizeof(sendMsg.Message));
						m_ClientItem.pMainWind->SendClientMsg(&sendMsg, &m_ClientItem, 0);
						Sleep(100);
						m_ClientItem.pMainWind->RemoveClientFromList(m_ClientItem);
						break;
					}
				}

				////客户端请求群发
				else if (mymsgs.type == _sendMessage) {
					////查看是否含有屏蔽词
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
					////在chat.txt记录发信者ip,用户名，群发消息
					ofstream outstuf;
					outstuf.open("chat.txt", ios::app);
					string ip = CT2A(m_ClientItem.strIp.GetBuffer(0));
					m_ClientItem.strIp.ReleaseBuffer();
					outstuf <<  "(ip:" << ip << ")用户" << mymsgs.myClientName << "群发消息:"<< mymsgs.Message <<"\r\n";
					outstuf.close();

					////ShowMessage在服务器消息框里，显示群发的消息。SendClientMsg向所有客户端发消息
					CString showmsg;
					string smsg;
					if (flag) {
						////服务器显示敏感词
						smsg = mymsgs.Message;
						showmsg = smsg.c_str();
						m_ClientItem.pMainWind->ShowMessage(_T("敏感词:") +showmsg);
						m_ClientItem.pMainWind->SendClientMsg(&sendMsg, &m_ClientItem, 0);
						////发送方客户端显示敏感词提示，其他客户端收不到该消息
						CString strMsg = _T("请勿发送敏感词:");
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

				////客户端成功登录后请求加入其它用户的成员列表
				else if (mymsgs.type == _clientAdd) {
					////在ClientList里面记录这个客户端的用户名
					strncpy_s(m_ClientItem.myClientName, mymsgs.myClientName, sizeof(m_ClientItem.myClientName));
					m_ClientItem.pMainWind->SetClientList(&m_ClientItem, mymsgs.myClientName);
					////让所有人AddListMember
					sendMsg.type = _clientAdd;
					strncpy_s(sendMsg.myClientName, mymsgs.myClientName, sizeof(sendMsg.myClientName));
					strncpy_s(sendMsg.myClientPwd, "", sizeof(sendMsg.myClientPwd));
					strncpy_s(sendMsg.Message, "", sizeof(sendMsg.Message));
					m_ClientItem.pMainWind->SendClientMsg(&sendMsg, &m_ClientItem, 1);
					Sleep(100);

					////服务器消息框显示
					string loginname = mymsgs.myClientName;
					string logintext = loginname + "加入了局域网聊天室";
					CString showmsg;
					string smsg;
					smsg = logintext;
					showmsg = smsg.c_str();
					m_ClientItem.pMainWind->ShowMessage(showmsg);

					////所有人的消息框显示这个用户登录了
					sendMsg.type = _sendMessage;
					strncpy_s(sendMsg.myClientName, mymsgs.myClientName, sizeof(sendMsg.myClientName));
					strncpy_s(sendMsg.myClientPwd, "", sizeof(sendMsg.myClientPwd));
					strncpy_s(sendMsg.Message, logintext.c_str(), sizeof(sendMsg.Message));
					m_ClientItem.pMainWind->SendClientMsg(&sendMsg, &m_ClientItem, 1);
				}

				////客户端请求发私信
				else if (mymsgs.type == _privateMessage) {
					////服务器显示，并向客户端发私信
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

					////在chat.txt写入私信的ip，用户名，内容
					ofstream outstuf;
					string ip = CT2A(m_ClientItem.strIp.GetBuffer(0));
					m_ClientItem.strIp.ReleaseBuffer();
					outstuf.open("chat.txt", ios::app);
					outstuf  << "(ip" << ip << ")用户" << m_ClientItem.myClientName << "私聊" << mymsgs.myClientName <<":"<<mymsgs.Message << "\r\n";
					outstuf.close();
				}

				////客户端请求发私信的文件
				else if (mymsgs.type == _privateFileLen) {
				////服务器显示，并向客户端发私信
				sendMsg.type = _privateFileLen;
				strncpy_s(sendMsg.myClientName, mymsgs.myClientName, sizeof(sendMsg.myClientName));//接受方
				strncpy_s(sendMsg.myClientPwd, m_ClientItem.myClientName, sizeof(sendMsg.myClientPwd));//发送方
				strncpy_s(sendMsg.Message, mymsgs.Message, sizeof(sendMsg.Message));//问题
				sendMsg.myfilesize = mymsgs.myfilesize;
				m_ClientItem.pMainWind->SendClientMsgOTO(&sendMsg, &m_ClientItem);

				////在chat.txt写入私信的ip，用户名，内容
				ofstream outstuf;
				string ip = CT2A(m_ClientItem.strIp.GetBuffer(0));
				m_ClientItem.strIp.ReleaseBuffer();
				outstuf.open("chat.txt", ios::app);
				outstuf << "(ip" << ip << ")用户" << m_ClientItem.myClientName << "私聊" << mymsgs.myClientName << "希望发送的文件:" << mymsgs.Message << "\r\n";
				outstuf.close();
				}
				////客户端请求发私信的文件
				else if (mymsgs.type == _privateFileReply) {
				//////服务器显示，并向客户端发私信文件
				sendMsg.type = mymsgs.type;
				strncpy_s(sendMsg.myClientName, mymsgs.myClientName, sizeof(sendMsg.myClientName));//接受方
				strncpy_s(sendMsg.myClientPwd, m_ClientItem.myClientName, sizeof(sendMsg.myClientPwd));//发送方
				strncpy_s(sendMsg.Message, mymsgs.Message, sizeof(sendMsg.Message));
				sendMsg.myfilesize = mymsgs.myfilesize;
				m_ClientItem.pMainWind->SendClientMsgOTO(&sendMsg, &m_ClientItem);

				}
			}

			////如果连接中止，recv返回 0
			else { 
				////服务器显示，发离开消息给所有客户端，删除这个客户端的连接
				if (m_ClientItem.myClientName != "") {
					sendMsg.type = _clientExit;
					strncpy_s(sendMsg.myClientName, m_ClientItem.myClientName, sizeof(sendMsg.myClientName));
					strncpy_s(sendMsg.myClientPwd, "", sizeof(sendMsg.myClientPwd));
					string uname = m_ClientItem.myClientName;
					string msg = uname + "退出了局域网聊天室";
					strcpy_s(sendMsg.Message, sizeof(sendMsg.Message), msg.c_str());
					CString showmsg;
					showmsg = msg.c_str();
					m_ClientItem.pMainWind->ShowMessage(showmsg);
					m_ClientItem.pMainWind->RemoveClientFromList(m_ClientItem);
					m_ClientItem.pMainWind->SendClientMsg(&sendMsg, &m_ClientItem, 1);
					////在login.txt写入离开的时间，ip，用户名
					CTime tm; tm = CTime::GetCurrentTime();
					CString ti = tm.Format("%Y/%m/%d %X");
					ofstream outstuf;
					string time = CT2A(ti.GetBuffer(0));
					ti.ReleaseBuffer();
					string ip = CT2A(m_ClientItem.strIp.GetBuffer(0));
					m_ClientItem.strIp.ReleaseBuffer();
					outstuf.open("login.txt", ios::app);
					outstuf << time << "(ip" << ip << ")用户" << m_ClientItem.myClientName << "退出了局域网聊天室"<< "\r\n";
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

