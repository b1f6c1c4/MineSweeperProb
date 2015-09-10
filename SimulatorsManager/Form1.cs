using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Reflection;
using System.Security.Cryptography;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Xml;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

namespace SimulatorsManager
{
    public partial class Form1 : Form
    {
        private const string AccessKey = "e574f92981814dfa9cbb17206d18f7fd";
        private const string Secret = "4f78d93ad02942c9b9dcdafd17f9f0c2";

        private readonly List<Simulator> m_Simulators = new List<Simulator>();

        private readonly UdpClient m_Udp;
        private readonly TcpListener m_Tcp;
        private int m_FileTranser;
        // ReSharper disable PrivateFieldCanBeConvertedToLocalVariable
        private readonly Thread m_TcpThread;
        private readonly Thread m_UdpThread;
        private readonly SimpleHttpServer m_HttpServer;
        // ReSharper restore PrivateFieldCanBeConvertedToLocalVariable

        public Form1()
        {
            InitializeComponent();

            UpdateSimulators();
            dataGridView1.AutoGenerateColumns = false;
            dataGridView1.DataSource = m_Simulators;
            dataGridView1.Columns[4].DefaultCellStyle.WrapMode = DataGridViewTriState.True;
            m_Udp = new UdpClient(27016);
            IPAddress ipW = null, ipL = null;
            foreach (
                var ip in
                    Dns.GetHostEntry(Dns.GetHostName())
                       .AddressList.Where(ip => ip.AddressFamily == AddressFamily.InterNetwork))
                if (ip.ToString().StartsWith("43.", StringComparison.Ordinal))
                    ipW = ip;
                else
                    ipL = ip;
            if (ipL == null)
            {
                MessageBox.Show("ipL == null");
                throw new Exception();
            }
            if (ipW == null)
                ipW = IPAddress.Parse(ipL.ToString());

            m_FileTranser = 0;
            m_Tcp = new TcpListener(ipL, 27016);
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

            m_HttpServer = new SimpleHttpServer(ipW, 27015);
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
                                            .GetManifestResourceStream("SimulatorsManager.wwwroot.default.html")
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
                                            .GetManifestResourceStream("SimulatorsManager.wwwroot.jquery-2.1.4.min.js")
                        };
                if (request.Uri.EndsWith("/favicon.ico", StringComparison.Ordinal))
                    return
                        new HttpResponse
                        {
                            ResponseCode = 404,
                        };
                throw new HttpException(404);
            }
            if (request.Method == "POST")
            {
                if (request.Uri.EndsWith("/", StringComparison.Ordinal))
                    return GenerateHttpResponse();
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
                    dataGridView1.InvalidateCell(0, id);
                    return GenerateHttpResponse();
                }
                throw new HttpException(404);
            }
            throw new HttpException(405);
        }

        private HttpResponse GenerateHttpResponse()
        {
            var stream = new MemoryStream();
            var sw = new StreamWriter(stream);
            sw.Write(JsonConvert.SerializeObject(m_Simulators));
            sw.Flush();
            stream.Position = 0;
            return
                new HttpResponse
                    {
                        ResponseCode = 200,
                        Header =
                            new Dictionary<string, string>
                                {
                                    { "Content-Type", "text/json" },
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
                    dataGridView1.UpdateCellValue(4, id);
                }
                catch (Exception)
                {
                    // ignored
                }
            // ReSharper disable once FunctionNeverReturns
        }

        private void UpdateSimulators()
        {
            m_Simulators.Clear();
            var xml = SendAction("DescribeInstances", null);
            if (xml.DocumentElement == null)
                throw new Exception();
            if (xml.DocumentElement["InstanceSet"] == null)
                throw new Exception();
            foreach (XmlElement element in xml.DocumentElement["InstanceSet"])
            {
                if (element["cpu"] == null)
                    throw new Exception();
                if (element["instanceName"] == null)
                    throw new Exception();
                if (element["status"] == null)
                    throw new Exception();
                if (element["ipAddresses"] == null)
                    throw new Exception();
                m_Simulators.Add(
                                 new Simulator
                                     {
                                         ID = element["instanceName"].InnerText,
                                         Checked = true,
                                         State = element["status"].InnerText,
                                         IP = element["ipAddresses"].InnerText,
                                         Returns = string.Empty
                                     });
            }
            m_Simulators.Reverse();
            dataGridView1.Invalidate();
        }

        private static XmlDocument SendAction(string action, IDictionary<string, string> dic)
        {
            var sb = new StringBuilder();
            var postData = new Dictionary<string, string>
                               {
                                   { "Action", action },
                                   { "AWSAccessKeyId", AccessKey },
                                   {
                                       "Timestamp",
                                       //DateTime.Now.ToUniversalTime()
                                       new DateTime(2015, 09, 07, 12, 29, 42)
                                       .ToString("yyyy-MM-ddTHH:mm:ss000Z")
                                   },
                                   { "Region", "Beijing" },
                                   { "SignatureMethod", "HmacSHA256" },
                                   { "SignatureVersion", "2" }
                               };

            if (dic != null)
                foreach (var kvp in dic)
                    postData.Add(kvp.Key, kvp.Value);

            var ks = postData.Keys.ToList();
            ks.Sort(StringComparer.Ordinal);
            foreach (var k in ks)
            {
                sb.Append(UrlEncode(k));
                sb.Append("=");
                sb.Append(UrlEncode(postData[k]));
                sb.Append("&");
            }
            if (ks.Any())
                sb.Remove(sb.Length - 1, 1);

            var hmacsha256 = new HMACSHA256(Encoding.ASCII.GetBytes(Secret));
            var sign = hmacsha256.ComputeHash(Encoding.ASCII.GetBytes("POST\nmosapi.meituan.com\n/mcs/v1\n" + sb));
            sb.AppendFormat(
                            "&{0}={1}",
                            "Signature",
                            Convert.ToBase64String(sign));

            var data = Encoding.UTF8.GetBytes(sb.ToString());

            var req = WebRequest.CreateHttp("https://mosapi.meituan.com/mcs/v1");
            req.KeepAlive = false;
            req.Method = "POST";
            //req.ContentType = "application/x-form-urlencoded";
            req.ContentLength = data.Length;
            req.Accept = "application/xml";
            using (var stream = req.GetRequestStream())
                stream.Write(data, 0, data.Length);

            try
            {
                using (var res = req.GetResponse())
                using (var stream = res.GetResponseStream())
                {
                    if (stream == null)
                        throw new Exception();

                    using (var sr = new StreamReader(stream))
                    {
                        var xml = new XmlDocument();
                        xml.LoadXml(sr.ReadToEnd());
                        return xml;
                    }
                }
            }
            catch (WebException e)
            {
                using (var stream = e.Response.GetResponseStream())
                {
                    if (stream == null)
                        throw new Exception();

                    using (var sr = new StreamReader(stream))
                    {
                        var xml = new XmlDocument();
                        xml.LoadXml(sr.ReadToEnd());
                        return xml;
                    }
                }
            }
        }

        private static string UrlEncode(string s)
        {
            var sb = new StringBuilder();
            foreach (var c in s)
                if (char.IsDigit(c) ||
                    char.IsLetter(c) ||
                    c == '-' ||
                    c == '_' ||
                    c == '.' ||
                    c == '~')
                    sb.Append(c);
                else
                    foreach (var b in Encoding.UTF8.GetBytes(new string(c, 1)))
                        sb.AppendFormat("%{0:X2}", b);
            return sb.ToString();
        }

        private void textBox1_KeyUp(object sender, KeyEventArgs e)
        {
            if (e.KeyCode != Keys.Enter)
                return;

            var sp = textBox1.Text.Split(new[] { ':' }, 2, StringSplitOptions.None);
            switch (sp[0])
            {
                case "":
                    if (sp.Length == 1)
                        return;
                    for (var i = 0; i < m_Simulators.Count; i++)
                    {
                        if (!m_Simulators[i].Checked)
                            continue;
                        var i1 = i;
                        Task.Run(
                                 () =>
                                 {
                                     m_Simulators[i1].Tcp(sp[1]);
                                     dataGridView1.UpdateCellValue(4, i1);
                                 });
                    }
                    break;
                case "update":
                    UpdateSimulators();
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
                default:
                    foreach (var simulator in m_Simulators.Where(simulator => simulator.Checked))
                        simulator.Udp(m_Udp, sp[0]);
                    break;
            }

            textBox1.Clear();
        }
    }


    internal class Simulator
    {
        // ReSharper disable UnusedAutoPropertyAccessor.Global
        public bool Checked { get; set; }
        public string ID { get; set; }
        public string IP { get; set; }
        public string State { get; set; }
        public string Returns { get; set; }
        // ReSharper restore UnusedAutoPropertyAccessor.Global

        public void Udp(UdpClient udp, string command)
        {
            try
            {
                var data = Encoding.ASCII.GetBytes(command);
                udp.Send(data, data.Length, new IPEndPoint(IPAddress.Parse(IP), 27016));
            }
            catch (SocketException e)
            {
                Returns = $"localhost@{DateTime.Now:HH:mm:ss.ff}:{Environment.NewLine}{e}";
            }
        }

        public void Tcp(string command)
        {
            try
            {
                var buff = new byte[4096];
                int count;
                using (var tcp = new TcpClient())
                {
                    tcp.Connect(new IPEndPoint(IPAddress.Parse(IP), 27015));
                    using (var stream = tcp.GetStream())
                    {
                        var data = Encoding.UTF8.GetBytes(command);
                        stream.Write(data, 0, data.Length);
                        stream.Flush();

                        count = stream.Read(buff, 0, 4096);
                    }
                    tcp.Close();
                }
                Returns =
                    $"27015@{DateTime.Now:HH:mm:ss.ff}:{Environment.NewLine}{Encoding.ASCII.GetString(buff, 0, count)}";
            }
            catch (SocketException e)
            {
                Returns = $"localhost@{DateTime.Now:HH:mm:ss.ff}:{Environment.NewLine}{e}";
            }
        }
    }
}
