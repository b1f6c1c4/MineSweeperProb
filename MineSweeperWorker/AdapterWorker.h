#pragma once
#include "stdafx.h"
#include <random>

class AdapterWorker
{
public:
    virtual ~AdapterWorker() { }

    Configuration Config;

    NO_COPY(AdapterWorker);
    NO_MOVE(AdapterWorker);

protected:
    AdapterWorker();

    size_t Run();
    void Cache() const;

private:
    std::mt19937 m_Random;
};
