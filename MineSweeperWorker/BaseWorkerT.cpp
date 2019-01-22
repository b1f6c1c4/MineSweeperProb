#include "BaseWorkerT.h"

void BaseWorkerT::Prepare()
{
    Result.reserve(Result.size() + Repetition);
}

void BaseWorkerT::Gather(size_t res)
{
    Result.push_back(res);
}
