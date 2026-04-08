#include "tcp_server.h"
#include <algorithm>
#include <mstcpip.h>

thread_local TcpClientConnection* g_respondingTcpClient = nullptr;

TcpServer::TcpServer() {
  WSADATA wsaData;
  WSAStartup(MAKEWORD(2, 2), &wsaData);
}

TcpServer::~TcpServer() {
  Stop();
}

bool TcpServer::Start(int port, const std::string& pin, MessageHandler onMessage) {
  if (m_running.load()) return false;
  m_port = port;
  m_pin = pin;
  m_onMessage = std::move(onMessage);

  m_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (m_listenSocket == INVALID_SOCKET) return false;

  int opt = 1;
  setsockopt(m_listenSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

  u_long mode = 1;
  ioctlsocket(m_listenSocket, FIONBIO, &mode);

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons((u_short)port);

  if (bind(m_listenSocket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
    closesocket(m_listenSocket);
    m_listenSocket = INVALID_SOCKET;
    return false;
  }

  if (listen(m_listenSocket, SOMAXCONN) == SOCKET_ERROR) {
    closesocket(m_listenSocket);
    m_listenSocket = INVALID_SOCKET;
    return false;
  }

  m_running.store(true);
  return true;
}

void TcpServer::Stop() {
  if (!m_running.exchange(false)) return;
  std::lock_guard<std::mutex> lock(m_clientsMutex);
  for (auto& c : m_clients) {
    if (c.socket != INVALID_SOCKET) closesocket(c.socket);
  }
  m_clients.clear();
  if (m_listenSocket != INVALID_SOCKET) {
    closesocket(m_listenSocket);
    m_listenSocket = INVALID_SOCKET;
  }
}

void TcpServer::AcceptNewClients() {
  {
    std::lock_guard<std::mutex> lock(m_clientsMutex);
    if (m_clients.size() >= MAX_CLIENTS) return;
  }

  sockaddr_in clientAddr{};
  int addrLen = sizeof(clientAddr);
  SOCKET clientSocket = accept(m_listenSocket, (sockaddr*)&clientAddr, &addrLen);
  if (clientSocket == INVALID_SOCKET) return;

  u_long mode = 1;
  ioctlsocket(clientSocket, FIONBIO, &mode);

  // Enable TCP keepalive
  BOOL keepAlive = TRUE;
  setsockopt(clientSocket, SOL_SOCKET, SO_KEEPALIVE, (const char*)&keepAlive, sizeof(keepAlive));
  DWORD keepIdle = 15000, keepInterval = 5000;
  setsockopt(clientSocket, IPPROTO_TCP, TCP_KEEPIDLE, (const char*)&keepIdle, sizeof(keepIdle));
  setsockopt(clientSocket, IPPROTO_TCP, TCP_KEEPINTVL, (const char*)&keepInterval, sizeof(keepInterval));

  TcpClientConnection conn;
  conn.socket = clientSocket;
  conn.lastActivity = GetTickCount64();

  std::lock_guard<std::mutex> lock(m_clientsMutex);
  m_clients.push_back(std::move(conn));
}

void TcpServer::ReadFromClients() {
  std::lock_guard<std::mutex> lock(m_clientsMutex);
  uint8_t buf[RECV_BUFFER_SIZE];

  for (size_t i = 0; i < m_clients.size();) {
    auto& c = m_clients[i];
    int bytesRead = recv(c.socket, (char*)buf, RECV_BUFFER_SIZE, 0);

    if (bytesRead > 0) {
      c.lastActivity = GetTickCount64();
      c.readBuffer.insert(c.readBuffer.end(), buf, buf + bytesRead);
      ProcessFrames(c);
      ++i;
    } else if (bytesRead == 0) {
      RemoveClient(i);
    } else {
      int err = WSAGetLastError();
      if (err == WSAEWOULDBLOCK) {
        ++i;
      } else {
        RemoveClient(i);
      }
    }
  }
}

void TcpServer::ProcessFrames(TcpClientConnection& client) {
  while (client.readBuffer.size() >= 4) {
    uint32_t payloadLen = 0;
    memcpy(&payloadLen, client.readBuffer.data(), 4);

    // Reject oversized frames
    if (payloadLen > 4 * 1024 * 1024) {
      closesocket(client.socket);
      client.socket = INVALID_SOCKET;
      return;
    }

    if (client.readBuffer.size() < 4 + payloadLen) break;

    std::string utf8((char*)client.readBuffer.data() + 4, payloadLen);
    client.readBuffer.erase(client.readBuffer.begin(), client.readBuffer.begin() + 4 + payloadLen);

    // Handle AUTH: AUTH|<pin>|<deviceId>|<deviceName>
    // Milkwave does not require explicit authorization - clients are authenticated
    // immediately if the PIN matches (or no PIN is configured).
    if (utf8.rfind("AUTH|", 0) == 0) {
      // Evict stale connections from the same device
      {
        size_t p1 = utf8.find('|', 0);
        size_t p2 = (p1 != std::string::npos) ? utf8.find('|', p1 + 1) : std::string::npos;
        size_t p3 = (p2 != std::string::npos) ? utf8.find('|', p2 + 1) : std::string::npos;
        if (p2 != std::string::npos) {
          std::string incomingId = utf8.substr(p2 + 1, (p3 != std::string::npos ? p3 : utf8.size()) - p2 - 1);
          if (!incomingId.empty()) {
            for (size_t j = 0; j < m_clients.size(); ++j) {
              if (&m_clients[j] != &client && m_clients[j].deviceId == incomingId) {
                closesocket(m_clients[j].socket);
                m_clients[j].socket = INVALID_SOCKET;
              }
            }
            for (size_t j = m_clients.size(); j-- > 0;) {
              if (m_clients[j].socket == INVALID_SOCKET && &m_clients[j] != &client) {
                m_clients.erase(m_clients.begin() + j);
              }
            }
          }
        }
      }

      std::vector<std::string> parts;
      size_t start = 0;
      for (size_t pos = 0; pos <= utf8.size(); ++pos) {
        if (pos == utf8.size() || utf8[pos] == '|') {
          parts.push_back(utf8.substr(start, pos - start));
          start = pos + 1;
        }
      }

      if (parts.size() >= 4) {
        const std::string& clientPin = parts[1];
        client.deviceId = parts[2];
        client.deviceName = parts[3];

        // Check PIN: if no PIN configured, allow all. Otherwise must match.
        if (m_pin.empty() || clientPin == m_pin) {
          client.authenticated = true;
          SendTo(client, "AUTH_OK");
        } else {
          SendTo(client, "AUTH_FAIL|WRONG_PIN");
          closesocket(client.socket);
          client.socket = INVALID_SOCKET;
        }
      } else {
        SendTo(client, "AUTH_FAIL|MALFORMED");
      }
      continue;
    }

    // Drop all non-AUTH commands from unauthenticated clients
    if (!client.authenticated) continue;

    // Handle PING
    if (utf8 == "PING") {
      SendTo(client, "PONG");
      continue;
    }

    // Convert to wide and dispatch
    std::wstring wide = UTF8ToWide(utf8);
    if (!wide.empty() && m_onMessage) {
      m_onMessage(client, wide);
    }
  }
}

void TcpServer::SendRaw(SOCKET sock, const uint8_t* data, int len) {
  // Temporarily switch to blocking for reliable send
  u_long blocking = 0;
  ioctlsocket(sock, FIONBIO, &blocking);
  int sent = 0;
  while (sent < len) {
    int r = send(sock, (const char*)(data + sent), len - sent, 0);
    if (r == SOCKET_ERROR) break;
    sent += r;
  }
  u_long nonBlocking = 1;
  ioctlsocket(sock, FIONBIO, &nonBlocking);
}

void TcpServer::SendTo(TcpClientConnection& client, const std::string& utf8Message) {
  uint32_t payloadLen = (uint32_t)utf8Message.size();
  uint8_t header[4];
  memcpy(header, &payloadLen, 4);
  SendRaw(client.socket, header, 4);
  SendRaw(client.socket, (const uint8_t*)utf8Message.data(), (int)payloadLen);
}

void TcpServer::SendTo(TcpClientConnection& client, const std::wstring& message) {
  SendTo(client, WideToUTF8(message));
}

void TcpServer::Broadcast(const std::wstring& message) {
  std::string utf8 = WideToUTF8(message);
  std::lock_guard<std::mutex> lock(m_clientsMutex);
  for (auto& c : m_clients) {
    if (c.authenticated) {
      SendTo(c, utf8);
    }
  }
}

void TcpServer::Poll() {
  if (!m_running.load()) return;
  AcceptNewClients();
  ReadFromClients();
  CheckTimeouts();
}

void TcpServer::CheckTimeouts() {
  ULONGLONG now = GetTickCount64();
  std::lock_guard<std::mutex> lock(m_clientsMutex);
  for (size_t i = 0; i < m_clients.size();) {
    if (now - m_clients[i].lastActivity > CLIENT_TIMEOUT_MS) {
      RemoveClient(i);
    } else {
      ++i;
    }
  }
}

void TcpServer::RemoveClient(size_t index) {
  if (index < m_clients.size()) {
    closesocket(m_clients[index].socket);
    m_clients.erase(m_clients.begin() + index);
  }
}

std::vector<TcpClientInfo> TcpServer::GetConnectedClients() const {
  std::vector<TcpClientInfo> result;
  std::lock_guard<std::mutex> lock(m_clientsMutex);
  for (const auto& c : m_clients) {
    if (c.authenticated) {
      TcpClientInfo info;
      info.deviceId = c.deviceId;
      info.deviceName = c.deviceName;
      result.push_back(std::move(info));
    }
  }
  return result;
}

std::wstring TcpServer::UTF8ToWide(const std::string& utf8) {
  if (utf8.empty()) return {};
  int size = MultiByteToWideChar(CP_UTF8, 0, utf8.data(), (int)utf8.size(), nullptr, 0);
  if (size <= 0) return {};
  std::wstring wide(size, 0);
  MultiByteToWideChar(CP_UTF8, 0, utf8.data(), (int)utf8.size(), &wide[0], size);
  return wide;
}

std::string TcpServer::WideToUTF8(const std::wstring& wide) {
  if (wide.empty()) return {};
  int size = WideCharToMultiByte(CP_UTF8, 0, wide.data(), (int)wide.size(), nullptr, 0, nullptr, nullptr);
  if (size <= 0) return {};
  std::string utf8(size, 0);
  WideCharToMultiByte(CP_UTF8, 0, wide.data(), (int)wide.size(), &utf8[0], size, nullptr, nullptr);
  return utf8;
}
