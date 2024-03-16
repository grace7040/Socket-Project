#include <iostream>
#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32")

using namespace std;

int main()
{
    cout << "> echo-server is activated." << endl;

    WSADATA wsaData;
    SOCKET serverSocket, clientSocket;
    SOCKADDR_IN serverAddress, clientAddress;
    int PORT = 65456;


    //winsock 라이브러리 초기화
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "> WSAStartup() failed: " << WSAGetLastError() << endl;
    }

    //TCP 소켓 생성
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    //AF_INET       : 주소체계
    //SOCK_STREAM	: TCP 전송방식

    if (serverSocket == INVALID_SOCKET)
        cerr << "> socket() failed: " << WSAGetLastError() << endl;

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY); // unsigned long -> 빅 엔디안 형식으로
    serverAddress.sin_port = htons(PORT); // unsigned short -> 빅 엔디안 형식으로


    //소켓에 IP 및 포트번호 바인딩
    if (bind(serverSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
        cerr << "> bind() failed: " << WSAGetLastError() << endl;


    //리슨. 두번째 인자는 리슨 큐의 크기
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR)
        cerr << "> listen() failed: " << WSAGetLastError() << endl;


    //클라 accept. 연결 소켓 생성
    int len = sizeof(clientAddress);
    clientSocket = accept(serverSocket, (SOCKADDR*)&clientAddress, &len);
    //연결된 클라의 주소 정보를 clientAddress에 반환
    if (clientSocket == INVALID_SOCKET)
        cerr << "> accept() failed:  " << WSAGetLastError() << endl;

    char clientIP[32] = { 0 };
    inet_ntop(AF_INET, &clientAddress.sin_addr, clientIP, sizeof(clientIP));
    cout << "> client connected by IP address " << clientIP
        << " with Port number " << ntohs(clientAddress.sin_port) << endl;


    //통신 시작
    char recvData[1024];
    while (true) {
        memset(recvData, 0, 1024);
        recv(clientSocket, recvData, sizeof(recvData), 0);

        //echo
        cout << "> echoed: " << recvData << endl;
        send(clientSocket, recvData, sizeof(recvData) - 1, 0);

        //종료
        if (strcmp(recvData, "quit") == 0) {
            send(clientSocket, recvData, sizeof(recvData) - 1, 0);
            break;
        }

    }


    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();

    cout << "> echo-server is de-activated." << endl;

    return 0;
}

