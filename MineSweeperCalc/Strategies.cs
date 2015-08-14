using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using MineSweeperCalc.Solver;

namespace MineSweeperCalc
{
    public static class Strategies
    {
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
            Func<double, double> f = p => p * Math.Log(p, 2);
            return dist.Sum(kvp => kvp.Value.IsZero ? 0D : f(kvp.Value.Over(total)));
        }

        public static IEnumerable<Block> MinProb(IEnumerable<Block> blocks, GameMgr mgr)
        {
            var lst = blocks.ToList();
            var prob = lst.Min(b => mgr.Solver.Probability[b]);
            return lst.Where(b => mgr.Solver.Probability[b] <= prob);
        }

        public static IEnumerable<Block> MinProbMaxZeroProb(IEnumerable<Block> blocks, GameMgr mgr)
        {
            var lst = blocks.ToList();
            var prob = lst.Min(b => mgr.Solver.Probability[b]);
            lst = lst.Where(b => mgr.Solver.Probability[b] <= prob).ToList();

            var degreeDist = lst.ToDist(mgr);

            var zeroProb = lst.ToDictionary(b => b, b => ZeroProb(degreeDist[b]));
            var zero = zeroProb.Values.Max();
            return lst.Where(b => zeroProb[b] >= zero);
        }

        public static IEnumerable<Block> MinProbMaxZeroCount(IEnumerable<Block> blocks, GameMgr mgr)
        {
            var lst = blocks.ToList();
            var prob = lst.Min(b => mgr.Solver.Probability[b]);
            lst = lst.Where(b => mgr.Solver.Probability[b] <= prob).ToList();

            var degreeDist = lst.ToDist(mgr);

            var zeroCount = lst.ToDictionary(b => b, b => b.ZeroCount(degreeDist[b], mgr));
            var zero = zeroCount.Values.Max();
            return lst.Where(b => zeroCount[b] >= zero);
        }

        public static IEnumerable<Block> MinProbMaxQuantity(IEnumerable<Block> blocks, GameMgr mgr)
        {
            var lst = blocks.ToList();
            var prob = lst.Min(b => mgr.Solver.Probability[b]);
            lst = lst.Where(b => mgr.Solver.Probability[b] <= prob).ToList();

            var degreeDist = lst.ToDist(mgr);

            var quantity = lst.ToDictionary(b => b, b => Quantity(degreeDist[b]));
            var quant = quantity.Values.Max();
            return lst.Where(b => quantity[b] >= quant);
        }
    }
}
