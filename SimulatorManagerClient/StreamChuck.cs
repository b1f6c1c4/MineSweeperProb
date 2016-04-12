using System;
using System.IO;
using System.Net;

namespace SimulatorManagerClient
{
    public sealed class StreamChuck
    {
        private const int BuffSize = 4096;
        private readonly Stream m_Stream;

        public StreamChuck(Stream stream) { m_Stream = stream; }

        private static byte[] ToBytes(long data) => BitConverter.GetBytes(IPAddress.HostToNetworkOrder(data));
        private static long FromBytes(byte[] data) => IPAddress.NetworkToHostOrder(BitConverter.ToInt64(data, 0));

        public void PutPackage(byte[] data)
        {
            var lng = ToBytes(data.Length);
            m_Stream.Write(lng, 0, lng.Length);

            m_Stream.Write(data, 0, data.Length);
            m_Stream.Flush();
        }

        public void PutStream(Stream stream)
        {
            var lng = ToBytes(stream.Length);
            m_Stream.Write(lng, 0, lng.Length);

            var buff = new byte[BuffSize];
            while (true)
            {
                var count = stream.Read(buff, 0, BuffSize);
                if (count == 0)
                    break;
                m_Stream.Write(buff, 0, count);
            }
            m_Stream.Flush();
        }

        public byte[] GetPackage()
        {
            var lng = GetBytes(sizeof(long));
            var size = FromBytes(lng);
            return GetBytes((int)size);
        }

        public void GetStream(Stream stream)
        {
            var lng = GetBytes(sizeof(long));
            var size = FromBytes(lng);

            var buff = new byte[BuffSize];
            var read = 0;
            while (read < size)
            {
                var count = m_Stream.Read(buff, 0, (int)Math.Min(BuffSize, read - size));
                if (count == 0)
                    throw new EndOfStreamException();
                stream.Write(buff, 0, count);
                read += count;
            }
        }

        private byte[] GetBytes(int n)
        {
            var buff = new byte[n];
            var read = 0;

            while (read < n)
            {
                var count = m_Stream.Read(buff, read, n - read);
                if (count == 0)
                    throw new EndOfStreamException();
                read += count;
            }

            return buff;
        }
    }
}
