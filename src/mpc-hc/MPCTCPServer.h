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
