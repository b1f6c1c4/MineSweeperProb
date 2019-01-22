#include "BaseBaseWorker.h"

BaseBaseWorker::BaseBaseWorker() : Repetition(0) {}

BaseBaseWorker::~BaseBaseWorker() { }

void BaseBaseWorker::Process()
{
    Prepare();

    while (Repetition > 0)
    {
        if (m_Cancel.IsCancelled())
            break;

        auto res = ProcessOne();

        if (m_Cancel.IsCancelled())
            break;

        Gather(res);

        Repetition--;
    }
}

void BaseBaseWorker::Cancel()
{
    m_Cancel.Cancel();
}

void BaseBaseWorker::Prepare() { }
