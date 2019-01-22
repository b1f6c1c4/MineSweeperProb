#include "stdafx.h"
#include <windows.h>

#include "Worker.h"
#include "WorkerT.h"

void SetupAdapterWorker(AdapterWorker &worker, RawConfiguration config)
{
    worker.Config.Width = config.Width;
    worker.Config.Height = config.Height;
    worker.Config.TotalMines = config.TotalMines;
    worker.Config.InitialPositionSpecified = config.InitialPosition >= 0;
    worker.Config.Index = config.InitialPosition;
    worker.Config.Logic = config.Logic;
    worker.Config.HeuristicEnabled = config.HeuristicEnabled;
    worker.Config.DecisionTree.resize(config.DecisionTreeLen);
    if (config.DecisionTreeLen > 0)
        memcpy(&*worker.Config.DecisionTree.begin(), config.DecisionTree, config.DecisionTreeLen * sizeof(HeuristicMethod));
    worker.Config.ExhaustEnabled = config.ExhaustEnabled;
    worker.Config.ExhaustCriterion = config.ExhaustCriterion;
    worker.Config.PruningEnabled = false;
    worker.Config.PruningCriterion = 0;
}

void SetupBaseBaseWorker(BaseBaseWorker &worker, size_t repetition)
{
    worker.Repetition = repetition;
}

extern "C"
{
    ADAPTER_DLL_API BaseBaseWorker *MakeWorker(RawConfiguration config, size_t repetition)
    {
        auto worker = new Worker;
        SetupAdapterWorker(*worker, config);
        SetupBaseBaseWorker(*worker, repetition);
        return worker;
    }

    ADAPTER_DLL_API BaseBaseWorker *MakeWorkerT(RawConfiguration config, size_t repetition)
    {
        auto worker = new WorkerT;
        SetupAdapterWorker(*worker, config);
        SetupBaseBaseWorker(*worker, repetition);
        return worker;
    }

    ADAPTER_DLL_API size_t *Run(BaseBaseWorker *worker, size_t &len)
    {
        worker->Process();

        len = worker->Result.size();
        auto res = new size_t[len];
        memcpy(res, &*worker->Result.begin(), len * sizeof(size_t));
        return res;
    }

    ADAPTER_DLL_API void CancelWorker(BaseBaseWorker *ptr)
    {
        ptr->Cancel();
    }

    ADAPTER_DLL_API void DisposeWorker(BaseBaseWorker *ptr)
    {
        if (ptr != nullptr)
            delete[] ptr;
    }

    ADAPTER_DLL_API void DisposeResult(size_t *ptr)
    {
        if (ptr != nullptr)
            delete[] ptr;
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
