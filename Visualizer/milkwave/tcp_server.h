#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include <functional>
#include <cstdint>

#pragma comment(lib, "ws2_32.lib")

// Snapshot of a connected client for UI display
struct TcpClientInfo {
  std::string deviceId;
  std::string deviceName;
};

struct TcpClientConnection {
  SOCKET socket = INVALID_SOCKET;
  bool authenticated = false;
  std::string deviceId;
  std::string deviceName;
  ULONGLONG lastActivity = 0;  // GetTickCount64
  std::vector<uint8_t> readBuffer;
};

// Thread-local pointer to the TCP client that initiated the current command.
// Set before PostMessage dispatch, read by response-sending code to route
// replies to the requesting client instead of broadcasting.
extern thread_local TcpClientConnection* g_respondingTcpClient;

class TcpServer {
 public:
  using MessageHandler = std::function<void(TcpClientConnection& client, const std::wstring& message)>;

  TcpServer();
  ~TcpServer();

  bool Start(int port, const std::string& pin, MessageHandler onMessage);
  void Stop();
  void Poll();  // Called from main loop - non-blocking select

  void Broadcast(const std::wstring& message);  // Send to all authenticated clients
  void SendTo(TcpClientConnection& client, const std::string& utf8Message);
  void SendTo(TcpClientConnection& client, const std::wstring& message);

  bool IsRunning() const { return m_running.load(); }
  int GetPort() const { return m_port; }
  std::vector<TcpClientInfo> GetConnectedClients() const;

 private:
  void AcceptNewClients();
  void ReadFromClients();
  void ProcessFrames(TcpClientConnection& client);
  void RemoveClient(size_t index);
  void SendRaw(SOCKET sock, const uint8_t* data, int len);
  void CheckTimeouts();

  static std::wstring UTF8ToWide(const std::string& utf8);
  static std::string WideToUTF8(const std::wstring& wide);

  SOCKET m_listenSocket = INVALID_SOCKET;
  int m_port = 9270;
  std::string m_pin;  // Clear-text PIN (empty = no PIN required)
  std::atomic<bool> m_running{false};
  std::vector<TcpClientConnection> m_clients;
  mutable std::mutex m_clientsMutex;
  MessageHandler m_onMessage;

  static constexpr int RECV_BUFFER_SIZE = 65536;
  static constexpr ULONGLONG CLIENT_TIMEOUT_MS = 60000;
  static constexpr size_t MAX_CLIENTS = 16;
};
