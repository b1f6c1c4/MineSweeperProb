﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Threading;

namespace MineSweeperCalc.Solver
{
    /// <summary>
    ///     二项式系数计算器
    /// </summary>
    public static class BinomialHelper
    {
        /// <summary>
        ///     读写锁
        /// </summary>
        private static readonly ReaderWriterLockSlim TheLock;

        /// <summary>
        ///     二项式系数
        /// </summary>
        private static readonly List<List<BigInteger>> Coeff;

        static BinomialHelper()
        {
            TheLock = new ReaderWriterLockSlim();
            Coeff = new List<List<BigInteger>> { new List<BigInteger> { 1 } };
        }

        /// <summary>
        ///     扩充缓存的二项式系数表
        /// </summary>
        /// <param name="n">二项式系数的第一个参数</param>
        /// <param name="m">二项式系数的第二个参数</param>
        public static void UpdateTo(int n, int m)
        {
            if (n < 0)
                throw new ArgumentException("必须为非负整数", nameof(n));
            if (m > n ||
                m < 0)
                return;
            if (m == 0)
                return;
            if (m > n / 2)
                m = n / 2;

            TheLock.EnterWriteLock();
            try
            {
                if (Coeff[Coeff.Count - 1].Count * 2 < m)
                    for (var i = 0; i < Coeff.Count; i++)
                        for (var j = Coeff[i].Count; j <= (i - 1) / 2 && j < m; j++)
                            Coeff[i].Add(
                                         Coeff[i - 1][j - 1] +
                                         (j == (i - 1) / 2 && i % 2 == 1 ? Coeff[i - 1][j - 1] : Coeff[i - 1][j]));
                for (var i = Coeff.Count; i < n; i++)
                {
                    var lst = new List<BigInteger>(Math.Min((i - 1) / 2 + 1, m)) { BigInteger.One + Coeff[i - 1][0] };
                    for (var j = 1; j <= (i - 1) / 2 && j < m; j++)
                        lst.Add(
                                Coeff[i - 1][j - 1] +
                                (j == (i - 1) / 2 && i % 2 == 1 ? Coeff[i - 1][j - 1] : Coeff[i - 1][j]));
                    Coeff.Add(lst);
                }
            }
            finally
            {
                TheLock.ExitWriteLock();
            }
        }

        /// <summary>
        ///     计算二项式系数
        /// </summary>
        /// <param name="n">二项式系数的第一个参数</param>
        /// <param name="m">二项式系数的第二个参数</param>
        /// <returns>二项式系数</returns>
        public static BigInteger Binomial(int n, int m)
        {
            if (n < 0)
                throw new ArgumentException("必须为非负整数", nameof(n));
            if (m > n ||
                m < 0)
                return BigInteger.Zero;
            if (m == 0 ||
                m == n)
                return BigInteger.One;

            var mm = m <= n / 2 ? m : n - m;

            TheLock.EnterReadLock();
            try
            {
                if (Coeff.Count < n)
                    throw new ArgumentOutOfRangeException(nameof(n), "超出范围");
                if (Coeff[Coeff.Count - 1].Count < mm)
                    throw new ArgumentOutOfRangeException(nameof(m), "超出范围");
                return Coeff[n - 1][mm - 1];
            }
            finally
            {
                TheLock.ExitReadLock();
            }
        }
    }

    /// <summary>
    ///     <c>BigInteger</c>处理
    /// </summary>
    public static class BigIntegerHelper
    {
        /// <summary>
        ///     计算两<c>BigInteger</c>的比值
        /// </summary>
        /// <param name="numerator">分子</param>
        /// <param name="denominator">分母</param>
        /// <returns>比值</returns>
        public static double Over(this BigInteger numerator, BigInteger denominator)
        {
            double sNum, sDen;
            int eNum, eDen;

            numerator.Part(out sNum, out eNum);
            denominator.Part(out sDen, out eDen);

            return sNum / sDen * Math.Pow(2D, eNum - eDen);
        }

        /// <summary>
        ///     将<c>BigInteger</c>拆分成有效数字和指数
        ///     若<paramref name="val" />非0则<paramref name="significand" />在1（含）到2（不含）之间
        ///     若<paramref name="val" />为0则<paramref name="significand" />和<paramref name="exponent" />均为0
        /// </summary>
        /// <param name="val">待拆分的数</param>
        /// <param name="significand">有效数字</param>
        /// <param name="exponent">指数</param>
        public static void Part(this BigInteger val, out double significand, out int exponent)
        {
            if (val == 0)
            {
                significand = 0D;
                exponent = 0;
                return;
            }

            var bytes = val.ToByteArray();
            var lastID = bytes[bytes.Length - 1] == 0 ? bytes.Length - 2 : bytes.Length - 1;

            List<byte> lst;
            if (lastID >= 6)
                lst = bytes.Skip(lastID - 6).Take(7).Reverse().ToList();
            else
            {
                lst = bytes.Take(lastID + 1).Reverse().ToList();
                while (lst.Count < 7)
                    lst.Add(0x00);
            }

            var res = new byte[] { 0x3F, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

            var shift = 0;
            var b = lst[0];
            while (b < 0x80)
            {
                b <<= 1;
                shift++;
            }

            if (shift <= 3)
            {
                res[1] |= (byte)(0x0F & (lst[0] >> (3 - shift)));
                for (var i = 0; i < 6; i++)
                    res[i + 2] = (byte)((lst[i] << (5 + shift)) | (lst[i + 1] >> (3 - shift)));
            }
            else
            {
                res[1] |= (byte)(0x0F & (lst[0] << (shift - 3)) | lst[1] >> (11 - shift));
                for (var i = 1; i < 6; i++)
                    res[i + 1] = (byte)((lst[i] << (shift - 3)) | (lst[i + 1] >> (11 - shift)));
                res[7] = (byte)((lst[6] << (shift - 3)));
            }

            significand = BitConverter.ToDouble(res.Reverse().ToArray(), 0);
            exponent = lastID * 8 + (7 - shift);
        }

        /// <summary>
        ///     获取其以2为底的对数
        /// </summary>
        /// <param name="val">值</param>
        /// <returns>对数</returns>
        public static double Log2(this BigInteger val)
        {
            double sig;
            int exp;
            val.Part(out sig, out exp);
            return Math.Log(sig, 2) + exp;
        }
    }

    /// <summary>
    ///     有理数
    /// </summary>
    public struct Rational : IComparable<Rational>, IEquatable<Rational>
    {
        public Rational(BigInteger numerator):this(numerator, BigInteger.One) { }

        public Rational(BigInteger numerator, BigInteger denominator)
        {
            var b = BigInteger.GreatestCommonDivisor(numerator, denominator);
            if (denominator.Sign == 0)
                throw new DivideByZeroException();
            if (denominator.Sign < 0)
            {
                Numerator = -numerator / b;
                Denominator = -denominator / b;
            }
            else
            {
                Numerator = numerator / b;
                Denominator = denominator / b;
            }
        }

        /// <summary>
        ///     分子
        /// </summary>
        public BigInteger Numerator { get; }

        /// <summary>
        ///     分母
        /// </summary>
        public BigInteger Denominator { get; }

        public bool IsZero => Numerator.IsZero;

        public override string ToString() => $"{(double)this:E2}={Numerator}/{Denominator}";

        public int CompareTo(Rational other)
            => (Numerator * other.Denominator).CompareTo(Denominator * other.Numerator);

        public static implicit operator Rational(int v) => new Rational(v);

        public static implicit operator Rational(BigInteger r) => new Rational(r);

        public static implicit operator double(Rational r) => r.Numerator.Over(r.Denominator);

        public static Rational operator +(Rational r1, Rational r2)
            =>
                new Rational(
                    r1.Numerator * r2.Denominator + r2.Numerator * r1.Denominator,
                    r1.Denominator * r2.Denominator);

        public static Rational operator -(Rational r1, Rational r2)
            =>
                new Rational(
                    r1.Numerator * r2.Denominator - r2.Numerator * r1.Denominator,
                    r1.Denominator * r2.Denominator);

        public static Rational operator *(Rational r1, Rational r2)
            => new Rational(r1.Numerator * r2.Numerator, r1.Denominator * r2.Denominator);

        public static Rational operator /(Rational r1, Rational r2)
            => new Rational(r1.Numerator * r2.Denominator, r1.Denominator * r2.Numerator);

        public static bool operator <(Rational r1, Rational r2)
            => r1.CompareTo(r2) < 0;

        public static bool operator >(Rational r1, Rational r2)
            => r1.CompareTo(r2) > 0;

        public static bool operator <=(Rational r1, Rational r2)
            => r1.CompareTo(r2) <= 0;

        public static bool operator >=(Rational r1, Rational r2)
            => r1.CompareTo(r2) >= 0;

        public static bool operator ==(Rational r1, Rational r2)
            => r1.Equals(r2);

        public static bool operator !=(Rational r1, Rational r2)
            => !r1.Equals(r2);

        public bool Equals(Rational other) => Numerator.Equals(other.Numerator) && Denominator.Equals(other.Denominator);

        public override bool Equals(object obj)
        {
            if (ReferenceEquals(null, obj))
                return false;
            return obj is Rational && Equals((Rational)obj);
        }

        public override int GetHashCode()
        {
            unchecked
            {
                return (Numerator.GetHashCode() * 397) ^ Denominator.GetHashCode();
            }
        }
    }
}
