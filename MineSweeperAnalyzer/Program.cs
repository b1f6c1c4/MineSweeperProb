﻿using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Threading.Tasks;
using MineSweeperCalc;

namespace MineSweeperAnalyzer
{
    internal class Program
    {
        private static int N;
        private static readonly BinomialHelper Helper = new BinomialHelper(30 * 16, 99);

        private static void Main(string[] args)
        {
            N = Convert.ToInt32(Console.ReadLine());

            //using (var sw = new StreamWriter(@"output.txt"))
            //    for (var x = 0; x < 15; x++)
            //        for (var y = 0; y < 8; y++)
            //        {
            //            Console.WriteLine("X={0}, Y={1}", x, y);
            //            var dic = Process(x, y);
            //            foreach (var kvp in dic)
            //                sw.WriteLine("{0}\t{1}\t{2:R}\t{3}", x, y, kvp.Key, kvp.Value);
            //        }

            var dic = Process(6, 3);
            foreach (var kvp in dic.OrderBy(kvp => kvp.Key))
                Console.WriteLine("{0:F2}\t{1}", kvp.Key, kvp.Value);
            Console.ReadLine();
            Console.ReadLine();
        }

        private static ConcurrentDictionary<double, int> Process(int x, int y)
        {
            var dic = new ConcurrentDictionary<double, int>();
            Parallel.For(
                         0,
                         N,
                         i =>
                         {
                             var bit =
                                 (new Tester(new BlockMgr(30, 16, 99), Helper, (int)DateTime.Now.Ticks))
                                     .Execute(x, y);

                             dic.AddOrUpdate(bit, 1, (key, val) => val + 1);
                         });
            return dic;
        }
    }

    internal class Tester
    {
        private bool m_Started;
        private readonly BlockMgr m_Mgr;
        private readonly Solver<Block> m_Solver;
        private int m_ToOpen;
        private double m_Bits;
        private readonly Random m_Rnd;

        public Tester(BlockMgr mgr, BinomialHelper helper, int seed)
        {
            m_Rnd = new Random(seed);
            m_Mgr = mgr;

            var blocks = m_Mgr.TotalWidth * m_Mgr.TotalHeight;

            m_ToOpen = blocks - m_Mgr.TotalMines;

            m_Solver = new Solver<Block>(m_Mgr.Blocks, helper);
            m_Solver.AddRestrain(new BlockSet<Block>(m_Mgr.Blocks), m_Mgr.TotalMines);

            m_Started = true;
        }

        public double Execute(int x, int y)
        {
            OpenBlock(x, y);
            while (m_Started)
            {
                Solve();
                var flag = true;
                for (var i = 0; i < m_Mgr.TotalWidth; i++)
                    for (var j = 0; j < m_Mgr.TotalHeight; j++)
                        if (!m_Mgr[i, j].IsOpen &&
                            m_Solver[m_Mgr[i, j]] == BlockStatus.Blank)
                        {
                            OpenBlock(i, j);
                            if (!m_Started &&
                                m_ToOpen > 0)
                                throw new ApplicationException("判断错误");
                            flag = false;
                        }
                if (!flag)
                    continue;

                var prob = 1D;
                for (var i = 0; i < m_Mgr.TotalWidth; i++)
                    for (var j = 0; j < m_Mgr.TotalHeight; j++)
                        if (!m_Mgr[i, j].IsOpen &&
                            m_Solver.Probability[m_Mgr[i, j]] < prob &&
                            m_Solver[m_Mgr[i, j]] == BlockStatus.Unknown)
                            prob = m_Solver.Probability[m_Mgr[i, j]];

                var quantities = new Dictionary<Block, double>();
                for (var i = 0; i < m_Mgr.TotalWidth; i++)
                    for (var j = 0; j < m_Mgr.TotalHeight; j++)
                        if (!m_Mgr[i, j].IsOpen &&
                            m_Solver.Probability[m_Mgr[i, j]] <= prob)
                        {
                            var block = m_Mgr[i, j];
                            var theBlock = new BlockSet<Block>(new[] { block });
                            var dic = m_Solver.DistributionConditioned(block.Surrounding, theBlock, 0);
                            var total = dic.Aggregate(BigInteger.Zero, (cur, kvp) => cur + kvp.Value);
                            if (total.IsZero)
                                throw new Exception();
                            var q = dic.Sum(
                                            kvp =>
                                            -BigIntegerHelper.Ratio(kvp.Value, total)
                                            * Math.Log(BigIntegerHelper.Ratio(kvp.Value, total), 2));
                            quantities[block] = q;
                        }

                var maxQ = quantities.Values.Max();
                var lst = quantities.Where(kvp => kvp.Value >= maxQ).Select(kvp => kvp.Key).ToList();

                var blk = lst[m_Rnd.Next(lst.Count)];
                OpenBlock(blk.X, blk.Y);
            }
            return m_Bits;
        }

        private void OpenBlock(int x, int y)
        {
            var block = m_Mgr.OpenBlock(x, y);
            if (block.IsMine)
            {
                m_Started = false;
                return;
            }

            if (--m_ToOpen == 0)
                m_Started = false;

            m_Solver.AddRestrain(new BlockSet<Block>(new[] { block }), 0);

            var deg = block.Degree;
            if (deg == 0)
                foreach (var b in block.Surrounding.Cast<Block>().Where(b => !b.IsOpen))
                    OpenBlock(b.X, b.Y);
            else
                m_Solver.AddRestrain(block.Surrounding, deg);
        }

        private void Solve()
        {
            var total = m_Solver.Solve();

            double sig;
            int exp;
            BigIntegerHelper.Part(total, out sig, out exp);
            m_Bits = Math.Log(sig, 2) + exp;
        }
    }
}