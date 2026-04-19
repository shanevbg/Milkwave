using System.Net.Sockets;
using System.Text;

namespace MilkwaveRemote.Helper {

  public enum TcpAuthState {
    Connecting,
    ConnectionFailed,
    AuthPending,
    AuthOk,
    AuthFailed
  }

  /// <summary>
  /// TCP client for communicating with a remote MDropDX12 instance.
  /// Uses MDropDX12's length-framed UTF-8 wire protocol and AUTH handshake.
  /// </summary>
  public class TcpVisualizerClient : IVisualizerClient {
    private static readonly TimeSpan ConnectTimeout = TimeSpan.FromSeconds(10);
    private TcpClient? _tcp;
    private NetworkStream? _stream;
    private CancellationTokenSource? _cts;
    private readonly object _writeLock = new();
    private volatile bool _isConnected;

    public bool IsConnected => _isConnected;

    public event Action<string>? MessageReceived;
    public event Action? Disconnected;
    public event Action<TcpAuthState>? AuthStateChanged;

    /// <summary>
    /// Connect to a remote MDropDX12 TCP server and perform the AUTH handshake.
    /// </summary>
    public async Task ConnectAsync(string host, int port, string pin, string deviceId, string deviceName, TimeSpan? connectTimeout = null) {
      Disconnect();

      _cts = new CancellationTokenSource();

      try {
        AuthStateChanged?.Invoke(TcpAuthState.Connecting);

        _tcp = new TcpClient();
        using var connectCts = new CancellationTokenSource(connectTimeout ?? ConnectTimeout);
        Program.LogToFile($"TcpVisualizerClient connecting to {host}:{port}");
        await _tcp.ConnectAsync(host, port, connectCts.Token);
        Program.LogToFile($"TcpVisualizerClient connected to {host}:{port}");
        _stream = _tcp.GetStream();

        // Send AUTH handshake
        Program.LogToFile($"TcpVisualizerClient sending AUTH for {deviceId} (pinLen={pin.Length})");
        SendRaw($"AUTH|{pin}|{deviceId}|{deviceName}");

        // Start read loop on background thread
        var ct = _cts.Token;
        _ = Task.Run(() => ReadLoop(ct), ct);
      } catch (Exception ex) {
        Program.LogToFile($"TcpVisualizerClient connection failed: {ex.Message}");
        AuthStateChanged?.Invoke(TcpAuthState.ConnectionFailed);
        CleanupConnection();
      }
    }

    public bool Send(string message) {
      if (!_isConnected || _stream == null)
        return false;

      try {
        SendRaw(message);
        return true;
      } catch (Exception ex) {
        Program.LogToFile($"TcpVisualizerClient send failed: {ex.Message}");
        return false;
      }
    }

    public bool SendSignal(string signalName) {
      return Send($"SIGNAL|{signalName}");
    }

    public void Disconnect() {
      Program.LogToFile("TcpVisualizerClient disconnect requested");
      _cts?.Cancel();
      CleanupConnection();
    }

    public void Dispose() {
      Program.LogToFile("TcpVisualizerClient dispose requested");
      Disconnect();
      _cts?.Dispose();
      _cts = null;
      GC.SuppressFinalize(this);
    }

    private void SendRaw(string message) {
      if (_stream == null) return;

      byte[] payload = Encoding.UTF8.GetBytes(message);
      byte[] header = BitConverter.GetBytes((uint)payload.Length);

      lock (_writeLock) {
        _stream.Write(header, 0, header.Length);
        _stream.Write(payload, 0, payload.Length);
        _stream.Flush();
      }
    }

    private void ReadLoop(CancellationToken ct) {
      bool authCompleted = false;

      try {
        while (!ct.IsCancellationRequested && _tcp != null && _tcp.Connected && _stream != null) {
          // Read 4-byte length header
          byte[] headerBuf = new byte[4];
          if (!ReadExact(headerBuf, 4, ct))
            break;

          uint length = BitConverter.ToUInt32(headerBuf, 0);
          if (length == 0 || length > 1_048_576) // sanity: max 1MB
            break;

          // Read payload
          byte[] payloadBuf = new byte[length];
          if (!ReadExact(payloadBuf, (int)length, ct))
            break;

          string message = Encoding.UTF8.GetString(payloadBuf).TrimEnd('\0');

          if (!authCompleted) {
            if (message == "AUTH_OK") {
              Program.LogToFile("TcpVisualizerClient auth OK");
              authCompleted = true;
              _isConnected = true;
              AuthStateChanged?.Invoke(TcpAuthState.AuthOk);
            } else if (message == "AUTH_PENDING") {
              Program.LogToFile("TcpVisualizerClient auth pending");
              AuthStateChanged?.Invoke(TcpAuthState.AuthPending);
              // Stay in loop — AUTH_OK will arrive when user approves
            } else if (message.StartsWith("AUTH_FAIL")) {
              Program.LogToFile($"TcpVisualizerClient auth failed response: {message}");
              AuthStateChanged?.Invoke(TcpAuthState.AuthFailed);
              break;
            } else {
              // Unexpected response during auth
              Program.LogToFile($"TcpVisualizerClient unexpected auth response: {message}");
              AuthStateChanged?.Invoke(TcpAuthState.AuthFailed);
              break;
            }
          } else {
            // Post-auth: forward messages
            try {
              MessageReceived?.Invoke(message);
            } catch {
              // Don't let handler exceptions kill the read loop
            }
          }
        }
      } catch (OperationCanceledException) {
        // Normal shutdown
      } catch (Exception ex) {
        Program.LogToFile($"TcpVisualizerClient read loop error: {ex.Message}");
      }

      _isConnected = false;
      Disconnected?.Invoke();
    }

    private bool ReadExact(byte[] buffer, int count, CancellationToken ct) {
      int offset = 0;
      while (offset < count) {
        if (ct.IsCancellationRequested)
          return false;

        int bytesRead;
        try {
          bytesRead = _stream!.Read(buffer, offset, count - offset);
        } catch (IOException) {
          return false;
        } catch (ObjectDisposedException) {
          return false;
        }

        if (bytesRead == 0)
          return false; // connection closed

        offset += bytesRead;
      }
      return true;
    }

    private void CleanupConnection() {
      _isConnected = false;
      try { _stream?.Dispose(); } catch { }
      try { _tcp?.Dispose(); } catch { }
      _stream = null;
      _tcp = null;
    }
  }
}
