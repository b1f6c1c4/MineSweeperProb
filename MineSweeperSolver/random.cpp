#include "random.h"
#include <random>

static std::mt19937_64 random;

int RandomInteger(int maxExclusive)
{
    std::uniform_int_distribution<int> dist(0, maxExclusive - 1);
    return dist(random);
}
