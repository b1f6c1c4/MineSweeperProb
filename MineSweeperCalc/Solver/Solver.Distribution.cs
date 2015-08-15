using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;

namespace MineSweeperCalc.Solver
{
    public sealed partial class Solver<T>
        where T : IBlock<T>
    {
        /// <summary>
        ///     将一个分布叠加到另一个上
        /// </summary>
        /// <param name="from">一个分布</param>
        /// <param name="to">另一个分布</param>
        private static void Merge(BigInteger[] from, BigInteger[] to)
        {
            for (var i = 0; i < from.Length; i++)
                to[i] += from[i];
        }

        /// <summary>
        ///     计算两个随机变量的和的分布
        /// </summary>
        /// <param name="from">源分布</param>
        /// <param name="cases">新分布</param>
        /// <returns>和的分布</returns>
        private static BigInteger[] Add(BigInteger[] from, BigInteger[] cases)
        {
            var dicN = new BigInteger[from.Length + cases.Length - 1];
            for (var i = 0; i < from.Length; i++)
                for (var j = 0; j < cases.Length; j++)
                    dicN[i + j] += from[i] * cases[j];
            return dicN;
        }

        /// <summary>
        ///     清点格的集合与现有格的集合的交叉数
        /// </summary>
        /// <param name="set">格的集合</param>
        /// <returns>各交叉数</returns>
        private int[] GetIntersectionCounts(BlockSet<T> set)
        {
            var lst = new List<T>(set.Blocks);
            var sets = new int[m_BlockSets.Count];
            for (var i = 0; i < m_BlockSets.Count; i++)
            {
                List<T> exceptA, exceptB, intersection;
                Overlap(m_BlockSets[i].Blocks, lst, out exceptA, out exceptB, out intersection);
                lst = exceptB;
                sets[i] = intersection.Count;
            }
            return sets;
        }

        /// <summary>
        ///     计算Hash
        /// </summary>
        /// <param name="set">数</param>
        /// <returns>Hash</returns>
        private static int Hash(int[] set) => set.Aggregate(5381, (c, v) => (c << 5) + c + v + 30);

        /// <summary>
        ///     条件分布的计算参数
        /// </summary>
        private sealed class DistParameters : IEquatable<DistParameters>
        {
            public DistParameters(int[] sets, int length)
            {
                Sets = sets;
                Length = length;

                m_Hash = Hash(Sets);
            }

            private readonly int m_Hash;

            public override int GetHashCode() => m_Hash;

            public bool Equals(DistParameters other)
            {
                if (m_Hash != other.m_Hash)
                    return false;
                if (Sets.Length != other.Sets.Length)
                    return false;
                if (Sets.Where((t, i) => t != other.Sets[i]).Any())
                    return false;
                return true;
            }

            public int[] Sets { get; }
            public int Length { get; }
        }

        /// <summary>
        ///     分布缓存
        /// </summary>
        private readonly ConcurrentDictionary<DistParameters, BigInteger[]> m_DistCache =
            new ConcurrentDictionary<DistParameters, BigInteger[]>();

        /// <summary>
        ///     分布缓存写入锁
        /// </summary>
        private readonly ConcurrentDictionary<DistParameters, object> m_DistCacheLock =
            new ConcurrentDictionary<DistParameters, object>();

        /// <summary>
        ///     计算指定格的集合的雷数的分布
        /// </summary>
        /// <param name="set">格的集合</param>
        /// <returns>分布</returns>
        // ReSharper disable once MemberCanBePrivate.Global
        public IDictionary<int, BigInteger> Distribution(BlockSet<T> set)
        {
            int dMines, dBlanks;
            var lst = ReduceSet(set, out dMines, out dBlanks);
            var sets = GetIntersectionCounts(new BlockSet<T>(lst));

            var dist = Distribution(new DistParameters(sets, lst.Count));
            var dic = new Dictionary<int, BigInteger>(dist.Length);
            for (var i = 0; i < dist.Length; i++)
                dic.Add(i + dMines, dist[i]);
            return dic;
        }

        /// <summary>
        ///     计算指定格的集合的雷数的分布
        /// </summary>
        /// <param name="par">参数</param>
        /// <returns>分布</returns>
        private BigInteger[] Distribution(DistParameters par)
        {
            BigInteger[] dic;
            if (m_DistCache.TryGetValue(par, out dic))
                return dic;

            object lockObj;
            while (true)
            {
                if (m_DistCacheLock.TryGetValue(par, out lockObj))
                    break;
                lockObj = new object();
                if (m_DistCacheLock.TryAdd(par, lockObj))
                    break;
            }

            lock (lockObj)
            {
                if (m_DistCache.TryGetValue(par, out dic))
                    return dic;

                dic = new BigInteger[par.Length + 1];
                foreach (var solution in Solutions)
                {
                    var dicT = new BigInteger[1];
                    dicT[0] = 1;
                    for (var i = 0; i < m_BlockSets.Count; i++)
                    {
                        var n = m_BlockSets[i].Count;
                        var a = par.Sets[i];
                        var m = solution.Dist[i];

                        var cases = new BigInteger[Math.Min(m, a) + 1];
                        for (var j = 0; j <= m && j <= a; j++)
                            cases[j] = BinomialHelper.Binomial(a, j) * BinomialHelper.Binomial(n - a, m - j);

                        dicT = Add(dicT, cases);
                    }
                    Merge(dicT, dic);
                }

                m_DistCache[par] = dic;
            }
            return dic;
        }

        /// <summary>
        ///     清点两格的集合与现有格的集合的交叉数
        /// </summary>
        /// <param name="set1">第一个格的集合</param>
        /// <param name="set2">第二个格的集合</param>
        /// <param name="sets1">仅第一个格的集合的各交叉数</param>
        /// <param name="sets2">仅第二个格的集合的各交叉数</param>
        /// <param name="sets3">两格的集合共同的各交叉数</param>
        private void GetIntersectionCounts(BlockSet<T> set1, BlockSet<T> set2,
                                           out int[] sets1, out int[] sets2, out int[] sets3)
        {
            var lst1 = new List<T>(set1.Blocks);
            var lst2 = new List<T>(set2.Blocks);
            sets1 = new int[m_BlockSets.Count];
            sets2 = new int[m_BlockSets.Count];
            sets3 = new int[m_BlockSets.Count];
            for (var i = 0; i < m_BlockSets.Count; i++)
            {
                List<T> exceptA, exceptB1T, exceptB2T, exceptB1, exceptB2, exceptC, resume1, resume2;
                Overlap(m_BlockSets[i].Blocks, lst1, out exceptA, out resume1, out exceptB1T);
                Overlap(m_BlockSets[i].Blocks, lst2, out exceptA, out resume2, out exceptB2T);
                Overlap(exceptB1T, exceptB2T, out exceptB1, out exceptB2, out exceptC);
                lst1 = resume1;
                lst2 = resume2;
                sets1[i] = exceptB1.Count;
                sets2[i] = exceptB2.Count;
                sets3[i] = exceptC.Count;
            }
        }

        /// <summary>
        ///     计算在一定条件下指定格的集合的雷数的分布
        /// </summary>
        /// <param name="set">格的集合</param>
        /// <param name="setCond">用作条件的格的集合</param>
        /// <param name="mines">用作条件的格的集合中的雷数</param>
        /// <returns>分布</returns>
        public IDictionary<int, BigInteger> DistributionCond(BlockSet<T> set, BlockSet<T> setCond, int mines)
        {
            int dMines, dBlanks;
            var lst = ReduceSet(set, out dMines, out dBlanks);

            int dMinesCond, dBlanksCond;
            var lstCond = ReduceSet(setCond, out dMinesCond, out dBlanksCond);

            if (lstCond.Count == 0)
            {
                if (dMinesCond == mines)
                    return Distribution(set);
                return new Dictionary<int, BigInteger>();
            }

            int[] sets1, sets2, sets3;
            GetIntersectionCounts(
                                  new BlockSet<T>(lst),
                                  new BlockSet<T>(lstCond),
                                  out sets1,
                                  out sets2,
                                  out sets3);

            var dist = DistCond(new DistCondParameters(sets1, sets2, sets3, mines - dMinesCond, lst.Count));
            var dic = new Dictionary<int, BigInteger>(dist.Length);
            for (var i = 0; i < dist.Length; i++)
                dic.Add(i + dMines, dist[i]);
            return dic;
        }

        /// <summary>
        ///     条件分布的计算参数
        /// </summary>
        private sealed class DistCondParameters : IEquatable<DistCondParameters>
        {
            public DistCondParameters(int[] sets1, int[] sets2, int[] sets3, int minesCond, int length)
            {
                Sets1 = sets1;
                Sets2 = sets2;
                Sets3 = sets3;
                MinesCond = minesCond;
                Length = length;

                m_Hash = (Hash(Sets1) << 14) ^ (Hash(Sets2) << 7) ^ (Hash(Sets3) << 1) ^ MinesCond.GetHashCode();
            }

            private readonly int m_Hash;

            public override int GetHashCode() => m_Hash;

            public bool Equals(DistCondParameters other)
            {
                if (m_Hash != other.m_Hash)
                    return false;
                if (MinesCond != other.MinesCond)
                    return false;
                if (Sets1.Length != other.Sets1.Length)
                    return false;
                if (Sets1.Where((t, i) => t != other.Sets1[i]).Any())
                    return false;
                if (Sets2.Length != other.Sets2.Length)
                    return false;
                if (Sets2.Where((t, i) => t != other.Sets2[i]).Any())
                    return false;
                if (Sets3.Length != other.Sets3.Length)
                    return false;
                if (Sets3.Where((t, i) => t != other.Sets3[i]).Any())
                    return false;
                return true;
            }

            public int[] Sets1 { get; }
            public int[] Sets2 { get; }
            public int[] Sets3 { get; }
            public int MinesCond { get; }
            public int Length { get; }
        }

        /// <summary>
        ///     条件分布缓存
        /// </summary>
        private readonly ConcurrentDictionary<DistCondParameters, BigInteger[]> m_DistCondCache =
            new ConcurrentDictionary<DistCondParameters, BigInteger[]>();

        /// <summary>
        ///     条件分布缓存写入锁
        /// </summary>
        private readonly ConcurrentDictionary<DistCondParameters, object> m_DistCondCacheLock =
            new ConcurrentDictionary<DistCondParameters, object>();

        /// <summary>
        ///     计算在一定条件下指定格的集合的雷数的分布
        /// </summary>
        /// <param name="par">参数</param>
        /// <returns>分布</returns>
        private BigInteger[] DistCond(DistCondParameters par)
        {
            BigInteger[] dic;
            if (m_DistCondCache.TryGetValue(par, out dic))
                return dic;

            object lockObj;
            while (true)
            {
                if (m_DistCondCacheLock.TryGetValue(par, out lockObj))
                    break;
                lockObj = new object();
                if (m_DistCondCacheLock.TryAdd(par, lockObj))
                    break;
            }

            lock (lockObj)
            {
                if (m_DistCondCache.TryGetValue(par, out dic))
                    return dic;

                dic = new BigInteger[par.Length + 1];
                foreach (var solution in Solutions)
                {
                    var max = new int[m_BlockSets.Count];
                    for (var i = 0; i < m_BlockSets.Count; i++)
                        max[i] = Math.Min(solution.Dist[i], par.Sets2[i] + par.Sets3[i]);

                    var stack = new List<int>(m_BlockSets.Count);
                    if (m_BlockSets.Count > 1)
                        stack.Add(0);
                    while (true)
                        if (stack.Count == m_BlockSets.Count - 1)
                        {
                            var mns = par.MinesCond - stack.Sum();
                            if (mns >= 0 &&
                                mns <= max[m_BlockSets.Count - 1])
                            {
                                stack.Add(mns);

                                var dicT = new BigInteger[1];
                                dicT[0] = 1;
                                for (var i = 0; i < m_BlockSets.Count; i++)
                                {
                                    var n = m_BlockSets[i].Count;
                                    var a = par.Sets1[i];
                                    var b = par.Sets2[i];
                                    var c = par.Sets3[i];
                                    var m = solution.Dist[i]; // in a + b + c
                                    var p = stack[i]; // in b + c

                                    {
                                        var cases = new BigInteger[Math.Min(p, c) + 1];
                                        for (var j = 0; j <= p && j <= c; j++)
                                            cases[j] = BinomialHelper.Binomial(c, j) *
                                                       BinomialHelper.Binomial(b, p - j);

                                        dicT = Add(dicT, cases);
                                    }
                                    {
                                        var cases = new BigInteger[Math.Min(m - p, a) + 1];
                                        for (var j = 0; j <= m - p && j <= a; j++)
                                            cases[j] = BinomialHelper.Binomial(a, j) *
                                                       BinomialHelper.Binomial(n - a - b - c, m - p - j);

                                        dicT = Add(dicT, cases);
                                    }
                                }
                                Merge(dicT, dic);

                                stack.RemoveAt(stack.Count - 1);
                            }
                            if (stack.Count == 0)
                                break;
                            if (mns <= 0)
                            {
                                stack.RemoveAt(stack.Count - 1);
                                if (stack.Count == 0)
                                    break;
                            }
                            stack[stack.Count - 1]++;
                        }
                        else if (stack[stack.Count - 1] <= max[stack.Count - 1])
                            stack.Add(0);
                        else
                        {
                            stack.RemoveAt(stack.Count - 1);
                            if (stack.Count == 0)
                                break;
                            stack[stack.Count - 1]++;
                        }
                }

                m_DistCondCache[par] = dic;
            }

            return dic;
        }
    }
}
