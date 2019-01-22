#pragma once
#include "stdafx.h"
#include "BaseBaseWorker.h"

class BaseWorkerT : public BaseBaseWorker
{
public:
    void Gather(size_t res) override;

protected:
    void Prepare() override;
};
