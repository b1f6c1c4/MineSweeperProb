#pragma once
#include "stdafx.h"

#include <WinSock2.h>
#include <ws2tcpip.h>

bool PutPackage(SOCKET, const char *, __int32);
bool GetPackage(SOCKET, char *&, __int32 &);
