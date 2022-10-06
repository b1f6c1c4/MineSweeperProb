#pragma once

#include "Strategies.h"

struct Configuration : Strategy // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    int Width;
    int Height;

    int TotalMines;

    bool IsSNR;
};

/* String-to-configuration conversion. */
Configuration parse(const char *hsh);

/* Full-auto run, return if succeeded.
 *
 * Note: SeedEngine() and cache() must be called before.
 *
 * Note: This function is thread-safe.
 */
bool run(const Configuration &Config);

/* Pre-compute binomials, which are used in Solvers.
 *
 * Note: This function is NOT thread-safe.
 */
void cache(const Configuration &Config);
