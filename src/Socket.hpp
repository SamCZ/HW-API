#pragma once

#include <string>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <functional>

class TCPServer
{
private:
	std::string m_Address;
	uint16_t m_Port;
	int m_Socket;
	struct sockaddr_in m_SocketAddress;
	bool m_Running;
public:
	TCPServer(std::string address, uint16_t port);
	explicit TCPServer(uint16_t port);
	~TCPServer();

	bool Start();
	void Stop();

	void Listen(std::function<std::string()> messageFnc);
};