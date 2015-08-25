#include "BigInteger.h"
#include <assert.h>

BigInteger::BigInteger() : m_Data(1, 0), m_Bits(0) {}

BigInteger::BigInteger(int val)
{
    m_Data.reserve(sizeof(val));

    auto b = false;
    unsigned int v = abs(val);
    while (v != 0)
    {
        if (v & 0x80)
            b = true;
        m_Data.push_back(v % 0x100);
        v /= 0x100;
    }
    if (b || m_Data.empty())
        m_Data.push_back(0);

    UpdateBits();
}

BigInteger::~BigInteger() {}

BigInteger &BigInteger::operator+=(const BigInteger &other)
{
    if (other.m_Data.size() > m_Data.size())
        m_Data.resize(other.m_Data.size(), 0);

    BYTE c = 0;
    auto it = m_Data.begin();
    auto itO = other.m_Data.begin();
    while (itO != other.m_Data.end())
    {
        unsigned short val = *it + *itO + c;
        *it = static_cast<BYTE>(val);
        c = static_cast<BYTE>(val >> 8);
        ++it , ++itO;
    }
    if (m_Data.back() & 0x80)
        m_Data.push_back(0);
    UpdateBits();
    return *this;
}

BigInteger &BigInteger::operator*=(BYTE other)
{
    unsigned short m = other;

    BYTE c = 0;
    auto it = m_Data.begin();
    while (it != m_Data.end())
    {
        unsigned short val = *it * m + c;
        *it = static_cast<BYTE>(val);
        c = static_cast<BYTE>(val >> 8);
        ++it;
    }
    if (m_Data.back() & 0x80)
        m_Data.push_back(0);

    UpdateBits();
    return *this;
}

BigInteger &BigInteger::operator*=(const BigInteger &other)
{
    std::vector<BYTE> orig((m_Bits + other.m_Bits) / 8 + 1);
    orig.swap(m_Data);


    auto shift = 0;
    for (auto itO = other.m_Data.begin(); itO != other.m_Data.end(); ++itO, ++shift)
    {
        if (*itO == 0)
            continue;

        unsigned short m = *itO;

        BYTE c = 0;
        auto itR = m_Data.begin() + shift;
        for (auto it = orig.begin(); it != orig.end(); ++it , ++itR)
        {
            unsigned short val = *it * m + *itR + c;
            *itR = static_cast<BYTE>(val);
            c = static_cast<BYTE>(val >> 8);
        }
        while (c != 0)
        {
            if (itR == m_Data.end())
            {
                m_Data.push_back(c);
                break;
            }
            unsigned short val = *itR + c;
            *itR++ = static_cast<BYTE>(val);
            c = static_cast<BYTE>(val >> 8);
        }
    }

    UpdateBits();
    return *this;
}

double BigInteger::GetSignificand() const
{
    if (*this == 0)
        return 0;
    double significand = 1;
    auto p = reinterpret_cast<BYTE *>(&significand);
    auto flag = *p == 0x3F;
    BYTE lst[7];

    if (!flag)
        for (auto i = 0; i < 7 - i; ++i)
            std::swap(p[i], p[7 - i]);

    auto id = 0;
    for (auto it = m_Data.rbegin(); it != m_Data.rend() && id < 7; ++it , ++id)
    {
        if (it == m_Data.rbegin() && *it == 0)
        {
            --id;
            continue;
        }
        lst[id] = *it;
    }
    while (id < 7)
        lst[id++] = 0x00;

    auto shift = 0;
    auto b = lst[0];
    while (!(b & 0x80))
    {
        b <<= 1;
        shift++;
    }

    if (shift <= 3)
    {
        p[1] |= 0x0F & (lst[0] >> (3 - shift));
        for (auto i = 0; i < 6; ++i)
            p[i + 2] = (lst[i] << (5 + shift)) | (lst[i + 1] >> (3 - shift));
    }
    else
    {
        p[1] |= 0x0F & (lst[0] << (shift - 3)) | lst[1] >> (11 - shift);
        for (auto i = 1; i < 6; ++i)
            p[i + 1] = (lst[i] << (shift - 3)) | (lst[i + 1] >> (11 - shift));
        p[7] = lst[6] << (shift - 3);
    }

    if (!flag)
        for (auto i = 0; i < 7 - i; ++i)
            std::swap(p[i], p[7 - i]);

    return significand;
}

int BigInteger::GetExponent() const
{
    return m_Bits;
}

double BigInteger::Log2() const
{
    return m_Bits + log2(GetSignificand()) - 1;
}

void BigInteger::UpdateBits()
{
    while (m_Data.size() >= 2 && m_Data.back() == 0 && *++m_Data.rbegin() == 0)
        m_Data.pop_back();

    m_Bits = 8 * (m_Data.size() - 1);
    auto v = m_Data.back();
    while (v != 0)
    {
        ++m_Bits;
        v /= 2;
    }
}

DLL_API BigInteger operator+(const BigInteger &one, const BigInteger &another)
{
    auto b = BigInteger(one);
    b += another;
    return b;
}

DLL_API BigInteger operator*(const BigInteger &one, const BigInteger &another)
{
    auto b = BigInteger(one);
    b *= another;
    return b;
}

DLL_API double operator/(const BigInteger &one, const BigInteger &another)
{
    return one.GetSignificand() / another.GetSignificand() * pow(2, one.m_Bits - another.m_Bits);
}

DLL_API bool operator<(const BigInteger& lhs, const BigInteger& rhs)
{
    if (lhs.m_Bits < rhs.m_Bits)
        return true;
    if (lhs.m_Bits > rhs.m_Bits)
        return false;
    auto itL = lhs.m_Data.rbegin();
    auto itR = rhs.m_Data.rbegin();
    while (itL != lhs.m_Data.rend())
    {
        if (*itL < *itR)
            return true;
        if (*itL > *itR)
            return false;
        ++itL, ++itR;
    }
    return false;
}

DLL_API bool operator>(const BigInteger& lhs, const BigInteger& rhs)
{
    return rhs < lhs;
}

DLL_API bool operator<=(const BigInteger& lhs, const BigInteger& rhs)
{
    return !(rhs < lhs);
}

DLL_API bool operator>=(const BigInteger& lhs, const BigInteger& rhs)
{
    return !(lhs < rhs);
}

DLL_API bool operator==(const BigInteger &lhs, const BigInteger &rhs)
{
    if (lhs.m_Bits != rhs.m_Bits)
        return false;

    for (auto i = 0; i < lhs.m_Data.size(); ++i)
        if (lhs.m_Data[i] != rhs.m_Data[i])
            return false;

    return true;
}

DLL_API bool operator!=(const BigInteger &lhs, const BigInteger &rhs)
{
    return !(lhs == rhs);
}