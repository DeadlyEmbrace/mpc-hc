/*
 * (C) 2013-2014 see Authors.txt
 *
 * This file is part of MPC-HC.
 *
 * MPC-HC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPC-HC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "stdafx.h"
#include "MPCTCPServer.h"
#include <codecvt>
#include "rapidjson/include/rapidjson/document.h";
#include "text.h";

#include "MainFrm.h";

using namespace rapidjson;

MPCTCPServer::MPCTCPServer(CMainFrame* mainFrame) {
    MPCTCPServer::innerMainFrame = mainFrame;
    MPCTCPServer::self = this;
    GetEventd().Connect(m_eventc, {
       MpcEvent::SWITCHED_TO_FULLSCREEN,
       MpcEvent::SWITCHED_TO_FULLSCREEN_D3D,
       MpcEvent::SWITCHED_FROM_FULLSCREEN,
        }, std::bind(&MPCTCPServer::EventCallback, this, std::placeholders::_1));
};

void MPCTCPServer::EventCallback(MpcEvent ev)
{
    std::string isFullscreen = "false";
    bool fullScreenChanged = false;
    switch (ev) {
    case MpcEvent::SWITCHED_TO_FULLSCREEN:
    case MpcEvent::SWITCHED_TO_FULLSCREEN_D3D:
        isFullscreen = "true";
        fullScreenChanged = true;
        break;
    case MpcEvent::SWITCHED_FROM_FULLSCREEN:
        isFullscreen = "false";
        fullScreenChanged = true;
        break;
    default:
        ASSERT(FALSE);
    }

    if (fullScreenChanged)
    {
        sendMessageToAllClients("Fullscreen", "{\"IsFullscreen\":" + isFullscreen + "}");
    }
}


// Handle TCP messages received from clients
void onIncomingTcpMessage(const Client& client, const char* msg, size_t size)
{
    MPCTCPServer::self->handleIncomingTcpMessage(client, msg, size);
}

/// <summary>
/// Handle incoming TCP messages
/// </summary>
/// <param name="client">Client the message was received from</param>
/// <param name="msg">Received message</param>
/// <param name="size">Size of the message</param>
void MPCTCPServer::handleIncomingTcpMessage(const Client& client, const char* msg, size_t size)
{
    Document document;
    if (!document.Parse(msg).HasParseError())
    {
        std::string command = document["Command"].GetString();
        std::string parameters = document["Parameters"].GetString();

        /*
         * TODO
         * Get current playlist
         * Insert into playlist (add index entry to JSON)
         * Delete from playlist (index value only)
         * Set currently playing index (index only)
         * Move entry in playlist (uses two index value, movefrom and moveto)
         */

        if (command == "GetCurrentStatus")
        {
            sendStateToClient(client);
            return;
        }

        if (command == "GetAPIVersion")
        {
            sendMessageToSpecificClient(client, "APIVersion", "{\"Version\": \"1.0.0\"}");
            return;
        }

        CMainFrame* frame = MPCTCPServer::innerMainFrame;
        TcpCommand* tcpCommand = new TcpCommand();
        tcpCommand->Command = command;
        tcpCommand->Parameters = parameters;
        frame->SendMessage(WM_TCP_COMMAND, reinterpret_cast<WPARAM>(tcpCommand));
    }
}

void MPCTCPServer::sendStateToClient(const Client& client)
{
    CMainFrame* frame = MPCTCPServer::innerMainFrame;

    sendPlaybackStatusToClient(client);

    std::string playbackPositionParameter = getPlaybackPositionParameter(std::lround(frame->GetPos() / 10000i64), std::lround(frame->GetDur() / 10000i64));
    sendMessageToSpecificClient(client, "Position", playbackPositionParameter);

    std::string isFullscreen = frame->m_fFullScreen ? "true" : "false";
    sendMessageToSpecificClient(client, "Fullscreen", "{\"IsFullscreen\":" + isFullscreen + "}");
}

/// <summary>
/// Send the current playback position to all clients
/// </summary>
void MPCTCPServer::sendProgressToClients(REFERENCE_TIME pos, REFERENCE_TIME dur)
{
    std::string parameter = getPlaybackPositionParameter(pos, dur);
    sendMessageToAllClients("Position", parameter);
}

/// <summary>
/// Send the current playback status to all clients
/// </summary>
void MPCTCPServer::sendPlaybackStatusToClients()
{
    std::string parameter = getPlaybackStatus();
    sendMessageToAllClients("PlaybackStateChange", parameter);
}

/// <summary>
/// Send a message to all clients as a JSON string
/// </summary>
/// <param name="server">TCPServer instance to use for sending the message</param>
/// <param name="command">Command to send to the client</param>
/// <param name="parameters">Parameters of the command to sned to the client</param>
void MPCTCPServer::sendMessageToAllClients(std::string command, std::string parameters)
{
    std::string msg = "{ \"Command\": \"" + command + "\",\"Parameters\":" + parameters + "}\r\n";
    m_tcpServer.sendToAllClients(msg.c_str(), msg.size());
}

void MPCTCPServer::sendMessageToSpecificClient(const Client& client, std::string command, std::string parameters)
{
    std::string msg = "{ \"Command\": \"" + command + "\",\"Parameters\":" + parameters + "}\r\n";
    m_tcpServer.sendToClient(client, msg.c_str(), msg.size());
}

void MPCTCPServer::sendPlaybackStatusToClient(const Client& client)
{
    std::string parameter = getPlaybackStatus();
    sendMessageToSpecificClient(client, "PlaybackStateChange", parameter);
}

std::string MPCTCPServer::getPlaybackStatus()
{
    CMainFrame* frame = MPCTCPServer::innerMainFrame;
    CString path = frame->m_wndPlaylistBar.GetCurFileName();
    CString file = frame->GetFileName();

    CString statestring;
    if (frame->GetLoadState() == MLS::LOADING)
    {
        statestring = _T("Loading");
    }
    else if (frame->GetLoadState() == MLS::CLOSED)
    {
        statestring = _T("Closed");
    }
    else
    {
        OAFilterState fs = frame->GetMediaState();
        CString state;
        state.Format(_T("%d"), fs);
        switch (fs) {
        case State_Stopped:
            statestring.LoadString(IDS_CONTROLS_STOPPED);
            break;
        case State_Paused:
            statestring.LoadString(IDS_CONTROLS_PAUSED);
            break;
        case State_Running:
            statestring.LoadString(IDS_CONTROLS_PLAYING);
            break;
        default:
            statestring = _T("N/A");
            break;
        }
    }

    std::string fullPath = CStringA(path) + "\\" + CStringA(file);
    CString escapedPath = std::regex_replace(fullPath, std::regex("\\\\"), "\\\\").c_str();
    std::string parameter = "{\"File\":\"" + CStringA(escapedPath) + "\",\"State\": \"" + CStringA(statestring) + "\"}";
    return parameter;
}

std::string MPCTCPServer::getPlaybackPositionParameter(REFERENCE_TIME pos, REFERENCE_TIME dur)
{
    CString position = NumToCString(pos / 10000i64);
    CString duration = NumToCString(dur / 10000i64);

    std::string parameter = "{\"Position\":\"" + CStringA(position) + "\",\"Duration\":\"" + CStringA(duration) + "\"}";
    return parameter;
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
    std::string msg = "{ \"Command\": \"" + command + "\",\"Parameters\":" + parameters + "}\r\n";
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
        sendMessageToClient(std::ref(server), std::ref(client), "Connection", "{\"Connected\":true}");
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
