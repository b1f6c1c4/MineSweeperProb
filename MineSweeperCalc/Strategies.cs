using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Threading.Tasks;
using MineSweeperCalc.Solver;

// ReSharper disable UnusedMember.Global

namespace MineSweeperCalc
{
    /// <summary>
    ///     策略
    /// </summary>
    public static class Strategies
    {
        #region Auxiliary

        private static BigInteger Sum(this IEnumerable<BigInteger> lst)
            => lst.Aggregate(BigInteger.Zero, (c, v) => c + v);

        private static IDictionary<Block, IDictionary<int, BigInteger>> ToDist(this List<Block> blocks,
                                                                               GameMgr mgr,
                                                                               bool multiThread)
        {
            if (!multiThread)
                return blocks.ToDictionary(
                                           b => b,
                                           b => mgr.Solver.DistributionCond(b.Surrounding, new BlockSet<Block>(b), 0));

            var dic = new ConcurrentDictionary<Block, IDictionary<int, BigInteger>>();
            Action<Block> proc = b => dic[b] = mgr.Solver.DistributionCond(b.Surrounding, new BlockSet<Block>(b), 0);
            Parallel.ForEach(blocks, proc);
            return dic;
        }

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

        private static double Quantity(Block b, IDictionary<int, BigInteger> dist, GameMgr mgr, double coeff)
        {
            var total = dist.Values.Sum();
            if (total != mgr.Solver.Distribution(new BlockSet<Block>(b))[0])
                throw new Exception("F");

            var p0 = (1 - mgr.Solver.Probability[b]);
            var q0 = mgr.Solver.TotalStates.Log2();
            var sum = -coeff * (1 - p0);
            foreach (var kvp in dist)
            {
                if (kvp.Value.IsZero)
                    continue;
                var p = p0 * kvp.Value.Over(total);
                var q = q0 - kvp.Value.Log2();
                sum += p * q;
            }
            return sum;
        }

        private static IEnumerable<Block> Best(IReadOnlyCollection<Block> lst, Func<Block, double> f, bool dir = false)
        {
            var vals = lst.Select(f).ToList();
            var m = dir ? vals.Min() : vals.Max();
            return lst.Where((t, i) => !dir && vals[i] >= m || dir && vals[i] <= m);
        }

        private static IEnumerable<Block> ZeroProb(IReadOnlyCollection<Block> lst,
                                                   IDictionary<Block, IDictionary<int, BigInteger>> degreeDist,
                                                   bool dir = false) => Best(lst, b => ZeroProb(degreeDist[b]), dir);

        private static IEnumerable<Block> ZeroCount(IReadOnlyCollection<Block> lst, GameMgr mgr,
                                                    IDictionary<Block, IDictionary<int, BigInteger>> degreeDist,
                                                    bool dir = false)
            => Best(lst, b => b.ZeroCount(degreeDist[b], mgr), dir);

        private static IEnumerable<Block> Quantity(IReadOnlyCollection<Block> lst,
                                                   IDictionary<Block, IDictionary<int, BigInteger>> degreeDist,
                                                   bool dir = false)
            => Best(lst, b => Quantity(degreeDist[b]), dir);

        #endregion

        public static IEnumerable<Block> MinProb(List<Block> blocks, GameMgr mgr, bool multiThread)
            => Best(blocks, b => mgr.Solver.Probability[b], true);

        public static IEnumerable<Block> MinProbMaxZeroProb(List<Block> blocks, GameMgr mgr, bool multiThread)
        {
            var lst = Best(blocks, b => mgr.Solver.Probability[b], true).ToList();
            var degreeDist = lst.ToDist(mgr, multiThread);
            return ZeroProb(lst, degreeDist);
        }

        public static IEnumerable<Block> MinProbMaxZeroCount(List<Block> blocks, GameMgr mgr, bool multiThread)
        {
            var lst = Best(blocks, b => mgr.Solver.Probability[b], true).ToList();
            var degreeDist = lst.ToDist(mgr, multiThread);
            return ZeroCount(lst, mgr, degreeDist);
        }

        public static IEnumerable<Block> MinProbMaxQuantity(List<Block> blocks, GameMgr mgr, bool multiThread)
        {
            var lst = Best(blocks, b => mgr.Solver.Probability[b], true).ToList();
            var degreeDist = lst.ToDist(mgr, multiThread);
            return Quantity(lst, degreeDist);
        }

        public static IEnumerable<Block> MinProbMinQuantity(List<Block> blocks, GameMgr mgr, bool multiThread)
        {
            var lst = Best(blocks, b => mgr.Solver.Probability[b], true).ToList();
            var degreeDist = lst.ToDist(mgr, multiThread);
            return Quantity(lst, degreeDist, true);
        }

        public static IEnumerable<Block> MinProbMaxZeroProbMaxQuantity(List<Block> blocks, GameMgr mgr, bool multiThread)
        {
            var lst = Best(blocks, b => mgr.Solver.Probability[b], true).ToList();
            var degreeDist = lst.ToDist(mgr, multiThread);
            lst = ZeroProb(lst, degreeDist).ToList();
            return Quantity(lst, degreeDist);
        }

        public static IEnumerable<Block> MinProbMaxZeroProbMinQuantity(List<Block> blocks, GameMgr mgr, bool multiThread)
        {
            var lst = Best(blocks, b => mgr.Solver.Probability[b], true).ToList();
            var degreeDist = lst.ToDist(mgr, multiThread);
            lst = ZeroProb(lst, degreeDist).ToList();
            return Quantity(lst, degreeDist, true);
        }

        public static IEnumerable<Block> MinProbMaxZeroCountMaxQuantity(List<Block> blocks, GameMgr mgr,
                                                                        bool multiThread)
        {
            var lst = Best(blocks, b => mgr.Solver.Probability[b], true).ToList();
            var degreeDist = lst.ToDist(mgr, multiThread);
            lst = ZeroCount(lst, mgr, degreeDist).ToList();
            return Quantity(lst, degreeDist);
        }

        public static IEnumerable<Block> MinProbMaxZeroCountMinQuantity(List<Block> blocks, GameMgr mgr,
                                                                        bool multiThread)
        {
            var lst = Best(blocks, b => mgr.Solver.Probability[b], true).ToList();
            var degreeDist = lst.ToDist(mgr, multiThread);
            lst = ZeroCount(lst, mgr, degreeDist).ToList();
            return Quantity(lst, degreeDist, true);
        }

        public static IEnumerable<Block> MinProbMaxQuantityMaxZeroProb(List<Block> blocks, GameMgr mgr, bool multiThread)
        {
            var lst = Best(blocks, b => mgr.Solver.Probability[b], true).ToList();
            var degreeDist = lst.ToDist(mgr, multiThread);
            lst = Quantity(lst, degreeDist).ToList();
            return ZeroProb(lst, degreeDist);
        }

        public static IEnumerable<Block> MinProbMaxQuantityMaxZeroCount(List<Block> blocks, GameMgr mgr,
                                                                        bool multiThread)
        {
            var lst = Best(blocks, b => mgr.Solver.Probability[b], true).ToList();
            var degreeDist = lst.ToDist(mgr, multiThread);
            lst = Quantity(lst, degreeDist).ToList();
            return ZeroCount(lst, mgr, degreeDist);
        }

        public static IEnumerable<Block> MinProbMinQuantityMaxZeroProb(List<Block> blocks, GameMgr mgr, bool multiThread)
        {
            var lst = Best(blocks, b => mgr.Solver.Probability[b], true).ToList();
            var degreeDist = lst.ToDist(mgr, multiThread);
            lst = Quantity(lst, degreeDist, true).ToList();
            return ZeroProb(lst, degreeDist);
        }

        public static IEnumerable<Block> MinProbMinQuantityMaxZeroCount(List<Block> blocks, GameMgr mgr,
                                                                        bool multiThread)
        {
            var lst = Best(blocks, b => mgr.Solver.Probability[b], true).ToList();
            var degreeDist = lst.ToDist(mgr, multiThread);
            lst = Quantity(lst, degreeDist, true).ToList();
            return ZeroCount(lst, mgr, degreeDist);
        }

        public static IEnumerable<Block> MixProbZeroProb(List<Block> blocks, GameMgr mgr, bool multiThread)
        {
            var degreeDist = blocks.ToDist(mgr, multiThread);
            return Best(blocks, b => (1 - mgr.Solver.Probability[b]) * ZeroProb(degreeDist[b]));
        }

        public static IEnumerable<Block> MixProbZeroCount(List<Block> blocks, GameMgr mgr, bool multiThread)
        {
            var degreeDist = blocks.ToDist(mgr, multiThread);
            return Best(blocks, b => (1 - mgr.Solver.Probability[b]) * b.ZeroCount(degreeDist[b], mgr));
        }

        public static IEnumerable<Block> MixProbQuantity(List<Block> blocks, GameMgr mgr, bool multiThread)
        {
            var degreeDist = blocks.ToDist(mgr, multiThread);
            return Best(blocks, b => (1 - mgr.Solver.Probability[b]) * Quantity(degreeDist[b]));
        }

        public static IEnumerable<Block> HalfQuantity(List<Block> blocks, GameMgr mgr, bool multiThread)
        {
            var degreeDist = blocks.ToDist(mgr, multiThread);
            return Best(blocks, b => Quantity(b, degreeDist[b], mgr, 0D));
        }

        public static IEnumerable<Block> HalfQuantity2(List<Block> blocks, GameMgr mgr, bool multiThread)
        {
            var degreeDist = blocks.ToDist(mgr, multiThread);
            return Best(blocks, b => Quantity(b, degreeDist[b], mgr, 1D));
        }

        public static IEnumerable<Block> HalfQuantity3(List<Block> blocks, GameMgr mgr, bool multiThread)
        {
            var degreeDist = blocks.ToDist(mgr, multiThread);
            return Best(blocks, b => Quantity(b, degreeDist[b], mgr, 10D));
        }

        public static IEnumerable<Block> HalfQuantity4(List<Block> blocks, GameMgr mgr, bool multiThread)
        {
            var degreeDist = blocks.ToDist(mgr, multiThread);
            return Best(blocks, b => Quantity(b, degreeDist[b], mgr, 100D));
        }
    }
}
