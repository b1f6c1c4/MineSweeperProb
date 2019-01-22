#include "Worker.h"

Worker::Worker() { }

void Worker::Prepare()
{
    BaseWorker::Prepare();
    AdapterWorker::Cache();
}

size_t Worker::ProcessOne()
{
    return AdapterWorker::Run();
}
