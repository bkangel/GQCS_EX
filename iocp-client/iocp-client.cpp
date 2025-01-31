// iocp-client.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.

#include "stdafx.h"
// winsock2 사용을 위해 아래 코멘트 추가
#pragma comment(lib, "ws2_32.lib")
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <process.h>
using namespace std;

#define	MAX_BUFFER		1024
#define SERVER_PORT		8000
#define SERVER_IP		"127.0.0.1"
//
//struct stSOCKETINFO
//{
//	WSAOVERLAPPED	overlapped;
//	WSABUF			dataBuf;
//	SOCKET			socket;
//	char			messageBuffer[MAX_BUFFER];
//	int				recvBytes;
//	int				sendBytes;
//};

#include <random>
#include <chrono>

std::mt19937_64 genMT(time(0));
std::uniform_int_distribution<__int64> uniformDist(1, 45);

unsigned int WINAPI mainworker(LPVOID p)
{
	// TCP 소켓 생성
	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (clientSocket == INVALID_SOCKET) {
		std::cout << "Error : " << WSAGetLastError() << std::endl;
		return false;
	}

	std::cout << "socket initialize success." << std::endl;

	// 접속할 서버 정보를 저장할 구조체
	SOCKADDR_IN stServerAddr;

	char	szOutMsg[MAX_BUFFER];
	char	sz_socketbuf_[MAX_BUFFER];
	stServerAddr.sin_family = AF_INET;
	// 접속할 서버 포트 및 IP
	stServerAddr.sin_port = htons(SERVER_PORT);
	stServerAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
	int nRet;
	nRet = connect(clientSocket, (sockaddr*)&stServerAddr, sizeof(sockaddr));
	if (nRet == SOCKET_ERROR) {
		std::cout << "Error : " << WSAGetLastError() << std::endl;
		return false;
	}

	std::cout << "Connection success..." << std::endl;
	while (true) {
		//std::cout << ">>";
		sprintf_s(szOutMsg, "%llx: %lld", clientSocket, uniformDist(genMT));
		//Sleep((uniformDist(genMT) % 30) + 1);
		//std::cin >> szOutMsg;
		if (_strcmpi(szOutMsg, "quit") == 0) break;

		int nSendLen = send(clientSocket, szOutMsg, strlen(szOutMsg), 0);

		if (nSendLen == -1) {
			std::cout << "Error : " << WSAGetLastError() << std::endl;
			return false;
		}

		//std::cout << "Message sended : bytes[" << nSendLen << "], message : [" <<	szOutMsg << "]" << std::endl;

		int nRecvLen = recv(clientSocket, sz_socketbuf_, 1024, 0);
		if (nRecvLen == 0) {
			std::cout << "Client connection has been closed" << std::endl;
			closesocket(clientSocket);
			return false;
		}
		else if (nRecvLen == -1) {
			std::cout << "Error : " << WSAGetLastError() << std::endl;
			closesocket(clientSocket);
			return false;
		}

		sz_socketbuf_[nRecvLen] = NULL;
		//std::cout << "Message received : bytes[" << nRecvLen << "], message : [" <<	sz_socketbuf_ << "]" << std::endl;
	}

	closesocket(clientSocket);
	std::cout << "Client has been terminated..." << std::endl;

    return 0;
}

int main()
{
	WSADATA wsaData;
	// 윈속 버전을 2.2로 초기화
	int nRet = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (nRet != 0) {
		std::cout << "Error : " << WSAGetLastError() << std::endl;
		return false;
	}

	unsigned int threadId;
	// 시스템 정보 가져옴
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	printf_s("[INFO] CPU 갯수 : %d\n", sysInfo.dwNumberOfProcessors);
	// 적절한 작업 스레드의 갯수는 (CPU * 2) + 1
	int nThreadCnt = 48;// sysInfo.dwNumberOfProcessors * 2;

	// thread handler 선언
	HANDLE* m_pWorkerHandle = new HANDLE[nThreadCnt];
	// thread 생성
	for (int i = 0; i < nThreadCnt; i++)
	{
		m_pWorkerHandle[i] = (HANDLE *)_beginthreadex(
			NULL, 0, &mainworker, nullptr, CREATE_SUSPENDED, &threadId
		);
		if (m_pWorkerHandle[i] == NULL)
		{
			printf_s("[ERROR] Worker Thread 생성 실패\n");
			return false;
		}
		ResumeThread(m_pWorkerHandle[i]);
	}
	printf_s("[INFO] Worker Thread 시작...\n");
	getchar();
}



