#include "stdafx.h"
#include "GameMgr.h"
#include "random.h"

BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD ul_reason_for_call,
                      LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        RandomInit();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        RandomClose();
        break;
    }
    return TRUE;
}

extern "C" DLL_API GameMgr *CreateGameMgr(int width, int height, int totalMines)
{
    return new GameMgr(width, height, totalMines);
}

extern "C" DLL_API void DisposeGameMgr(GameMgr *mgr)
{
    if (mgr != nullptr)
        delete mgr;
}

extern "C" DLL_API void OpenBlock(GameMgr *mgr, int x, int y)
{
    mgr->OpenBlock(x, y);
}

extern "C" DLL_API int GetGameStatus(GameMgr *mgr)
{
    if (mgr->IsStarted())
        return 0;
    return mgr->IsSucceed() ? 1 : -1;
}

extern "C" DLL_API double GetBits(GameMgr *mgr)
{
    return mgr->GetBits();
}

extern "C" DLL_API int GetBlockFlag(GameMgr *mgr, int x, int y)
{
    auto &prop = mgr->GetBlockProperty(x, y);
    if (mgr->IsStarted())
    {
        if (prop.IsOpen)
            return prop.Degree;
        return Unknown;
    }
    if (prop.IsOpen)
    {
        if (prop.IsMine)
            return Unknown | Mine;
        return prop.Degree;
    }
    if (prop.IsMine)
        return Mine;
    return Unknown;
}

extern "C" DLL_API const double *GetProbabilities(GameMgr *mgr)
{
    return mgr->GetSolver().GetProbabilities();
}

extern "C" DLL_API const BlockStatus *GetBlockStatuses(GameMgr *mgr)
{
    return mgr->GetSolver().GetBlockStatuses();
}

extern "C" DLL_API void Solve(GameMgr *mgr, bool withProb)
{
    mgr->GetSolver().Solve(withProb);
}
