#pragma once

#include "Strategies.h"

struct Configuration : Strategy // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    int Width;
    int Height;

    int TotalMines;

    bool IsSNR;
};

Configuration parse(const char *hsh);
bool run(const Configuration &Config);
void cache(const Configuration &Config);
