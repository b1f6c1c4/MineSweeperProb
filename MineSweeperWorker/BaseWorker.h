#pragma once
#include "stdafx.h"
#include "BaseBaseWorker.h"

class BaseWorker : public BaseBaseWorker
{
public:
    void Gather(size_t res) override;
};
