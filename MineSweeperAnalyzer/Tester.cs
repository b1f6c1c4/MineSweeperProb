using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using MineSweeperCalc;
using MineSweeperCalc.Solver;

namespace MineSweeperAnalyzer
{
    /// <summary>
    ///     ²ßÂÔ²âÊÔÆ÷
    /// </summary>
    internal class Tester
    {
        private readonly GameMgr m_Mgr;
        private double m_Bits;
        private readonly Random m_Rnd;

        public Tester(GameMgr mgr, int seed)
        {
            m_Rnd = new Random(seed);
            m_Mgr = mgr;
        }

        public double Execute(int x, int y)
        {
            m_Mgr.OpenBlock(x, y);
            while (m_Mgr.Started)
            {
                m_Bits = m_Mgr.Solver.Solve().Log2();
                var flag = false;
                for (var i = 0; i < m_Mgr.TotalWidth; i++)
                {
                    for (var j = 0; j < m_Mgr.TotalHeight; j++)
                        if (!m_Mgr[i, j].IsOpen &&
                            m_Mgr.Solver[m_Mgr[i, j]] == BlockStatus.Blank)
                        {
                            m_Mgr.OpenBlock(i, j);
                            flag = true;
                            if (m_Mgr.Started)
                                continue;

                            if (!m_Mgr.Suceed)
                                throw new ApplicationException("ÅÐ¶Ï´íÎó");
                            break;
                        }
                    if (!m_Mgr.Started)
                        break;
                }
                if (flag)
                    continue;

                var prob = 1D;
                for (var i = 0; i < m_Mgr.TotalWidth; i++)
                    for (var j = 0; j < m_Mgr.TotalHeight; j++)
                        if (!m_Mgr[i, j].IsOpen &&
                            m_Mgr.Solver.Probability[m_Mgr[i, j]] < prob &&
                            m_Mgr.Solver[m_Mgr[i, j]] == BlockStatus.Unknown)
                            prob = m_Mgr.Solver.Probability[m_Mgr[i, j]];

                var pOfZero = new Dictionary<Block, double>();
                var quantities = new Dictionary<Block, double>();
                for (var i = 0; i < m_Mgr.TotalWidth; i++)
                    for (var j = 0; j < m_Mgr.TotalHeight; j++)
                        if (!m_Mgr[i, j].IsOpen &&
                            m_Mgr.Solver.Probability[m_Mgr[i, j]] <= prob)
                        {
                            var block = m_Mgr[i, j];
                            var theBlock = new BlockSet<Block>(new[] { block });
                            var dic = m_Mgr.Solver.DistributionConditioned(block.Surrounding, theBlock, 0);
                            var total = dic.Aggregate(BigInteger.Zero, (cur, kvp) => cur + kvp.Value);
                            if (total.IsZero)
                                throw new Exception();
                            pOfZero[block] = dic[dic.Keys.Min()].Over(total);
                            var q =
                                dic.Sum(
                                        kvp =>
                                        {
                                            if (kvp.Value == 0)
                                                return 0D;
                                            var p = kvp.Value.Over(total);
                                            return -p * Math.Log(p, 2);
                                        });
                            quantities[block] = q;
                        }

                var maxP = pOfZero.Values.Max();
                var lst = pOfZero.Where(kvp => kvp.Value >= maxP).Select(kvp => kvp.Key).ToList();
                var maxQ = lst.Select(b => quantities[b]).Max();
                lst =
                    quantities.Where(kvp => lst.Contains(kvp.Key) && kvp.Value >= maxQ).Select(kvp => kvp.Key).ToList();

                var blk = lst[m_Rnd.Next(lst.Count)];
                m_Mgr.OpenBlock(blk.X, blk.Y);
            }
            return m_Bits;
        }
    }
}
