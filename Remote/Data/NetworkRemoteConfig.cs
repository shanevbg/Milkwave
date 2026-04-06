namespace MilkwaveRemote.Data {
  public class NetworkRemoteTarget {
    public bool Active { get; set; } = false;
    public string Name { get; set; } = "";
    public string Host { get; set; } = "";
    public int Port { get; set; } = 9270;
    public string Pin { get; set; } = "";
    public string DeviceId { get; set; } = "milkwave-remote";
    public string DeviceName { get; set; } = "Milkwave Remote";
  }

  public class NetworkRemoteConfig {
    public List<NetworkRemoteTarget> Targets { get; set; } = new();
  }
}
