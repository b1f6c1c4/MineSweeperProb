#include "BigInteger.h"

BigInteger::BigInteger() : m_Data(1, 0), m_Bits(0) {}

BigInteger::BigInteger(int val)
{
    m_Data.reserve(sizeof(val));

    auto b = false;
    unsigned int v = abs(val);
    while (v != 0)
    {
        if (!(v & 0x80))
            b = true;
        m_Data.push_back(v % 0x100);
        v /= 0x100;
    }

    UpdateBits();
}

BigInteger::~BigInteger() {}

BigInteger &BigInteger::operator+=(const BigInteger &other)
{
    if (other.m_Data.size() > m_Data.size())
        m_Data.resize(other.m_Data.size(), 0);

    byte c = 0;
    auto it = m_Data.begin();
    auto itO = other.m_Data.begin();
    while (itO != other.m_Data.end())
    {
        auto nextC = *it & *itO & 0x80;
        *it += *itO;
        *it += c;
        c = nextC ? 1 : 0;
        ++it , ++itO;
    }
    if (m_Data.back() & 0x80)
        m_Data.push_back(0);
    UpdateBits();
    return *this;
}

BigInteger &BigInteger::operator*=(byte other)
{
    unsigned short m = other;

    byte c = 0;
    auto it = m_Data.begin();
    while (it != m_Data.end())
    {
        unsigned short val = static_cast<unsigned short>(*it) * m;
        *it = static_cast<byte>(val);
        auto nextC = *it & c & 0x80;
        *it += c;
        c = (val >> 8) + (nextC ? 1 : 0);
        ++it;
    }
    if (m_Data.back() & 0x80)
        m_Data.push_back(0);

    UpdateBits();
    return *this;
}

BigInteger &BigInteger::operator*=(const BigInteger &other)
{
    std::vector<byte> orig((m_Bits + other.m_Bits) / 8 + 1);
    orig.swap(m_Data);


    auto shift = 0;
    for (auto itO = other.m_Data.begin(); itO != other.m_Data.end(); ++itO)
    {
        if (*itO == 0)
            continue;

        unsigned short m = *itO;

        byte c = 0;
        for (auto it = orig.begin(), itR = m_Data.begin() + shift; it != orig.end(); ++it , ++itR)
        {
            unsigned short val = static_cast<unsigned short>(*it) * m;
            *it = static_cast<byte>(val);
            auto nextC = *it & c & 0x80;
            *it += c;
            c = (val >> 8) + (nextC ? 1 : 0);
        }
    }

    UpdateBits();
    return *this;
}

double BigInteger::GetSignificand() const
{
    double significand = 1;
    auto p = reinterpret_cast<byte *>(&significand);
    auto flag = *p == 0x3F;
    byte lst[7];

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
    return m_Bits + log2(GetSignificand());
}

void BigInteger::UpdateBits()
{
    m_Bits = 8 * (m_Data.size() - 1);
    auto v = m_Data.back();
    while (v != 0)
    {
        ++m_Bits;
        v /= 2;
    }
}

BigInteger operator+(const BigInteger &one, const BigInteger &another)
{
    auto b = BigInteger(one);
    b += another;
    return b;
}

BigInteger operator*(const BigInteger &one, const BigInteger &another)
{
    auto b = BigInteger(one);
    b *= another;
    return b;
}

double operator/(const BigInteger &one, const BigInteger &another)
{
    return one.GetSignificand() / another.GetSignificand() * pow(2, one.m_Bits - another.m_Bits);
}

bool operator==(const BigInteger &lhs, const BigInteger &rhs)
{
    if (lhs.m_Bits != rhs.m_Bits)
        return false;

    for (auto i = 0; i < lhs.m_Data.size(); ++i)
        if (lhs.m_Data[i] != rhs.m_Data[i])
            return false;

    return true;
}

bool operator!=(const BigInteger &lhs, const BigInteger &rhs)
{
    return !(lhs == rhs);
}
