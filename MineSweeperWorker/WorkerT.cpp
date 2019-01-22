#include "WorkerT.h"
#include <chrono>

typedef std::chrono::high_resolution_clock Clock;

WorkerT::WorkerT() { }

void WorkerT::Prepare()
{
    BaseWorkerT::Prepare();
    AdapterWorker::Cache();
}

size_t WorkerT::ProcessOne()
{
    auto t0 = Clock::now();

    AdapterWorker::Run();

    auto t1 = Clock::now();
    auto dur = t1 - t0;
    return dur.count();
}
