#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <windows.h>
#include <string>
#include <iostream>
#include <vector>
#pragma comment(lib, "Ws2_32.lib")
#pragma warning(disable: 4996)

using namespace std;

struct Message {
	char name[20];
	char text[120];
} message;

#define PORT 666    // порт сервера
#define sHELLO "Hello, STUDENT\n"

std::vector<sockaddr_in> Connections;
char buff[1024];

void newConnect(sockaddr_in& newConnect)
{
	std::string a(inet_ntoa(newConnect.sin_addr));
	bool new_ = true;
	for (int i = 0; i < Connections.size(); i++)
	{
		if (!a.compare(std::string(inet_ntoa(Connections[i].sin_addr))))
		{
			new_ = false;
			break;
		}
	}
	if (new_)
	{
		Connections.push_back(newConnect);
	}
}

int createServer(SOCKET& Lsock)
{
	cout << "UDP DEMO CHAT-SERVER\n";
	// шаг 1 - подключение библиотеки 
	if (WSAStartup(0x202, (WSADATA*) &buff[0]))
	{
  		cout << "WSAStartup error: "<< WSAGetLastError();
  		return 1;
  	}
	Lsock = socket(AF_INET,SOCK_DGRAM,0);
	if (Lsock == INVALID_SOCKET)
	{
  		cout << "SOCKET() ERROR: " << WSAGetLastError();
  		WSACleanup();
  		return 1;
  	}
	// шаг 3 - связывание сокета с локальным адресом 
	sockaddr_in Laddr;
	Laddr.sin_family = AF_INET;
	Laddr.sin_addr.s_addr = INADDR_ANY;   // любой IP адрес
	Laddr.sin_port=htons(PORT);
	if (bind(Lsock, (sockaddr*) &Laddr, sizeof(Laddr)))
	{
		cout << "BIND ERROR:" << WSAGetLastError();
        closesocket(Lsock);
        WSACleanup();
        return 1;
    }
    return 0;
}

void sentMessageToAll(SOCKET& Lsock)
{
	for (auto it = Connections.begin(); it != Connections.end(); it++)
	{
		int code = sendto(Lsock, (char*)&message, sizeof(message), 0, (sockaddr*) &(*it), sizeof((*it)));
		if (code < 0)
		{
			cout << "client: " << inet_ntoa((*it).sin_addr) <<  " lost";
			it = Connections.erase(it);
		}
	}
}

int main()
{
	SOCKET Lsock;
	if (createServer(Lsock))
		return -1;

    // шаг 4 обработка пакетов, присланных клиентами
	while(1)
	{
		// получаем посылку
  		sockaddr_in Caddr;
  		int Caddr_size = sizeof(Caddr);
  		int bsize = recvfrom(Lsock, (char*)&message, sizeof(message), 0, (sockaddr*) &Caddr, &Caddr_size);
  		if (bsize == SOCKET_ERROR)
			cout << "RECVFROM() ERROR:" << WSAGetLastError ();
		newConnect(Caddr);
  		
  		// деаноним клиента
  		HOSTENT *hst;
  		hst = gethostbyaddr((char*) &Caddr.sin_addr, 4, AF_INET);
  		cout << "NEW DATAGRAM!\nCLIENT INFO\n\t" <<
  			((hst) ? hst->h_name : "Unknown host") << "\n\t" <<
  				inet_ntoa(Caddr.sin_addr) << "\n\t" << ntohs(Caddr.sin_port) << "\n\t" << "END INFO\n"; 
		
		// посылка датаграммы клиентам
    	sentMessageToAll(Lsock);
	}
	return 0;
}