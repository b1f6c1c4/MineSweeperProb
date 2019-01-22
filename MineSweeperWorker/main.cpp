#include "stdafx.h"
#include "Worker.h"
#include "WorkerT.h"
#include <cstring>

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
        std::memcpy(&*worker.Config.DecisionTree.begin(), config.DecisionTree, config.DecisionTreeLen * sizeof(HeuristicMethod));
    worker.Config.ExhaustEnabled = config.ExhaustEnabled;
    worker.Config.ExhaustCriterion = config.ExhaustCriterion;
    worker.Config.PruningEnabled = false;
    worker.Config.PruningCriterion = 0;
}

void SetupBaseBaseWorker(BaseBaseWorker &worker, size_t repetition)
{
    worker.Repetition = repetition;
}

BaseBaseWorker *MakeWorker(RawConfiguration config, size_t repetition)
{
    auto worker = new Worker;
    SetupAdapterWorker(*worker, config);
    SetupBaseBaseWorker(*worker, repetition);
    return worker;
}

BaseBaseWorker *MakeWorkerT(RawConfiguration config, size_t repetition)
{
    auto worker = new WorkerT;
    SetupAdapterWorker(*worker, config);
    SetupBaseBaseWorker(*worker, repetition);
    return worker;
}

size_t *Run(BaseBaseWorker *worker, size_t &len)
{
    worker->Process();

    len = worker->Result.size();
    auto res = new size_t[len];
    std::memcpy(res, &*worker->Result.begin(), len * sizeof(size_t));
    return res;
}

void CancelWorker(BaseBaseWorker *ptr)
{
    ptr->Cancel();
}

void DisposeWorker(BaseBaseWorker *ptr)
{
    if (ptr != nullptr)
        delete[] ptr;
}

void DisposeResult(size_t *ptr)
{
    if (ptr != nullptr)
        delete[] ptr;
}

int main(int argc, char **argv)
{
    return 0;
}
