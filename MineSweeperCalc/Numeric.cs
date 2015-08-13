using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;

namespace MineSweeperCalc
{
    public static class BinomialHelper
    {
        private static bool m_Editing;
        private static readonly object m_LockObj;
        private static readonly List<List<BigInteger>> m_Arr;

        static BinomialHelper()
        {
            m_LockObj = new object();
            m_Arr = new List<List<BigInteger>> { new List<BigInteger> { 1 } };
        }

        public static void UpdateTo(int n, int m)
        {
            if (n < 0)
                throw new ArgumentException("必须为非负整数", nameof(n));
            if (m > n ||
                m < 0)
                return;
            if (m == 0 ||
                m == n)
                return;
            if (m > n / 2)
                m = n - m;
            lock (m_LockObj)
            {
                m_Editing = true;
                if (m_Arr[m_Arr.Count - 1].Count * 2 < m)
                {
                    for (var i = 0; i < m_Arr.Count; i++)
                        for (var j = m_Arr[i].Count; j <= (i - 1) / 2 && j < m; j++)
                            m_Arr[i].Add(
                                         m_Arr[i - 1][j - 1] +
                                         (j == (i - 1) / 2 && i % 2 == 1 ? m_Arr[i - 1][j - 1] : m_Arr[i - 1][j]));
                }
                for (var i = m_Arr.Count; i < n; i++)
                {
                    var lst = new List<BigInteger>(Math.Min((i - 1) / 2 + 1, m)) { BigInteger.One + m_Arr[i - 1][0] };
                    for (var j = 1; j <= (i - 1) / 2 && j <m; j++)
                        lst.Add(
                                m_Arr[i - 1][j - 1] +
                                (j == (i - 1) / 2 && i % 2 == 1 ? m_Arr[i - 1][j - 1] : m_Arr[i - 1][j]));
                    m_Arr.Add(lst);
                }
                m_Editing = false;
            }
        }

        public static BigInteger Binomial(int n, int m)
        {
            if (n < 0)
                throw new ArgumentException("必须为非负整数", nameof(n));
            if (m > n || m < 0)
                return BigInteger.Zero;
            if (m == 0 ||
                m == n)
                return BigInteger.One;
            return m_Arr[n - 1][m <= n / 2 ? m - 1 : n - m - 1];
        }
    }

    public static class BigIntegerHelper
    {
        public static double Ratio(BigInteger numerator, BigInteger denominator)
        {
            double sNum, sDen;
            int eNum, eDen;

            Part(numerator, out sNum, out eNum);
            Part(denominator, out sDen, out eDen);

            return sNum / sDen * Math.Pow(2D, eNum - eDen);
        }

        public static void Part(BigInteger val, out double significand, out int exponent)
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
    }
}
