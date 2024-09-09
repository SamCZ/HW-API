#include "Socket.hpp"
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <ostream>
#include <sstream>
#include <functional>

TCPServer::TCPServer(std::string address, uint16_t port)
	: m_Address(std::move(address))
	, m_Port(port)
	, m_SocketAddress()
	, m_Running()
{

}

TCPServer::TCPServer(uint16_t port)
	: m_Address()
	, m_Port(port)
	, m_SocketAddress()
	, m_Running()
{

}

bool TCPServer::Start()
{
	m_Socket = socket(AF_INET, SOCK_STREAM, 0);

	if (m_Socket < 0)
	{
		std::cerr << "Could not initialize socket !" << std::endl;
		return false;
	}

	m_SocketAddress.sin_family = AF_INET;
	m_SocketAddress.sin_port = htons(m_Port);

	if (m_Address.length() == 0)
	{
		m_SocketAddress.sin_addr.s_addr = INADDR_ANY;
	}
	else
	{
		m_SocketAddress.sin_addr.s_addr = inet_addr(m_Address.c_str());
	}

	if (bind(m_Socket, (sockaddr*)&m_SocketAddress, sizeof(m_SocketAddress)) < 0)
	{
		std::cerr << "Could not bind socket to " << m_Address << ":" << m_Port << " !" << std::endl;

		return false;
	}

	return true;
}

void TCPServer::Stop()
{
	m_Running = false;
}

TCPServer::~TCPServer()
{

}

const int BUFFER_SIZE = 30720;

void TCPServer::Listen(std::function<std::string()> messageFnc)
{
	if (listen(m_Socket, 20) < 0)
	{
		std::cerr << "Socket listen failed" << std::endl;
		return;
	}

	m_Running = true;
	int bytesReceived;

	while (m_Running)
	{
		uint32_t addrLen = sizeof(m_SocketAddress);
		int client = accept(m_Socket, (sockaddr*)&m_SocketAddress, &addrLen);

		if (client < 0)
		{
			std::cerr << "Server failed to accept incoming connection from ADDRESS: " << inet_ntoa(m_SocketAddress.sin_addr) << "; PORT: " << ntohs(m_SocketAddress.sin_port);
			continue;
		}

		char buffer[BUFFER_SIZE] = {0};
		bytesReceived = recv(client, buffer, BUFFER_SIZE, 0);
		if (bytesReceived < 0)
		{
			std::cerr << "Failed to receive bytes from client socket connection" << std::endl;
			continue;
		}

		std::string content = messageFnc();

		std::stringstream ss;
		ss << "HTTP/1.1 200 OK\nContent-Type: application/json\nContent-Length: " << content.size() << "\n\n"<< content.c_str();

		std::string response = ss.str();

		std::cout << response << std::endl;

		int bytesSent;
		long totalBytesSent = 0;

		while (totalBytesSent < response.size())
		{
			bytesSent = send(client, response.c_str(), response.size(), 0);
			if (bytesSent < 0)
			{
				break;
			}
			totalBytesSent += bytesSent;
		}

		close(client);
	}

	if (m_Socket >= 0)
	{
		close(m_Socket);
		m_Socket = -1;
	}
}