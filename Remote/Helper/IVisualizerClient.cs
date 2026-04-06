namespace MilkwaveRemote.Helper {
  public interface IVisualizerClient : IDisposable {
    bool IsConnected { get; }
    bool Send(string message);
    bool SendSignal(string signalName);
    void Disconnect();
    event Action<string>? MessageReceived;
    event Action? Disconnected;
  }
}
