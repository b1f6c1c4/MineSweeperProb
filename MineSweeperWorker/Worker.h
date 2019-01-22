#pragma once
#include "stdafx.h"
#include "BaseWorker.h"
#include "AdapterWorker.h"

class Worker : public BaseWorker, public AdapterWorker
{
public:
    Worker();

    NO_COPY(Worker);
    NO_MOVE(Worker);

protected:
    void Prepare() override;
    size_t ProcessOne() override;
};
