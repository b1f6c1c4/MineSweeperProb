#include "stdafx.h"
#include <fstream>
#include <sstream>
#include <thread>
#include <mutex>
#include "../MineSweeperSolver/GameMgr.h"
#include "../MineSweeperSolver/BinomialHelper.h"
#include <WinSock2.h>
#include <ws2tcpip.h>

#pragma comment (lib, "Ws2_32.lib")
#include <iostream>

std::mutex mtx;
std::vector<std::thread> m_Threads;

int numTasks;
size_t *cert;
size_t *dense;
volatile size_t restT, totalTT;
volatile size_t *rest;
volatile size_t *succeed, *succeedT;
volatile size_t *total, *totalT;

bool ProcessID(int id)
{
    GameMgr mgr(30, 16, 99);
    mgr.DrainCriterion = cert[id];
    mgr.OpenBlock(0, 0);
    mgr.Automatic();
    return mgr.GetSucceed();
}

void Process(int id, bool (*fun)(int))
{
    for (;; ++id)
    {
        id %= numTasks;
        auto d = dense[id];
        while (d-- > 0)
        {
            {
                std::unique_lock<std::mutex> lock(mtx);
                if (restT == 0)
                    return;

                if (rest[id] == 0)
                    continue;

                --restT;
                --rest[id];
            }

            auto res = fun(id);

            {
                std::unique_lock<std::mutex> lock(mtx);
                if (res)
                    ++succeed[id];
                ++total[id];
            }
        }
    }
}

bool Launch(std::istringstream &sin)
{
    if (!m_Threads.empty())
        return false;

    int thrs;
    sin >> thrs >> numTasks;
#undef max
    auto min = std::numeric_limits<size_t>().max();
    totalTT = restT = 0;
    dense = new size_t[numTasks];
    cert = new size_t[numTasks];
    rest = new size_t[numTasks];
    succeed = new size_t[numTasks];
    total = new size_t[numTasks];
    succeedT = new size_t[numTasks];
    totalT = new size_t[numTasks];
    for (auto i = 0; i < numTasks; ++i)
    {
        int r;
        sin >> cert[i] >> r;
        dense[i] = r;
        if (dense[i] < min)
            min = dense[i];
        rest[i] = r;
        restT += r;
        succeedT[i] = succeed[i] = 0;
        totalT[i] = total[i] = 0;
    }

    size_t cnt = 0;
    while (min != 0)
    {
        min >>= 1;
        ++cnt;
    }
    --cnt;
    for (auto i = 0; i < numTasks; ++i)
        dense[i] >>= cnt;

    while (thrs-- > 0)
        m_Threads.emplace_back(&Process, thrs, &ProcessID);

    return true;
}

int Save(std::ostringstream *sout)
{
    std::ofstream fout("output.txt", std::ios::app);
    size_t rT;
    {
        std::unique_lock<std::mutex> lock(mtx);

        for (auto i = 0; i < numTasks; ++i)
        {
            if (total[i] == 0)
                continue;

            fout << cert[i] << " " << succeed[i] << " " << total[i] << std::endl;
            if (sout != nullptr)
                *sout << cert[i] << " " << succeed[i] << " " << total[i] << std::endl;
            succeedT[i] += succeed[i];
            totalT[i] += total[i];
            totalTT += total[i];
            succeed[i] = 0;
            total[i] = 0;
        }
        rT = restT;
    }
    fout.close();
    return rT;
}

void Clear()
{
    if (numTasks != 0)
    {
        delete[] dense;
        delete[] cert;
        delete[] rest;
        delete[] succeed;
        delete[] total;
        delete[] succeedT;
        delete[] totalT;
    }
}

int main()
{
    CacheBinomials(30 * 16, 99);

    numTasks = 0;
    restT = 0;

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return 1;

    addrinfo *result = nullptr, hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    hints.ai_flags = AI_PASSIVE;
    if (getaddrinfo(nullptr, "27015", &hints, &result) != 0)
    {
        WSACleanup();
        return 1;
    }

    auto theSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (theSocket == INVALID_SOCKET)
    {
        std::cout << WSAGetLastError() << std::endl;
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    if (bind(theSocket, result->ai_addr, result->ai_addrlen) == SOCKET_ERROR)
    {
        std::cout << WSAGetLastError() << std::endl;
        freeaddrinfo(result);
        closesocket(theSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

#define BUFF_LENGTH 2048
    char recvBuff[BUFF_LENGTH];
    while (true)
    {
        std::cout << "Wait for command: ";

        sockaddr addr;
        int addrLen = sizeof(addr);
        auto ret = recvfrom(theSocket, recvBuff, BUFF_LENGTH, 0, &addr, &addrLen);
        if (ret == 0)
            continue;
        if (ret < 0)
        {
            Save(nullptr);
            closesocket(theSocket);
            WSACleanup();
            return 1;
        }

        recvBuff[ret] = '\0';

        std::cout << recvBuff << std::endl;

        std::istringstream sin(recvBuff);

        std::string action;
        sin >> action;
        if (action == "launch")
        {
            std::ostringstream sout;
            if (Launch(sin))
                sout << "Launched " << m_Threads.size() << " threads";
            else
                sout << "Already launched";
            auto str = sout.str();
            if (sendto(theSocket, str.c_str(), str.length(), 0, &addr, addrLen) == SOCKET_ERROR)
            {
                std::cout << WSAGetLastError() << std::endl;
                Save(nullptr);
                closesocket(theSocket);
                WSACleanup();
                system("pause");
                return 1;
            }
            continue;
        }
        if (action == "save")
        {
            std::ostringstream sout;
            if (numTasks == 0)
                sout << "Not yet launched";
            else
            {
                auto rT = Save(&sout);
                if (rT != 0)
                    sout << "Resume " << rT << " tests";
                else
                    sout << "Finished";
            }
            auto str = sout.str();
            if (sendto(theSocket, str.c_str(), str.length(), 0, &addr, addrLen) == SOCKET_ERROR)
            {
                std::cout << WSAGetLastError() << std::endl;
                Save(nullptr);
                closesocket(theSocket);
                WSACleanup();
                system("pause");
                return 1;
            }
            continue;
        }
        if (action == "reset")
        {
            std::ostringstream sout;
            if (numTasks == 0)
                sout << "Not yet launched";
            else
            {
                size_t rT;
                {
                    std::unique_lock<std::mutex> lock(mtx);
                    rT = restT;
                }
                if (rT != 0)
                    sout << "Not yet finished";
                else
                {
                    for (auto &th : m_Threads)
                        th.join();
                    m_Threads.clear();
                    Clear();
                    numTasks = 0;
                    sout << "Reset OK";
                }
            }
            auto str = sout.str();
            if (sendto(theSocket, str.c_str(), str.length(), 0, &addr, addrLen) == SOCKET_ERROR)
            {
                std::cout << WSAGetLastError() << std::endl;
                Save(nullptr);
                closesocket(theSocket);
                WSACleanup();
                system("pause");
                return 1;
            }
            continue;
        }
        if (action == "terminate")
        {
            std::ostringstream sout;
            if (numTasks == 0)
                sout << "Not yet launched, terminating";
            else
            {
                auto rT = Save(&sout);
                sout << "Resume " << rT << " tests, terminating";
            }
            auto str = sout.str();
            if (sendto(theSocket, str.c_str(), str.length(), 0, &addr, addrLen) == SOCKET_ERROR)
            {
                std::cout << WSAGetLastError() << std::endl;
                Save(nullptr);
                closesocket(theSocket);
                WSACleanup();
                system("pause");
                return 1;
            }
            Clear();
            return 0;
        }
        {
            std::ostringstream sout;
            sout << "Invalid command";
            auto str = sout.str();
            if (sendto(theSocket, str.c_str(), str.length(), 0, &addr, addrLen) == SOCKET_ERROR)
            {
                std::cout << WSAGetLastError() << std::endl;
                Save(nullptr);
                closesocket(theSocket);
                WSACleanup();
                system("pause");
                return 1;
            }
        }
    }
    
    system("pause");
    return 0;
}
