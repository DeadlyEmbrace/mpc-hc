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

#pragma once

#include "tcp_server_client/TCPServerClient/include/tcp_server.h";
#include "tcp_server_client/TCPServerClient/include/pipe_ret_t.h";
#include "mplayerc.h";

class CMainFrame;

class MPCTCPServer {

public:
    inline static CMainFrame* innerMainFrame;
    inline static MPCTCPServer* self;
    EventClient m_eventc;

private:
    // TCP Server and observer
    TcpServer m_tcpServer;
    server_observer_t m_observer;
    void sendMessageToAllClients(std::string command, std::string parameters);

    /// <summary>
    /// Sends a message to a specific client
    /// </summary>
    /// <param name="client">The client to send the message to</param>
    /// <param name="command">The command to send to the client</param>
    /// <param name="parameters">The parameter of the command to send to the client</param>
    void sendMessageToSpecificClient(const Client& client, std::string command, std::string parameters);

    /// <summary>
    /// Retrieve the playback status as a parameter string that can be sent to client(s)
    /// </summary>
    /// <returns>Playback status as a parameter string</returns>
    std::string getPlaybackStatus();

    /// <summary>
    /// Convert the playback position and duration to a parameter string that can be send to client(s)
    /// </summary>
    /// <param name="pos">Current playback position</param>
    /// <param name="dur">Current file`s duration</param>
    /// <returns>Playback position and duration as a parameter string</returns>
    std::string getPlaybackPositionParameter(REFERENCE_TIME pos, REFERENCE_TIME dur);

    /// <summary>
    /// Send the current playback status to a specific client
    /// </summary>
    /// <param name="client">Client to send the playback status to</param>
    void sendPlaybackStatusToClient(const Client& client);

    /// <summary>
    /// Send the current player status to a client.
    /// This will simply invoke all of the status details responses one by one
    /// </summary>
    /// <param name="client">Client to send the status to</param>
    void sendStateToClient(const Client& client);

    /// <summary>
    /// Handle event callbacks
    /// </summary>
    /// <param name="ev">Event to handle</param>
    void EventCallback(MpcEvent ev);

public:
    MPCTCPServer(CMainFrame* mainFrame);
    void handleIncomingTcpMessage(const Client& client, const char* msg, size_t size);
    void startTcpServer(int nPort);
    void sendProgressToClients(REFERENCE_TIME pos, REFERENCE_TIME dur);
    void sendPlaybackStatusToClients();
};

struct TcpCommand
{
    std::string Command;
    std::string Parameters;
};
