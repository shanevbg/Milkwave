using System.Diagnostics;
using System.Runtime.InteropServices;

namespace MilkwaveRemote.Helper {
  internal static class MonitorHelper {
    private static readonly string LogPath = Path.Combine(
        AppContext.BaseDirectory, "monitor-debug.log");

    // CPU: uses GetSystemTimes (P/Invoke) — instant, no PerformanceCounter
    private static long _prevIdleTicks;
    private static long _prevTotalTicks;
    private static bool _cpuPrimed;

    // GPU: uses native PDH API — bypasses slow .NET PerformanceCounter wrapper
    private static IntPtr _pdhQuery;
    private static IntPtr _pdhGpuCounter;
    private static volatile bool _gpuReady;
    private static volatile bool _gpuInitStarted;
    private static volatile bool _gpuFailed;
    private static int _gpuLogCount;

    [DllImport("kernel32.dll", SetLastError = true)]
    [return: MarshalAs(UnmanagedType.Bool)]
    private static extern bool GetSystemTimes(
        out long idleTime, out long kernelTime, out long userTime);

    // PDH (Performance Data Helper) native API
    [DllImport("pdh.dll", CharSet = CharSet.Unicode)]
    private static extern int PdhOpenQuery(string? dataSource, IntPtr userData, out IntPtr query);

    [DllImport("pdh.dll", CharSet = CharSet.Unicode)]
    private static extern int PdhAddEnglishCounter(IntPtr query, string fullCounterPath,
        IntPtr userData, out IntPtr counter);

    [DllImport("pdh.dll")]
    private static extern int PdhCollectQueryData(IntPtr query);

    [DllImport("pdh.dll")]
    private static extern int PdhGetFormattedCounterArray(IntPtr counter, uint dwFormat,
        ref uint lpdwBufferSize, out uint lpdwItemCount, IntPtr ItemBuffer);

    [DllImport("pdh.dll")]
    private static extern int PdhCloseQuery(IntPtr query);

    // Each item in the array: instance name pointer + PDH_FMT_COUNTERVALUE
    [StructLayout(LayoutKind.Sequential)]
    private struct PDH_FMT_COUNTERVALUE {
      public uint CStatus;
      public double doubleValue;
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct PDH_FMT_COUNTERVALUE_ITEM {
      public IntPtr szName;     // LPWSTR
      public PDH_FMT_COUNTERVALUE FmtValue;
    }

    private const uint PDH_FMT_DOUBLE = 0x00000200;
    private const uint PDH_FMT_NOCAP100 = 0x00008000;
    private const int PDH_MORE_DATA = unchecked((int)0x800007D2);

    public static void Log(string message) {
      try {
        var line = $"[{DateTime.Now:HH:mm:ss.fff}] {message}\n";
        File.AppendAllText(LogPath, line);
      } catch { }
    }

    public static void Initialize() {
      Log("Initialize: priming CPU (GetSystemTimes) and starting GPU PDH init");
      if (GetSystemTimes(out var idle, out var kernel, out var user)) {
        _prevIdleTicks = idle;
        _prevTotalTicks = kernel + user;
        _cpuPrimed = true;
        Log("Initialize: CPU primed");
      } else {
        Log($"Initialize: GetSystemTimes failed, error={Marshal.GetLastWin32Error()}");
      }
      EnsureGpuInit();
    }

    private static void EnsureGpuInit() {
      if (_gpuInitStarted || _gpuFailed) return;
      _gpuInitStarted = true;
      var initThread = new Thread(() => {
        try {
          Log("GPU PDH init: opening query...");
          var sw = Stopwatch.StartNew();

          int status = PdhOpenQuery(null, IntPtr.Zero, out _pdhQuery);
          if (status != 0) {
            Log($"GPU PDH init: PdhOpenQuery failed: 0x{status:X8}");
            _gpuFailed = true;
            return;
          }
          Log($"GPU PDH init: query opened in {sw.ElapsedMilliseconds}ms");

          status = PdhAddEnglishCounter(_pdhQuery,
              @"\GPU Engine(*)\Utilization Percentage",
              IntPtr.Zero, out _pdhGpuCounter);
          if (status != 0) {
            Log($"GPU PDH init: PdhAddEnglishCounter failed: 0x{status:X8}");
            PdhCloseQuery(_pdhQuery);
            _pdhQuery = IntPtr.Zero;
            _gpuFailed = true;
            return;
          }
          Log($"GPU PDH init: counter added in {sw.ElapsedMilliseconds}ms");

          // First collect primes the counter (establishes baseline)
          status = PdhCollectQueryData(_pdhQuery);
          if (status != 0) {
            Log($"GPU PDH init: first PdhCollectQueryData failed: 0x{status:X8}");
            PdhCloseQuery(_pdhQuery);
            _pdhQuery = IntPtr.Zero;
            _gpuFailed = true;
            return;
          }
          Log($"GPU PDH init: primed in {sw.ElapsedMilliseconds}ms");
          _gpuReady = true;
        } catch (Exception ex) {
          Log($"GPU PDH init exception: {ex.GetType().Name}: {ex.Message}");
          _gpuFailed = true;
        }
      }) {
        IsBackground = true,
        Name = "GpuPdhInit"
      };
      initThread.Start();

      // Watchdog: if init doesn't complete in 30 seconds, give up
      Task.Run(async () => {
        await Task.Delay(30_000);
        if (!_gpuReady && !_gpuFailed) {
          Log("GPU PDH init: TIMEOUT (30s) — GPU monitoring unavailable");
          _gpuFailed = true;
        }
      });
    }

    public static float GetCPUUsage() {
      if (!_cpuPrimed) return -1f;
      if (!GetSystemTimes(out var idle, out var kernel, out var user)) return -1f;

      long totalTicks = kernel + user;
      long totalDelta = totalTicks - _prevTotalTicks;
      long idleDelta = idle - _prevIdleTicks;

      _prevIdleTicks = idle;
      _prevTotalTicks = totalTicks;

      if (totalDelta <= 0) return 0f;
      return (1.0f - (float)idleDelta / totalDelta) * 100f;
    }

    public static float GetGPUUsage() {
      if (_gpuFailed) return -2f;
      if (!_gpuReady) {
        EnsureGpuInit();
        return -1f;
      }
      try {
        int status = PdhCollectQueryData(_pdhQuery);
        if (status != 0) {
          Log($"GetGPUUsage: PdhCollectQueryData failed: 0x{status:X8}");
          return -1f;
        }

        // First call: get required buffer size
        uint bufSize = 0;
        status = PdhGetFormattedCounterArray(_pdhGpuCounter,
            PDH_FMT_DOUBLE | PDH_FMT_NOCAP100,
            ref bufSize, out uint itemCount, IntPtr.Zero);

        if (status != PDH_MORE_DATA || bufSize == 0) {
          if (_gpuLogCount < 5) {
            Log($"GetGPUUsage: size query returned 0x{status:X8}, bufSize={bufSize}");
            _gpuLogCount++;
          }
          return -1f;
        }

        // Second call: read actual data
        IntPtr buf = Marshal.AllocHGlobal((int)bufSize);
        try {
          status = PdhGetFormattedCounterArray(_pdhGpuCounter,
              PDH_FMT_DOUBLE | PDH_FMT_NOCAP100,
              ref bufSize, out itemCount, buf);

          if (status != 0) {
            if (_gpuLogCount < 5) {
              Log($"GetGPUUsage: read returned 0x{status:X8}");
              _gpuLogCount++;
            }
            return -1f;
          }

          // Sum all GPU engine utilization values
          double total = 0;
          int itemSize = Marshal.SizeOf<PDH_FMT_COUNTERVALUE_ITEM>();
          for (int i = 0; i < (int)itemCount; i++) {
            var item = Marshal.PtrToStructure<PDH_FMT_COUNTERVALUE_ITEM>(
                buf + i * itemSize);
            if (item.FmtValue.CStatus == 0) { // PDH_CSTATUS_VALID_DATA
              total += item.FmtValue.doubleValue;
            }
          }

          // Log first few reads for debugging
          if (_gpuLogCount < 5) {
            Log($"GetGPUUsage: {itemCount} instances, total={total:F1}%");
            _gpuLogCount++;
          }

          return (float)total;
        } finally {
          Marshal.FreeHGlobal(buf);
        }
      } catch (Exception ex) {
        Log($"GetGPUUsage exception: {ex.Message}");
        return -1f;
      }
    }
  }
}
