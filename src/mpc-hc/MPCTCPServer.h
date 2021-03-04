#pragma once

#include "../tcp_server_client/TCPServerClient/include/tcp_server.h";

class CMainFrame;

class MPCTCPServer {
public:
    inline static CMainFrame* innerMainFrame;
    inline static MPCTCPServer* self;

private:
    // TCP Server and observer
    TcpServer m_tcpServer;
    server_observer_t m_observer;
    void sendMessageToAllClients(std::string command, std::string parameters);

public:
    MPCTCPServer(CMainFrame* mainFrame);
    void handleIncomingTcpMessage(const Client& client, const char* msg, size_t size);
    void startTcpServer(int nPort);
    void sendStateToClients();
};

struct TcpCommand
{
    std::string Command;
    std::string Parameters;
};
