using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;

namespace MineSweeperCalc
{
    public class BinomialHelper
    {
        private readonly BigInteger[,] m_Arr;

        public BinomialHelper(int n, int r)
        {
            var arr = new BigInteger[n, r];
            arr[0, 0] = BigInteger.One;
            for (var i = 1; i < n; i++)
                for (var j = 0; j < r && j <= i; j++)
                    arr[i, j] = (j == 0 ? BigInteger.One : arr[i - 1, j - 1]) + arr[i - 1, j];
            m_Arr = arr;
        }

        public BigInteger Binomial(int n, int m) => (n >= 1 && m >= 1) ? m_Arr[n - 1, m - 1] : BigInteger.One;
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
