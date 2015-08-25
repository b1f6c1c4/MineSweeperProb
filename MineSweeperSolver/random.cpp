#include "random.h"
#include <wincrypt.h>
#include <cmath>
#include <assert.h>

#pragma comment(lib, "advapi32.lib")

static HCRYPTPROV hProvider = 0;

void RandomInit()
{
    DWORD result = CryptAcquireContextW(&hProvider, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT);
    assert(result);
}

void RandomClose()
{
    CryptReleaseContext(hProvider, 0);
}

BYTE *Random(int length)
{
    auto pbBuffer = new BYTE[length];
    DWORD result = CryptGenRandom(hProvider, length, pbBuffer);
    assert(result);
    return pbBuffer;
}

int RandomInteger(int maxExclusive)
{
    auto bits = 0;
    auto v = maxExclusive - 1;
    while (v != 0)
    {
        ++bits;
        v >>= 2;
    }
    auto lng = (bits + 7) / 8;
    auto res = maxExclusive;
    while (res >= maxExclusive)
    {
        auto rnd = Random(lng);
        res = 0;
        for (auto i = 0; i < lng - 1; ++i)
        {
            res <<= 8;
            res |= rnd[i];
        }
        res <<= 8;
        res |= rnd[lng - 1] & ((1 << (bits - (lng - 1) * 8)) - 1);
        delete[] rnd;
    }
    return res;
}
