#pragma once
#include "stdafx.h"
#include <vector>

class DLL_API BigInteger
{
public:
    typedef unsigned char byte;
    BigInteger();
    // ReSharper disable once CppNonExplicitConvertingConstructor
    BigInteger(int val);
    ~BigInteger();

    BigInteger &operator+=(const BigInteger &other);
    BigInteger &operator*=(byte other);
    BigInteger &operator*=(const BigInteger &other);
    friend BigInteger operator+(const BigInteger &one, const BigInteger &another);
    friend BigInteger operator*(const BigInteger &one, const BigInteger &another);
    friend double operator/(const BigInteger &one, const BigInteger &another);
    friend bool operator==(const BigInteger &lhs, const BigInteger &rhs);
    friend bool operator!=(const BigInteger &lhs, const BigInteger &rhs);

    double GetSignificand() const;
    int GetExponent() const;
    double Log2() const;
private:
    std::vector<byte> m_Data;
    int m_Bits;

    void UpdateBits();
};

BigInteger operator+(const BigInteger &one, const BigInteger &another);
BigInteger operator*(const BigInteger &one, const BigInteger &another);
double operator/(const BigInteger &one, const BigInteger &another);
bool operator==(const BigInteger &lhs, const BigInteger &rhs);
bool operator!=(const BigInteger &lhs, const BigInteger &rhs);
