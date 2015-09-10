using System;
using System.Collections.Generic;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using Headers = System.Collections.Generic.Dictionary<string, string>;

namespace SimulatorsManager
{
    class SimpleHttpServer
    {
        private static readonly Dictionary<int, string> ResponseCodes
            = new Dictionary<int, string>
                  {
                      { 100, "Continue" },
                      { 101, "Switching Protocols" },
                      { 200, "OK" },
                      { 201, "Created" },
                      { 202, "Accepted" },
                      { 203, "Non-Authoritative Information" },
                      { 204, "No Content" },
                      { 205, "Reset Content" },
                      { 206, "Partial Content" },
                      { 300, "Multiple Choices" },
                      { 301, "Moved Permanently" },
                      { 302, "Found" },
                      { 303, "See Other" },
                      { 304, "Not Modified" },
                      { 305, "Use Proxy" },
                      { 307, "Temporary Redirect" },
                      { 400, "Bad Request" },
                      { 401, "Unauthorized" },
                      { 402, "Payment Required" },
                      { 403, "Forbidden" },
                      { 404, "Not Found" },
                      { 405, "Method Not Allowed" },
                      { 406, "Not Acceptable" },
                      { 407, "Proxy Authentication Required" },
                      { 408, "Request Timeout" },
                      { 409, "Conflict" },
                      { 410, "Gone" },
                      { 411, "Length Required" },
                      { 412, "Precondition Failed" },
                      { 413, "Request Entity Too Large" },
                      { 414, "Request-URI Too Long" },
                      { 415, "Unsupported Media Type" },
                      { 416, "Requested Range Not Satisfiable" },
                      { 417, "Expectation Failed" },
                      { 500, "Internal Server Error" },
                      { 501, "Not Implemented" },
                      { 502, "Bad Gateway" },
                      { 503, "Service Unavailable" },
                      { 504, "Gateway Timeout" },
                      { 505, "HTTP Version Not Supported" }
                  };

        // ReSharper disable once PrivateFieldCanBeConvertedToLocalVariable
        private readonly Thread m_ListenerThread;
        private readonly TcpListener m_Listener;

        public delegate HttpResponse OnHttpRequestEventHandler(HttpRequest request);

        public event OnHttpRequestEventHandler OnHttpRequest;

        public SimpleHttpServer(IPAddress ip, int port)
        {
            m_Listener = new TcpListener(ip, port);
            m_ListenerThread = new Thread(MainProcess)
            {
                IsBackground = true,
                Name = "HttpThread"
            };
            m_ListenerThread.Start();
        }

        private void MainProcess()
        {
            m_Listener.Start();
            while (true)
            {
                var tcp = m_Listener.AcceptTcpClient();
                var thr = new Thread(o => Process((TcpClient)o));
                thr.Start(tcp);
            }
            // ReSharper disable once FunctionNeverReturns
        }

        private void Process(TcpClient tcp)
        {
            try
            {
                using (var stream = tcp.GetStream())
                {
                    HttpResponse response;
                    try
                    {
                        var request = ParseRequest(stream);
                        if (OnHttpRequest == null)
                            throw new HttpException(501);
                        response = OnHttpRequest(request);
                    }
                    catch (Exception)
                    {
                        response = new HttpResponse { ResponseCode = 500 };
                    }

                    using (response)
                    {
                        WriteResponse(stream, response);
                    }

                    stream.Close();
                }
                tcp.Close();
            }
            catch (Exception)
            {
                // ignored
            }
        }

        private HttpRequest ParseRequest(Stream stream)
        {
            var request = new HttpRequest
                              {
                                  Method = Parse(stream, ParsingState.Method),
                                  Uri = Parse(stream, ParsingState.Uri),
                                  Header = new Headers(),
                                  RequestStream = stream
                              };
            while (true)
            {
                var key = Parse(stream, ParsingState.HeaderKey);
                if (string.IsNullOrEmpty(key))
                    break;
                var value = Parse(stream, ParsingState.HeaderValue);
                request.Header.Add(key, value);
            }
            return request;
        }

        private static void WriteResponse(Stream stream, HttpResponse response)
        {
            QWrite(stream, $"HTTP/1.1 {response.ResponseCode} {ResponseCodes[response.ResponseCode]}\r\n");

            if (response.Header == null)
                response.Header = new Headers();
            response.Header["Connection"] = "close";
            if (response.ResponseStream != null &&
                !response.Header.ContainsKey("Content-Length"))
                response.Header["Transfer-Encoding"] = "chunked";
            foreach (var kvp in response.Header)
                QWrite(stream, $"{kvp.Key}: {kvp.Value}\r\n");
            QWrite(stream, "\r\n");

            if (response.ResponseStream != null)
                if (response.Header.ContainsKey("Content-Length"))
                {
                    var buff = new byte[4096];
                    var rest = Convert.ToInt64(response.Header["Content-Length"]);
                    while (rest > 0)
                    {
                        var sz = rest < 4096
                                     ? response.ResponseStream.Read(buff, 0, (int)rest)
                                     : response.ResponseStream.Read(buff, 0, 4096);
                        stream.Write(buff, 0, sz);
                        rest -= sz;
                    }
                }
                else
                {
                    var buff = new byte[4096];
                    while (true)
                    {
                        var sz = response.ResponseStream.Read(buff, 0, 4096);
                        QWrite(stream, $"{sz:x}\r\n");
                        stream.Write(buff, 0, sz);
                        QWrite(stream, "\r\n");
                        if (sz == 0)
                            break;
                    }
                }
        }

        private static void QWrite(Stream stream, string str, Encoding encoding = null)
        {
            encoding = encoding ?? Encoding.UTF8;
            var data = encoding.GetBytes(str);
            stream.Write(data, 0, data.Length);
        }

        private enum ParsingState
        {
            Method,
            Uri,
            HeaderKey,
            HeaderValue
        }

        private string Parse(Stream stream, ParsingState st)
        {
            switch (st)
            {
                case ParsingState.Method:
                    {
                        var sb = new StringBuilder();
                        while (true)
                        {
                            var ch = stream.ReadByte();
                            if (ch < 0 || ch > 255)
                                throw new HttpException(400);
                            if (ch == ' ')
                                break;
                            sb.Append((char)ch);
                        }
                        switch (sb.ToString())
                        {
                            case "OPTIONS":
                            case "GET":
                            case "HEAD":
                            case "POST":
                            case "PUT":
                            case "DELETE":
                            case "TRACE":
                            case "CONNECT":
                                return sb.ToString();
                            default:
                                throw new HttpException(400);
                        }
                    }
                case ParsingState.Uri:
                    {
                        var sb = new StringBuilder();
                        while (true)
                        {
                            var ch = stream.ReadByte();
                            if (ch < 0 || ch > 255)
                                throw new HttpException(400);
                            if (ch == '\r')
                            {
                                ch = stream.ReadByte();
                                if (ch != '\n')
                                    throw new HttpException(400);
                                break;
                            }
                            sb.Append((char)ch);
                        }
                        if (sb.ToString(sb.Length - 8, 8) != "HTTP/1.1")
                            throw new HttpException(505);
                        return sb.ToString(0, sb.Length - 9);
                    }
                case ParsingState.HeaderKey:
                    {
                        var sb = new StringBuilder();
                        while (true)
                        {
                            var ch = stream.ReadByte();
                            if (ch < 0 || ch > 255)
                                throw new HttpException(400);
                            if (ch == ':')
                                break;
                            if (ch == '\r')
                            {
                                if (sb.Length != 0)
                                    throw new HttpException(400);
                                ch = stream.ReadByte();
                                if (ch != '\n')
                                    throw new HttpException(400);
                                break;
                            }
                            sb.Append((char)ch);
                        }
                        return sb.ToString();
                    }
                case ParsingState.HeaderValue:
                    {
                        var sb = new StringBuilder();
                        while (true)
                        {
                            var ch = stream.ReadByte();
                            if (ch < 0 || ch > 255)
                                throw new HttpException(400);
                            if (ch == ' ' &&
                                sb.Length == 0)
                                continue;
                            if (ch == '\r')
                            {
                                ch = stream.ReadByte();
                                if (ch != '\n')
                                    throw new HttpException(400);
                                break;
                            }
                            sb.Append((char)ch);
                        }
                        return sb.ToString();
                    }
                default:
                    throw new ApplicationException();
            }
        }
    }

    internal class HttpRequest
    {
        public string Method { get; set; }
        public string Uri { get; set; }
        public Headers Header { get; set; }
        public Stream RequestStream { get; set; }
    }

    internal class HttpResponse : IDisposable
    {
        public int ResponseCode { get; set; }
        public Headers Header { get; set; }
        public Stream ResponseStream { get; set; }
        public void Dispose() => ResponseStream?.Dispose();
    }

    internal class HttpException : Exception
    {
        public int ResponseCode { get; }

        public HttpException(int code)
        {
            ResponseCode = code;
        }
    }
}
