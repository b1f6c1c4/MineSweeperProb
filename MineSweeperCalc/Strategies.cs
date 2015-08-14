using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using MineSweeperCalc.Solver;

namespace MineSweeperCalc
{
    /// <summary>
    ///     策略
    /// </summary>
    public static class Strategies
    {
        #region Auxiliary

        private static BigInteger Sum(this IEnumerable<BigInteger> lst)
            => lst.Aggregate(BigInteger.One, (c, v) => c + v);

        private static Dictionary<Block, IDictionary<int, BigInteger>> ToDist(this IEnumerable<Block> blocks,
                                                                              GameMgr mgr)
            => blocks.ToDictionary(b => b, b => mgr.Solver.DistributionCond(b.Surrounding, new BlockSet<Block>(b), 0));

        private static double ZeroProb(IDictionary<int, BigInteger> dist)
            => dist[dist.Keys.Min()].Over(dist.Values.Sum());

        private static double ZeroCount(this Block block, IDictionary<int, BigInteger> dist, GameMgr mgr)
            =>
                dist[dist.Keys.Min()].Over(dist.Values.Sum()) *
                block.Surrounding.Blocks.Count(b => !b.IsOpen && mgr.Solver[b] == BlockStatus.Unknown);

        private static double Quantity(IDictionary<int, BigInteger> dist)
        {
            var total = dist.Values.Sum();
            Func<double, double> f = p => -p * Math.Log(p, 2);
            return dist.Sum(kvp => kvp.Value.IsZero ? 0D : f(kvp.Value.Over(total)));
        }

        private static IEnumerable<Block> Best(IReadOnlyCollection<Block> lst, Func<Block, double> f, bool dir = false)
        {
            var vals = lst.Select(f).ToList();
            var m = dir ? vals.Min() : vals.Max();
            return lst.Where((t, i) => dir && vals[i] >= m || !dir && vals[i] <= m);
        }

        private static IEnumerable<Block> ZeroProb(IReadOnlyCollection<Block> lst, IReadOnlyDictionary<Block, IDictionary<int, BigInteger>> degreeDist, bool dir = false) => Best(lst, b => ZeroProb(degreeDist[b]), dir);

        private static IEnumerable<Block> ZeroCount(IReadOnlyCollection<Block> lst, GameMgr mgr, IReadOnlyDictionary<Block, IDictionary<int, BigInteger>> degreeDist, bool dir = false)
            => Best(lst, b => b.ZeroCount(degreeDist[b], mgr), dir);

        private static IEnumerable<Block> Quantity(IReadOnlyCollection<Block> lst, IReadOnlyDictionary<Block, IDictionary<int, BigInteger>> degreeDist, bool dir = false)
            => Best(lst, b => Quantity(degreeDist[b]), dir);

        #endregion

        public static IEnumerable<Block> MinProb(List<Block> blocks, GameMgr mgr)
            => Best(blocks, b => mgr.Solver.Probability[b], true);

        public static IEnumerable<Block> MinProbMaxZeroProb(List<Block> blocks, GameMgr mgr)
        {
            var lst = Best(blocks, b => mgr.Solver.Probability[b], true).ToList();
            var degreeDist = lst.ToDist(mgr);
            return ZeroProb(lst, degreeDist);
        }

        public static IEnumerable<Block> MinProbMaxZeroCount(List<Block> blocks, GameMgr mgr)
        {
            var lst = Best(blocks, b => mgr.Solver.Probability[b], true).ToList();
            var degreeDist = lst.ToDist(mgr);
            return ZeroCount(lst, mgr, degreeDist);
        }

        public static IEnumerable<Block> MinProbMaxQuantity(List<Block> blocks, GameMgr mgr)
        {
            var lst = Best(blocks, b => mgr.Solver.Probability[b], true).ToList();
            var degreeDist = lst.ToDist(mgr);
            return Quantity(lst, degreeDist);
        }

        public static IEnumerable<Block> MinProbMinQuantity(List<Block> blocks, GameMgr mgr)
        {
            var lst = Best(blocks, b => mgr.Solver.Probability[b], true).ToList();
            var degreeDist = lst.ToDist(mgr);
            return Quantity(lst, degreeDist, true);
        }

        public static IEnumerable<Block> MinProbMaxZeroProbMaxQuantity(List<Block> blocks, GameMgr mgr)
        {
            var lst = Best(blocks, b => mgr.Solver.Probability[b], true).ToList();
            var degreeDist = lst.ToDist(mgr);
            lst = ZeroProb(lst, degreeDist).ToList();
            return Quantity(lst, degreeDist);
        }

        public static IEnumerable<Block> MinProbMaxZeroProbMinQuantity(List<Block> blocks, GameMgr mgr)
        {
            var lst = Best(blocks, b => mgr.Solver.Probability[b], true).ToList();
            var degreeDist = lst.ToDist(mgr);
            lst = ZeroProb(lst, degreeDist).ToList();
            return Quantity(lst, degreeDist, true);
        }

        public static IEnumerable<Block> MinProbMaxZeroCountMaxQuantity(List<Block> blocks, GameMgr mgr)
        {
            var lst = Best(blocks, b => mgr.Solver.Probability[b], true).ToList();
            var degreeDist = lst.ToDist(mgr);
            lst = ZeroCount(lst, mgr, degreeDist).ToList();
            return Quantity(lst, degreeDist);
        }

        public static IEnumerable<Block> MinProbMaxZeroCountMinQuantity(List<Block> blocks, GameMgr mgr)
        {
            var lst = Best(blocks, b => mgr.Solver.Probability[b], true).ToList();
            var degreeDist = lst.ToDist(mgr);
            lst = ZeroCount(lst, mgr, degreeDist).ToList();
            return Quantity(lst, degreeDist, true);
        }

        public static IEnumerable<Block> MinProbMaxQuantityMaxZeroProb(List<Block> blocks, GameMgr mgr)
        {
            var lst = Best(blocks, b => mgr.Solver.Probability[b], true).ToList();
            var degreeDist = lst.ToDist(mgr);
            lst = Quantity(lst, degreeDist).ToList();
            return ZeroProb(lst, degreeDist);
        }

        public static IEnumerable<Block> MinProbMaxQuantityMaxZeroCount(List<Block> blocks, GameMgr mgr)
        {
            var lst = Best(blocks, b => mgr.Solver.Probability[b], true).ToList();
            var degreeDist = lst.ToDist(mgr);
            lst = Quantity(lst, degreeDist).ToList();
            return ZeroCount(lst, mgr, degreeDist);
        }

        public static IEnumerable<Block> MinProbMinQuantityMaxZeroProb(List<Block> blocks, GameMgr mgr)
        {
            var lst = Best(blocks, b => mgr.Solver.Probability[b], true).ToList();
            var degreeDist = lst.ToDist(mgr);
            lst = Quantity(lst, degreeDist, true).ToList();
            return ZeroProb(lst, degreeDist);
        }

        public static IEnumerable<Block> MinProbMinQuantityMaxZeroCount(List<Block> blocks, GameMgr mgr)
        {
            var lst = Best(blocks, b => mgr.Solver.Probability[b], true).ToList();
            var degreeDist = lst.ToDist(mgr);
            lst = Quantity(lst, degreeDist, true).ToList();
            return ZeroCount(lst, mgr, degreeDist);
        }

        public static IEnumerable<Block> MixProbZeroProb(List<Block> blocks, GameMgr mgr)
        {
            var degreeDist = blocks.ToDist(mgr);
            return Best(blocks, b => (1 - mgr.Solver.Probability[b]) * ZeroProb(degreeDist[b]));
        }
    }
}
