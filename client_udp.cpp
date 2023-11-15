#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <string>
#include <windows.h>
#include <iostream>
#include <queue>
#include <thread>
#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable: 4996)
using namespace std;
#define PORT 666
#define SERVERADDR "127.0.0.1" // IP-адрес сервера

struct Message {
    char name[20];
    char text[120];
} get_mess, send_mess;

std::queue<Message> waitPrint;
SOCKET Connection;
sockaddr_in Daddr;
char buff[10 * 1014];

void getMessage() {
    while (1) {
        sockaddr_in SRaddr;
        int SRaddr_size = sizeof(SRaddr);
        int size = recvfrom(Connection, (char*)&get_mess, sizeof(get_mess), 0, (sockaddr*)&SRaddr, &SRaddr_size);
        if (size < 0) {
            //printf("SERVER DISCONNECT\n\n");
            //closesocket(Connection);
            //Connection = INVALID_SOCKET;
            //exit(0);
        }
        else
        {
            waitPrint.push(get_mess);
        }
    }
    closesocket(Connection);
    Connection = INVALID_SOCKET;
    return;
}

int connectToServer() {
    cout << "UDP Demo Chat Client\nMax size name 20, max size message 120\nType 'quit' to quit \n";
    // Шаг 1 - иницилизация библиотеки Winsocks
    if (WSAStartup(0x202, (WSADATA*)&buff))
    {
        cout << "WSASTARTUP ERROR: " << WSAGetLastError() << "\n";
        return 1;
    }
    // Шаг 2 - открытие сокета
    Connection = socket(AF_INET, SOCK_DGRAM, 0);
    if (Connection == INVALID_SOCKET)
    {
        cout << "SOCKET() ERROR: " << WSAGetLastError() << "\n";
        WSACleanup();
        return 1;
    }

    HOSTENT* hst;
    Daddr.sin_family = AF_INET;
    Daddr.sin_port = htons(PORT);
    // определение IP-адреса узла
    if (inet_addr(SERVERADDR))
    {
        Daddr.sin_addr.s_addr = inet_addr(SERVERADDR);
    }
    else
    {
        if (hst = gethostbyname(SERVERADDR))
        {
            Daddr.sin_addr.s_addr = ((unsigned long**)hst->h_addr_list)[0][0];
        }
        else
        {
            cout << "Unknown Host: " << WSAGetLastError() << "\n";
            closesocket(Connection);
            WSACleanup();
            return 1;
        }
    }

    return 0;
}

void printAllMessage()
{
    while (!waitPrint.empty())
    {
        Message temp = waitPrint.front();
        waitPrint.pop();
        std::cout << "[" << temp.name << "]: " << temp.text << std::endl;
    }
}

int main()
{
    if (connectToServer())
        return -1;
    cout << "you name: ";
    cin >> send_mess.name;

    std::thread tread(getMessage);
    tread.detach();

    while (1)
    {
        cin >> send_mess.text;
        if (!std::string(send_mess.text).compare(std::string("quit")))
        {
            break;
        }
        sendto(Connection, (char*)&send_mess, sizeof(send_mess), 0, (sockaddr*)&Daddr, sizeof(Daddr));
        printAllMessage();
    }
    // шаг последний - выход
    closesocket(Connection);
    WSACleanup();
    return 0;
}
