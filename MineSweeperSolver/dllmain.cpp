#include "stdafx.h"
#include "GameMgr.h"
#include "random.h"
#include "Drainer.h"

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

struct
    DLL_API GameStatus
{
    int TotalWidth, TotalHeight, TotalBlocks, TotalMines;
    bool Started, Succeed;
    double Bits, AllBits;
    int ToOpen;

    const BlockProperty *BlockProperties;
    const BlockStatus *InferredStatus;
	const double *Probabilities;
	const double *DrainProbabilities;

    int BestBlockCount;
    const Block *BestBlocks;
    int PreferredBlockCount;
    const Block *PreferredBlocks;
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

extern "C" DLL_API void Solve(GameMgr *mgr, SolvingState maxDepth)
{
    mgr->Solve(maxDepth, false);
}

extern "C" DLL_API GameStatus *GetGameStatus(GameMgr *mgr)
{
    auto st = new GameStatus;

#define ST(name) st->name = mgr->Get##name();
    ST(TotalWidth);
    ST(TotalHeight);
    ST(TotalMines);
    ST(Started);
    ST(Succeed);
    ST(Bits);
    ST(AllBits);
    ST(ToOpen);
    ST(BlockProperties);
    ST(BestBlockCount);
    ST(BestBlocks);
    ST(PreferredBlockCount);
    ST(PreferredBlocks);

    st->TotalBlocks = st->TotalWidth * st->TotalHeight;
    st->InferredStatus = mgr->GetSolver().GetBlockStatuses();
	st->Probabilities = mgr->GetSolver().GetProbabilities();
	if (mgr->GetDrainer() != nullptr)
		st->DrainProbabilities = mgr->GetDrainer()->GetBestProbabilities();
	else
		st->DrainProbabilities = nullptr;

    return st;
}

extern "C" DLL_API void ReleaseGameStatus(GameStatus *status)
{
    if (status != nullptr)
        delete status;
}

extern "C" DLL_API bool SemiAutomaticStep(GameMgr *mgr, SolvingState maxDepth)
{
    return mgr->SemiAutomaticStep(maxDepth);
}

extern "C" DLL_API bool SemiAutomatic(GameMgr *mgr, SolvingState maxDepth)
{
    return mgr->SemiAutomatic(maxDepth);
}

extern "C" DLL_API void AutomaticStep(GameMgr *mgr)
{
    mgr->AutomaticStep(SolvingState::Reduce | SolvingState::Overlap | SolvingState::Probability | SolvingState::ZeroProb);
}

extern "C" DLL_API void Automatic(GameMgr *mgr)
{
    mgr->Automatic();
}

extern "C" DLL_API void EnableDrainer(GameMgr *mgr)
{
    mgr->EnableDrainer();
}

extern "C" DLL_API void OpenOptimalBlocks(GameMgr *mgr)
{
    mgr->OpenOptimalBlocks();
}
