using System;
using System.Net;
using System.Net.Sockets;
using System.Text;
using SimulatorManagerClient;

namespace SimulatorsManager
{
    internal class Simulator
    {
        public delegate void UpdatedEventHandler();

        public event UpdatedEventHandler Updated;

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
                OnUpdate();
            }
        }

        public void Tcp(string command)
        {
            try
            {
                byte[] buff;
                using (var tcp = new TcpClient())
                {
                    tcp.Connect(new IPEndPoint(IPAddress.Parse(IP), 27015));
                    using (var stream = tcp.GetStream())
                    {
                        var sc = new StreamChuck(stream);
                        sc.PutPackage(Encoding.UTF8.GetBytes(command));
                        buff = sc.GetPackage();
                    }
                    tcp.Close();
                }
                Returns =
                    $"27015@{DateTime.Now:HH:mm:ss.ff}:{Environment.NewLine}{Encoding.UTF8.GetString(buff)}";
            }
            catch (SocketException e)
            {
                Returns = $"localhost@{DateTime.Now:HH:mm:ss.ff}:{Environment.NewLine}{e}";
            }
            OnUpdate();
        }

        public void OnUpdate() => Updated?.Invoke();
    }
}