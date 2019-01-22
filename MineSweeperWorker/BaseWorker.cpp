#include "BaseWorker.h"

void BaseWorker::Gather(size_t res)
{
    if (res >= Result.size())
        Result.resize(res + 1, 0);
    Result[res]++;
}
