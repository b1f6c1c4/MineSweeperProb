#include "StreamChuck.h"

#pragma comment (lib, "Ws2_32.lib")

bool GetBytes(SOCKET, char *, __int32);

bool PutPackage(SOCKET client, const char *buff, __int32 len)
{
    auto tmp = htonll(len);
    auto lng = reinterpret_cast<char *>(&tmp);

    if (send(client, lng, sizeof(__int64), 0) == SOCKET_ERROR)
        return false;

    if (send(client, buff, len, 0) == SOCKET_ERROR)
        return false;

    return true;
}

bool GetPackage(SOCKET client, char *&buff, __int32 &len)
{
    __int64 tmp;
    auto lng = reinterpret_cast<char *>(&tmp);

    buff = nullptr;
    len = -1;

    if (!GetBytes(client, lng, sizeof(__int64)))
        return false;

    len = ntohll(tmp);
    buff = new char[len];

    if (!GetBytes(client, buff, len))
    {
        delete[] buff;
        buff = nullptr;
        len = -1;
        return false;
    }

    return true;
}

bool GetBytes(SOCKET client, char *buff, __int32 n)
{
    auto read = static_cast<__int32>(0);

    while (read < n)
    {
        auto count = recv(client, buff + read, n - read, 0);
        if (count == 0)
            return false;
        read += count;
    }

    return true;
}
