#include <iostream>
#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32")

using namespace std;

int main()
{
    cout << "> echo-client is activated." << endl;

    WSADATA wsaData;
    SOCKET clientSocket;
    SOCKADDR_IN serverAddress;
    int PORT = 65456;   // 연결할 서버의 포트


    //winsock 라이브러리 초기화
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "> WSAStartup() failed: " << WSAGetLastError() << endl;
    }

    //TCP 소켓 생성
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    //AF_INET       : 주소체계
    //SOCK_STREAM	: TCP 전송방식


    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr);

    //커넥트
    connect(clientSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress));


    //통신 시작
    char recvData[1024];
    string sendMsg;
    while (true) {
        getline(cin, sendMsg);
        if (sendMsg == "") continue;

        //send
        send(clientSocket, sendMsg.c_str(), sendMsg.length(), 0);

        //recv
        memset(recvData, 0, 1024);
        recv(clientSocket, recvData, sizeof(recvData), 0);
        
        cout << "> received: " << recvData << endl;
        
        //종료
        if (strcmp(recvData, "quit") == 0) {
            break;
        }

    }


    closesocket(clientSocket);
    WSACleanup();

    cout << "> echo-client is de-activated." << endl;

    return 0;
}

