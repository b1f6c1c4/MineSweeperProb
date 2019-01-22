#include "random.h"

static std::mt19937_64 randomEngine = ProperlySeededRandomEngine<std::mt19937_64>();

int RandomInteger(int maxExclusive)
{
    std::uniform_int_distribution<int> dist(0, maxExclusive - 1);
    return dist(randomEngine);
}
