#pragma once
#include "stdafx.h"

class BaseBaseWorker
{
public:
    virtual ~BaseBaseWorker();

    size_t Repetition;

    std::vector<size_t> Result;

    void Process();

    void Cancel();

protected:
    BaseBaseWorker();

    CancellationToken m_Cancel;

    virtual void Gather(size_t res) = 0;
    virtual void Prepare();
    virtual size_t ProcessOne() = 0;
};
