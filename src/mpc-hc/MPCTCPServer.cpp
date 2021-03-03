#include "stdafx.h"
#include "MPCTCPServer.h"
#include <codecvt>
#include "rapidjson/include/rapidjson/document.h";

#include "MainFrm.h";

using namespace rapidjson;

MPCTCPServer::MPCTCPServer(CMainFrame* mainFrame) {
    MPCTCPServer::innerMainFrame = mainFrame;
    MPCTCPServer::self = this;
};

// Handle TCP messages received from clients
void onIncomingTcpMessage(const Client& client, const char* msg, size_t size)
{
    MPCTCPServer::self->handleIncomingTcpMessage(client, msg, size);
}

void MPCTCPServer::handleIncomingTcpMessage(const Client& client, const char* msg, size_t size)
{
    Document document;
    if (!document.Parse(msg).HasParseError())
    {
        std::string command = document["Command"].GetString();
        std::string parameters = document["Parameters"].GetString();

        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        std::wstring wide = converter.from_bytes(parameters);
        LPCWSTR result = wide.c_str();

        if (command == "OpenFile")
        {
            CAtlList<CString> fns;
            fns.AddHead(result);
            CMainFrame* frame = MPCTCPServer::innerMainFrame;
            frame->m_wndPlaylistBar.Open(fns, false);
            frame->OpenCurPlaylistItem();
        }
    }
}

/// <summary>
/// Send a message to a client as a JSON string
/// </summary>
/// <param name="server">TCPServer instance to use for sending the message</param>
/// <param name="client">Client to which the message should be send</param>
/// <param name="command">Command to send to the client</param>
/// <param name="parameters">Parameters of the command to sned to the client</param>
void sendMessageToClient(TcpServer& server, Client& client, std::string command, std::string parameters)
{
    std::string msg = "{ \"Command\": \"" + command + "\",\"Parameters\":" + parameters + "\"}\r\n";
    server.sendToClient(client, msg.c_str(), msg.size());
}

/// <summary>
/// Wait for clients to connect to the TCP Server
/// </summary>
/// <param name="server">The server instance to use for accepting clients</param>
void waitForClients(TcpServer& server)
{
    while (1) {
        Client client = server.acceptClient(0);
        sendMessageToClient(std::ref(server), std::ref(client), "Greeting", "Hello client");
        Sleep(1);
    }
}

/// <summary>
/// Start the TCP server
/// </summary>
/// <param name="nPort">Port on which the server should be start</param>
void MPCTCPServer::startTcpServer(int nPort)
{
    pipe_ret_t startRet = m_tcpServer.start(nPort + 1);
    if (startRet.success) {
        std::cout << "Server setup succeeded" << std::endl;
    }
    else {
        std::cout << "Server setup failed: " << startRet.msg << std::endl;
    }
    m_observer.incoming_packet_func = onIncomingTcpMessage;
    m_observer.disconnected_func = nullptr;
    m_observer.wantedIp = "127.0.0.1"; // TODO - This should be user configurable
    m_tcpServer.subscribe(m_observer);
    std::thread t1(waitForClients, std::ref(m_tcpServer));
    t1.detach();
}
