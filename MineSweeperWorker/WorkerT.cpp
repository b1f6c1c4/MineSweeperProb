#include "WorkerT.h"
#include <chrono>
#include <iostream>

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

    auto res = AdapterWorker::Run();

    auto t1 = Clock::now();
    auto dur = t1 - t0;
    std::cout << dur.count() << std::endl;

    return res;
}
