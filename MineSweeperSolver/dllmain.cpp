#include "stdafx.h"
#include "GameMgr.h"
#include "random.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
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

struct DLL_API GameStatus
{
    int TotalWidth, TotalHeight, TotalBlocks, TotalMines;
    bool Started, Succeed;
    double Bits, AllBits;
    int ToOpen;

    const BlockProperty *Blocks;
    const BlockStatus *InferredStatus;
    const double *Probabilities;
};

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

extern "C" DLL_API void Solve(GameMgr *mgr, bool withProb)
{
    mgr->GetSolver().Solve(withProb);
}

extern "C" DLL_API GameStatus *GetGameStatus(GameMgr *mgr)
{
    auto st = new GameStatus;
    st->TotalWidth = mgr->GetTotalWidth();
    st->TotalHeight = mgr->GetTotalHeight();
    st->TotalBlocks = st->TotalWidth * st->TotalHeight;
    st->TotalMines = mgr->GetTotalMines();
    st->Started = mgr->IsStarted();
    st->Succeed = mgr->IsSucceed();
    st->Bits = mgr->GetBits();
    st->AllBits = mgr->GetAllBits();
    st->ToOpen = mgr->GetToOpen();
    st->Blocks = mgr->GetBlockProperties();
    st->InferredStatus = mgr->GetSolver().GetBlockStatuses();
    st->Probabilities = mgr->GetSolver().GetProbabilities();

    return st;
}

extern "C" DLL_API void ReleaseGameStatus(GameStatus *status)
{
    if (status != nullptr)
        delete status;
}
