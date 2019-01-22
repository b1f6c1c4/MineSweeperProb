#pragma once
#include "stdafx.h"
#include "../MWLiteFundamental/BLL/BaseWorkerT.h"
#include "AdapterWorker.h"

class WorkerT : public BaseWorkerT, public AdapterWorker
{
public:
    WorkerT();

    NO_COPY(WorkerT);
    NO_MOVE(WorkerT);

protected:
    void Prepare() override;
    size_t ProcessOne() override;
};
