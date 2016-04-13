using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Reflection;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

namespace SimulatorsManager
{
    public class SimulatorsManager
    {
        public readonly BindingSource Binding = new BindingSource();
        private readonly List<Simulator> m_Simulators = new List<Simulator>();
        private readonly UdpClient m_Udp;
        private readonly TcpListener m_Tcp;
        private int m_FileTranser;

        // ReSharper disable PrivateFieldCanBeConvertedToLocalVariable
        private readonly Thread m_TcpThread;
        private readonly Thread m_UdpThread;
        private readonly SimpleHttpServer m_HttpServer;
        // ReSharper restore PrivateFieldCanBeConvertedToLocalVariable

        public SimulatorsManager()
        {
            Binding.DataSource = m_Simulators;

            m_Udp = new UdpClient(27016);

            if (!File.Exists("config.txt"))
                throw new ApplicationException("没有找到配置文件config.txt");
            IPAddress ipManager, ipHttp;
            int portHttp;
            using (var config = new StreamReader("config.txt", Encoding.UTF8))
            {
                // ReSharper disable PossibleNullReferenceException
                var str = config.ReadLine().Split(new[] { ',' }, 3);
                if (str.Length != 3)
                    throw new ApplicationException("配置文件语法错误");

                ipManager = IPAddress.Parse(str[0].Trim());
                ipHttp = IPAddress.Parse(str[1].Trim());
                portHttp = int.Parse(str[2].Trim());

                while (!config.EndOfStream)
                {
                    var s = config.ReadLine().Split(new[] { ',' }, 4);

                    AddSimulator(
                                 new Simulator
                                     {
                                         Checked = bool.Parse(s[0].Trim()),
                                         ID = s[1].Trim(),
                                         IP = s[2].Trim(),
                                         State = s.Length >= 4 ? s[3].Trim() : null
                                     });
                }
                // ReSharper restore PossibleNullReferenceException
            }

            m_FileTranser = 0;
            m_Tcp = new TcpListener(ipManager, 27016);
            m_TcpThread = new Thread(TcpProcess)
                              {
                                  IsBackground = true,
                                  Name = "TcpProcess"
                              };
            m_TcpThread.Start();

            m_UdpThread = new Thread(UdpProcess)
                              {
                                  IsBackground = true,
                                  Name = "UdpThread"
                              };
            m_UdpThread.Start();

            m_HttpServer = new SimpleHttpServer(ipHttp, portHttp);
            m_HttpServer.OnHttpRequest += OnHttpRequest;
        }

        private HttpResponse OnHttpRequest(HttpRequest request)
        {
            if (request.Method == "HEAD")
            {
                if (request.Uri.EndsWith("/", StringComparison.Ordinal))
                    return
                        new HttpResponse
                            {
                                ResponseCode = 200,
                                Header =
                                    new Dictionary<string, string>
                                        {
                                            { "Content-Type", "text/html" }
                                        }
                            };
                if (request.Uri.EndsWith("/jquery-2.1.4.min.js", StringComparison.Ordinal))
                    return
                        new HttpResponse
                            {
                                ResponseCode = 200,
                                Header =
                                    new Dictionary<string, string>
                                        {
                                            { "Content-Type", "application/x-javascript" }
                                        }
                            };
                throw new HttpException(404);
            }
            if (request.Method == "GET")
            {
                if (request.Uri.EndsWith("/", StringComparison.Ordinal))
                    return
                        new HttpResponse
                            {
                                ResponseCode = 200,
                                Header =
                                    new Dictionary<string, string>
                                        {
                                            { "Content-Type", "text/html" }
                                        },
                                ResponseStream =
                                    Assembly.GetExecutingAssembly()
                                            .GetManifestResourceStream("wwwroot.default.html")
                            };
                if (request.Uri.EndsWith("/jquery-2.1.4.min.js", StringComparison.Ordinal))
                    return
                        new HttpResponse
                            {
                                ResponseCode = 200,
                                Header =
                                    new Dictionary<string, string>
                                        {
                                            { "Content-Type", "application/x-javascript" }
                                        },
                                ResponseStream =
                                    Assembly.GetExecutingAssembly()
                                            .GetManifestResourceStream("wwwroot.jquery-2.1.4.min.js")
                            };
                throw new HttpException(404);
            }
            if (request.Method == "POST")
            {
                if (request.Uri.EndsWith("/", StringComparison.Ordinal))
                    return GenerateHttpResponse(JsonConvert.SerializeObject(m_Simulators));
                if (request.Uri.EndsWith("/check", StringComparison.Ordinal))
                {
                    var buff = new byte[4096];
                    var sz = request.RequestStream.Read(
                                                        buff,
                                                        0,
                                                        Math.Min(
                                                                 buff.Length,
                                                                 Convert.ToInt32(request.Header["Content-Length"])));
                    var json = JObject.Parse(Encoding.UTF8.GetString(buff, 0, sz));
                    var id = m_Simulators.FindIndex(s => s.ID == (string)json["ID"]);
                    m_Simulators[id].Checked = (bool)json["Checked"];
                    m_Simulators[id].OnUpdate();
                    return GenerateHttpResponse(JsonConvert.SerializeObject(m_Simulators));
                }
                if (request.Uri.EndsWith("/command", StringComparison.Ordinal))
                {
                    var buff = new byte[4096];
                    var sz = request.RequestStream.Read(
                                                        buff,
                                                        0,
                                                        Math.Min(
                                                                 buff.Length,
                                                                 Convert.ToInt32(request.Header["Content-Length"])));
                    ProcessCommand(Encoding.UTF8.GetString(buff, 0, sz));
                    return GenerateHttpResponse("ok", "text/plain");
                }
                throw new HttpException(404);
            }
            throw new HttpException(405);
        }

        private static HttpResponse GenerateHttpResponse(string str, string contentType = "text/json")
        {
            var stream = new MemoryStream();
            var sw = new StreamWriter(stream);
            sw.Write(str);
            sw.Flush();
            stream.Position = 0;
            return
                new HttpResponse
                    {
                        ResponseCode = 200,
                        Header =
                            new Dictionary<string, string>
                                {
                                    { "Content-Type", contentType },
                                    { "Content-Length", stream.Length.ToString(CultureInfo.InvariantCulture) }
                                },
                        ResponseStream = stream
                    };
        }

        private void TcpProcess()
        {
            var buff = new byte[4096];
            m_Tcp.Start();
            while (true)
                try
                {
                    var tcp = m_Tcp.AcceptTcpClient();
                    switch (m_FileTranser)
                    {
                        case 0:
                            using (var stream = tcp.GetStream())
                            {
                                using (var file = File.OpenRead("MineSweeperSolver.dll"))
                                    while (true)
                                    {
                                        var count = file.Read(buff, 0, 4096);
                                        if (count == 0)
                                            break;
                                        stream.Write(buff, 0, count);
                                    }
                                stream.Close();
                            }
                            tcp.Close();
                            break;
                        case 1:
                            using (var stream = tcp.GetStream())
                            {
                                using (var file = File.OpenRead("MineSweeperSimulator.exe"))
                                    while (true)
                                    {
                                        var count = file.Read(buff, 0, 4096);
                                        if (count == 0)
                                            break;
                                        stream.Write(buff, 0, count);
                                    }
                                stream.Close();
                            }
                            tcp.Close();
                            tcp.Close();
                            break;
                        case 2:
                            using (var stream = tcp.GetStream())
                            {
                                using (var file = File.OpenRead("SimulatorManagerClient.exe"))
                                    while (true)
                                    {
                                        var count = file.Read(buff, 0, 4096);
                                        if (count == 0)
                                            break;
                                        stream.Write(buff, 0, count);
                                    }
                                stream.Close();
                            }
                            tcp.Close();
                            tcp.Close();
                            break;
                        case 3:
                            using (var stream = tcp.GetStream())
                            {
                                using (var file = File.Open("output.txt", FileMode.Append))
                                    while (true)
                                    {
                                        var count = stream.Read(buff, 0, 4096);
                                        if (count == 0)
                                            break;
                                        file.Write(buff, 0, count);
                                    }
                                stream.Close();
                            }
                            tcp.Close();
                            break;
                    }
                }
                catch (Exception)
                {
                    // ignored
                }
            // ReSharper disable once FunctionNeverReturns
        }

        private void UdpProcess()
        {
            while (true)
                try
                {
                    var ip = new IPEndPoint(IPAddress.Any, 27016);
                    var data = m_Udp.Receive(ref ip);
                    var id = m_Simulators.FindIndex(s => s.IP == ip.Address.ToString());
                    m_Simulators[id].Returns =
                        $"{ip.Port}@{DateTime.Now:HH:mm:ss.ff}:{Environment.NewLine}{Encoding.UTF8.GetString(data)}";
                    m_Simulators[id].OnUpdate();
                }
                catch (Exception)
                {
                    // ignored
                }
            // ReSharper disable once FunctionNeverReturns
        }

        private void AddSimulator(Simulator simu)
        {
            simu.Updated += () => Binding.ResetItem(m_Simulators.IndexOf(simu));
            Binding.Insert(0, simu);
        }

        public bool ProcessCommand(string cmd)
        {
            var sp = cmd.Split(new[] { ':' }, 2, StringSplitOptions.None);
            switch (sp[0])
            {
                case "":
                    if (sp.Length == 1)
                        return false;
                    for (var i = 0; i < m_Simulators.Count; i++)
                    {
                        if (!m_Simulators[i].Checked)
                            continue;
                        var i1 = i;
                        Task.Run(() => m_Simulators[i1].Tcp(sp[1]));
                    }
                    break;
                case "dll":
                    m_FileTranser = 0;
                    foreach (var simulator in m_Simulators.Where(simulator => simulator.Checked))
                        simulator.Udp(m_Udp, "update dll");
                    break;
                case "exe":
                    m_FileTranser = 1;
                    foreach (var simulator in m_Simulators.Where(simulator => simulator.Checked))
                        simulator.Udp(m_Udp, "update exe");
                    break;
                case "client":
                    m_FileTranser = 2;
                    foreach (var simulator in m_Simulators.Where(simulator => simulator.Checked))
                        simulator.Udp(m_Udp, "update client");
                    break;
                case "out":
                    m_FileTranser = 3;
                    foreach (var simulator in m_Simulators.Where(simulator => simulator.Checked))
                        simulator.Udp(m_Udp, "upload");
                    break;
                case "fake":
                    AddSimulator(
                                 new Simulator
                                     {
                                         Checked = true,
                                         ID = "",
                                         IP = "127.0.0.1",
                                         State = "fake",
                                         Returns = ""
                                     });
                    break;
                default:
                    foreach (var simulator in m_Simulators.Where(simulator => simulator.Checked))
                        simulator.Udp(m_Udp, sp[0]);
                    break;
            }
            return true;
        }
    }
}
