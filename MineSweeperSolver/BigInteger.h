#pragma once
#include "stdafx.h"
#include <vector>

class
    DLL_API BigInteger
{
public:
    BigInteger();
    // ReSharper disable once CppNonExplicitConvertingConstructor
    BigInteger(int val);
    ~BigInteger();

    BigInteger &operator+=(const BigInteger &other);
    BigInteger &operator*=(BYTE other);
    BigInteger &operator*=(const BigInteger &other);
    friend DLL_API BigInteger operator+(const BigInteger &one, const BigInteger &another);
    friend DLL_API BigInteger operator*(const BigInteger &one, const BigInteger &another);
    friend DLL_API double operator/(const BigInteger &one, const BigInteger &another);
    friend DLL_API bool operator<(const BigInteger &lhs, const BigInteger &rhs);
    friend DLL_API bool operator>(const BigInteger &lhs, const BigInteger &rhs);
    friend DLL_API bool operator<=(const BigInteger &lhs, const BigInteger &rhs);
    friend DLL_API bool operator>=(const BigInteger &lhs, const BigInteger &rhs);
    friend DLL_API bool operator==(const BigInteger &lhs, const BigInteger &rhs);
    friend DLL_API bool operator!=(const BigInteger &lhs, const BigInteger &rhs);

    explicit operator size_t() const;

    double GetSignificand() const;
    int GetExponent() const;
    double Log2() const;
private:
    std::vector<BYTE> m_Data;
    int m_Bits;

    void UpdateBits();
};

DLL_API BigInteger operator+(const BigInteger &one, const BigInteger &another);
DLL_API BigInteger operator*(const BigInteger &one, const BigInteger &another);
DLL_API double operator/(const BigInteger &one, const BigInteger &another);
DLL_API bool operator<(const BigInteger &lhs, const BigInteger &rhs);
DLL_API bool operator>(const BigInteger &lhs, const BigInteger &rhs);
DLL_API bool operator<=(const BigInteger &lhs, const BigInteger &rhs);
DLL_API bool operator>=(const BigInteger &lhs, const BigInteger &rhs);
DLL_API bool operator==(const BigInteger &lhs, const BigInteger &rhs);
DLL_API bool operator!=(const BigInteger &lhs, const BigInteger &rhs);
