#include <iostream>
#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <thread>
#include <vector>
#pragma comment(lib, "ws2_32")

using namespace std;

int PORT = 65456;
int threadCnt = 1;
bool terminateServer = false;

void CreateServerSocket(SOCKET& serverSocket, SOCKADDR_IN& serverAddress, int PORT) {
    //TCP 소켓 생성
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    //AF_INET       : 주소체계
    //SOCK_STREAM	: TCP 전송방식

    if (serverSocket == INVALID_SOCKET)
        cerr << "> socket() failed: " << WSAGetLastError() << endl;

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY); // unsigned long -> 빅 엔디안 형식으로
    serverAddress.sin_port = htons(PORT); // unsigned short -> 빅 엔디안 형식으로

}


void BindAndListen(SOCKET& serverSocket, SOCKADDR_IN& serverAddress, SOCKET& clientSocket, SOCKADDR_IN& clientAddress) {
    //소켓에 IP 및 포트번호 바인딩
    if (bind(serverSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
        cerr << "> bind() failed: " << WSAGetLastError() << endl;


    //리슨. 두번째 인자는 리슨 큐의 크기
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR)
        cerr << "> listen() failed: " << WSAGetLastError() << endl;

}


void ServeForever(int threadNum, SOCKET clientSocket, SOCKADDR_IN clientAddress) {
    char clientIP[32] = { 0 };
    inet_ntop(AF_INET, &clientAddress.sin_addr, clientIP, sizeof(clientIP));
    cout << "> [Thread " << threadNum << "] " << "client connected by IP address " << clientIP
        << " with Port number " << ntohs(clientAddress.sin_port) << endl;


    //통신 시작
    char recvData[1024];
    while (true) {
        memset(recvData, 0, 1024);
        recv(clientSocket, recvData, sizeof(recvData), 0);

        //echo
        cout << "> [Thread "<<threadNum<<"] echoed: " << recvData << endl;
        send(clientSocket, recvData, sizeof(recvData) - 1, 0);

        //종료
        if (strcmp(recvData, "quit") == 0) {
            send(clientSocket, recvData, sizeof(recvData) - 1, 0);
            break;
        }
    }

    threadCnt--;
    cout << "> [Thread " << threadNum << "] terminated."<< endl;
    closesocket(clientSocket);
}

void ServerThread() {
    SOCKET serverSocket, clientSocket;
    SOCKADDR_IN serverAddress, clientAddress;
    
    CreateServerSocket(serverSocket, serverAddress, PORT);
    BindAndListen(serverSocket, serverAddress, clientSocket, clientAddress);

    vector<thread> client_threads;

    while (!terminateServer) {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(serverSocket, &readSet);

        timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int result = select(0, &readSet, nullptr, nullptr, &timeout);

        if (result == SOCKET_ERROR) {
            cerr << "select() failed: " << WSAGetLastError() << endl;
            break;
        }
        else if (result == 0) {
            continue;
        }

        if (FD_ISSET(serverSocket, &readSet)) {
            if (terminateServer) break;

            //클라 accept. 연결 소켓 생성
            int len = sizeof(clientAddress);
            clientSocket = accept(serverSocket, (SOCKADDR*)&clientAddress, &len);
            //연결된 클라의 주소 정보를 clientAddress에 반환
            if (clientSocket == INVALID_SOCKET)
                cerr << "> accept() failed: " << WSAGetLastError() << endl;

            client_threads.emplace_back(&ServeForever, threadCnt++, clientSocket, clientAddress);
        }
    }

    for (auto& thread : client_threads) {
        thread.join();
    }

    closesocket(serverSocket);
}

int main()
{
    cout << "> echo-server is activated." << endl;

    WSADATA wsaData;
    //winsock 라이브러리 초기화
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "> WSAStartup() failed." << WSAGetLastError() << endl;
    }

    
    //멀티스레딩
    thread server_thread(ServerThread);
    while (true) {
        std::string msg;
        std::cout << "> ";
        std::getline(std::cin, msg);
        if (msg == "quit") {
            if (threadCnt == 1) {
                std::cout << "> stop procedure started." << std::endl;
                terminateServer = true;
                break;
            }
            else {
                std::cout << "> active threads are remaining: " << threadCnt - 1 << " threads" << std::endl;
            }
        }
    }

    
    WSACleanup();
    server_thread.join();

    cout << "> echo-server is de-activated." << endl;

    return 0;
}

