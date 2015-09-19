using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;

namespace SimulatorManagerClient
{
    class Program
    {
        private static IPEndPoint m_IP;
        private static bool m_AutoSave;
        private static int m_AutoSaveInterval = 10000;

        static void Main(string[] args)
        {
            Process proc = null;

            var udp = new UdpClient(27016);
            m_IP = new IPEndPoint(IPAddress.Any, 27016);

            var thread = new Thread(AutoProcess) { IsBackground = true };
            thread.Start();

            while (true)
            {
                var data = udp.Receive(ref m_IP);
                var str = Encoding.UTF8.GetString(data);
                Console.WriteLine($"{m_IP.Address}:{m_IP.Port} {m_IP.AddressFamily} : {str}");
                var ret = "unknown command";
                switch (str)
                {
                    case "exit":
                        return;
                    case "version":
                        ret = $"dll:{File.GetLastWriteTime("MineSweeperSolver.dll"):yyyy-MM-ddTHH:mm:sszzzz} " +
                              $"exe:{File.GetLastWriteTime("MineSweeperSimulator.exe"):yyyy-MM-ddTHH:mm:sszzzz} " +
                              $"client:{File.GetLastWriteTime("SimulatorManagerClient.exe"):yyyy-MM-ddTHH:mm:sszzzz}";
                        break;
                    case "state":
                        ret = proc != null ? $"still running id:{proc.Id} exited:{proc.HasExited}" : "not yet started";
                        break;
                    case "start":
                        try
                        {
                            proc = Process.Start("MineSweeperSimulator.exe");
                            ret = proc != null ? $"id:{proc.Id} exited:{proc.HasExited}" : "failed";
                        }
                        catch (Exception e)
                        {
                            ret = e.ToString();
                        }
                        break;
                    case "terminate":
                        if (proc != null)
                        {
                            try
                            {
                                if (!proc.HasExited)
                                    proc.Kill();
                                proc = null;
                                ret = "ok";
                            }
                            catch (Exception e)
                            {
                                ret = e.ToString();
                            }
                        }
                        else
                            ret = "not yet started";
                        break;
                    case "update dll":
                        if (proc != null)
                            ret = "still running";
                        else
                        {
                            try
                            {
                                Download("MineSweeperSolver.dll");
                                ret = "ok";
                            }
                            catch (Exception e)
                            {
                                ret = e.ToString();
                            }
                        }
                        break;
                    case "update exe":
                        if (proc != null)
                            ret = "still running";
                        else
                        {
                            try
                            {
                                Download("MineSweeperSimulator.exe");
                                ret = "ok";
                            }
                            catch (Exception e)
                            {
                                ret = e.ToString();
                            }
                        }
                        break;
                    case "update client":
                        if (proc != null)
                            ret = "still running";
                        else
                        {
                            try
                            {
                                Download("tmp.exe");
                                Process.Start("UpdateClient.vbs");
                                return;
                            }
                            catch (Exception e)
                            {
                                ret = e.ToString();
                            }
                        }
                        break;
                    case "upload":
                        if (proc != null)
                            ret = "still running";
                        else
                        {
                            var buff = new byte[4096];
                            try
                            {
                                using (var tcp = new TcpClient())
                                {
                                    Console.WriteLine($"Connecting {m_IP.Address}:{m_IP.Port}");
                                    tcp.Connect(m_IP);
                                    Console.WriteLine($"Connected {m_IP.Address}:{m_IP.Port}");
                                    using (var stream = tcp.GetStream())
                                    using (var file = File.OpenRead("output.txt"))
                                        while (true)
                                        {
                                            var count = file.Read(buff, 0, 4096);
                                            Console.Write($"Read {count} bytes ...");
                                            if (count == 0)
                                                break;
                                            stream.Write(buff, 0, count);
                                            Console.WriteLine($" Sent {count} bytes");
                                        }
                                    Console.WriteLine(" Done");
                                    tcp.Close();
                                }
                                ret = "ok";
                            }
                            catch (Exception e)
                            {
                                ret = e.ToString();
                            }
                        }
                        break;
                    case "drop":
                        if (proc != null)
                            ret = "still running";
                        else
                        {
                            try
                            {
                                File.Delete("output.txt");
                                ret = "ok";
                            }
                            catch (Exception e)
                            {
                                ret = e.ToString();
                            }
                        }
                        break;
                    case "auto":
                        ret = $"auto: {m_AutoSave} {m_AutoSaveInterval}";
                        break;
                    case "auto on 10":
                        m_AutoSave = true;
                        m_AutoSaveInterval = 10000;
                        ret = $"auto: {m_AutoSave} {m_AutoSaveInterval}";
                        break;
                    case "auto on 60":
                        m_AutoSave = true;
                        m_AutoSaveInterval = 60000;
                        ret = $"auto: {m_AutoSave} {m_AutoSaveInterval}";
                        break;
                    case "auto on 600":
                        m_AutoSave = true;
                        m_AutoSaveInterval = 600000;
                        ret = $"auto: {m_AutoSave} {m_AutoSaveInterval}";
                        break;
                    case "auto off":
                        m_AutoSave = false;
                        m_AutoSaveInterval = 60000;
                        ret = $"auto: {m_AutoSave} {m_AutoSaveInterval}";
                        break;
                }
                var retData = Encoding.UTF8.GetBytes(ret);
                Console.WriteLine(ret);
                try
                {
                    udp.Send(retData, retData.Length, m_IP);
                }
                catch (Exception e)
                {
                    Console.WriteLine(e.ToString());
                }
                Console.WriteLine();
            }
        }

        private static void Download(string filename)
        {
            var buff = new byte[4096];
            using (var tcp = new TcpClient())
            {
                Console.WriteLine($"Connecting {m_IP.Address}:{m_IP.Port}");
                tcp.Connect(m_IP);
                Console.WriteLine($"Connected {m_IP.Address}:{m_IP.Port}");
                if (File.Exists(filename))
                    try
                    {
                        File.Delete(filename);
                    }
                    catch (Exception)
                    {
                        // ignored
                    }
                using (var stream = tcp.GetStream())
                using (var file = File.OpenWrite(filename))
                    while (true)
                    {
                        var count = stream.Read(buff, 0, 4096);
                        Console.Write($"Received {count} bytes ...");
                        if (count == 0)
                            break;
                        file.Write(buff, 0, count);
                        Console.WriteLine($" Wrote {count} bytes");
                    }
                Console.WriteLine(" Done");
            }
        }

        private static void AutoProcess()
        {
            var selfIP = new IPEndPoint(IPAddress.Loopback, 27015);
            foreach (
                var ip in
                    Dns.GetHostEntry(Dns.GetHostName())
                       .AddressList.Where(ip => ip.AddressFamily == AddressFamily.InterNetwork))
            {
                selfIP = new IPEndPoint(ip, 27015);
                break;
            }

            Console.WriteLine(selfIP.Address.ToString());

            var udp = new UdpClient(27017);
            while (true)
            {
                Thread.Sleep(m_AutoSaveInterval);
                Console.WriteLine("Check for auto");
                if (!m_AutoSave)
                {
                    Console.WriteLine();
                    continue;
                }

                byte[] buff;
                int lng;
                try
                {
                    using (var tcp = new TcpClient())
                    {
                        Console.WriteLine($"Connecting {selfIP.Address}:{selfIP.Port}");
                        tcp.Connect(selfIP);
                        Console.WriteLine($"Connected {selfIP.Address}:{selfIP.Port}");
                        using (var stream = tcp.GetStream())
                        {
                            var data = Encoding.ASCII.GetBytes("save");
                            stream.Write(data, 0, data.Length);
                            Console.Write($"Wrote {data.Length} ...");
                            stream.Flush();

                            buff = new byte[4096];
                            lng = stream.Read(buff, 0, 4096);
                            Console.WriteLine($" Read {lng}");
                        }
                    }
                }
                catch (Exception e)
                {
                    buff = Encoding.UTF8.GetBytes(e.ToString());
                    lng = buff.Length;
                }
                try
                {
                    udp.Send(buff, lng, m_IP);
                }
                catch (Exception e)
                {
                    Console.WriteLine(e.ToString());
                }
                Console.WriteLine(Encoding.UTF8.GetString(buff, 0, lng));
            }
            // ReSharper disable once FunctionNeverReturns
        }
    }
}
