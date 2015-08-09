using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;

namespace MineSweeperCalc
{
    /// <summary>
    ///     求解器
    /// </summary>
    /// <typeparam name="T">单个格的类型</typeparam>
    public sealed class Solver<T>
        where T : IBlock<T>
    {
        /// <summary>
        ///     当前状态
        /// </summary>
        private readonly BlockManager<T> m_Manager;

        /// <summary>
        ///     所有约束
        /// </summary>
        private readonly List<Restrain<T>> m_Restrains;

        /// <summary>
        ///     各格集合上添加的子约束
        /// </summary>
        private ConcurrentDictionary<BlockSet<T>, Interval> m_SubRestrains;

        /// <summary>
        ///     二项式系数计算器
        /// </summary>
        private readonly BinomialHelper m_BinomialHelper;

        /// <summary>
        ///     解空间
        /// </summary>
        public List<Solution<T>> Solutions { get; private set; }

        /// <summary>
        ///     数学期望
        /// </summary>
        public IDictionary<BlockSet<T>, double> Expectation { get; private set; }

        /// <summary>
        ///     概率
        /// </summary>
        public IDictionary<T, double> Probability { get; private set; }

        public Solver(IEnumerable<T> blocks, BinomialHelper binomialHelper)
        {
            m_BinomialHelper = binomialHelper;
            m_Manager = new BlockManager<T>(blocks);
            m_Restrains = new List<Restrain<T>>();
        }

        /// <summary>
        ///     添加约束
        /// </summary>
        /// <param name="r"></param>
        public void AddRestrain(Restrain<T> r)
        {
            r.TheBlocks.Coordinate(m_Restrains.Select(rr => rr.TheBlocks));
            m_Restrains.Add(r);
        }

        public BlockStatus this[T key] => m_Manager[key];

        /// <summary>
        ///     求解
        /// </summary>
        public void Solve()
        {
            m_SubRestrains = new ConcurrentDictionary<BlockSet<T>, Interval>();
            var flag = true;
            while (flag)
            {
                flag = false;
                ReduceRestrainsExhaust();
                m_SubRestrains.Clear();

                m_Restrains.ForEach(r => r.TheBlocks.Sort());
                for (var i = 0; i < m_Restrains.Count; i++)
                    for (var j = 0; j < i; j++)
                        flag |= SimpleOverlap(m_Restrains[i], m_Restrains[j]);
            }

            Solutions = new List<Solution<T>>();
            Traversal(new Dictionary<BlockSet<T>, int>(), BigInteger.One, m_SubRestrains);

            ProcessSolutions();
        }

        private bool SimpleOverlap(Restrain<T> first, Restrain<T> second)
        {
            var lstA = new List<BlockSet<T>>();
            var lstB = new List<BlockSet<T>>();
            var lstC = new List<BlockSet<T>>();
            var p = 0;
            var q = 0;
            while (p < first.TheBlocks.Count &&
                   q < second.TheBlocks.Count)
            {
                var cmp = first.TheBlocks[p].CompareTo(second.TheBlocks[q]);
                if (cmp > 0)
                    lstA.Add(first.TheBlocks[p++]);
                else if (cmp < 0)
                    lstB.Add(second.TheBlocks[q++]);
                else
                {
                    lstC.Add(first.TheBlocks[p]);
                    p++;
                    q++;
                }
            }
            while (p < first.TheBlocks.Count)
                lstA.Add(first.TheBlocks[p++]);
            while (q < second.TheBlocks.Count)
                lstB.Add(second.TheBlocks[q++]);

            var sumA = lstA.Sum(set => set.Count);
            var sumB = lstB.Sum(set => set.Count);
            var sumC = lstC.Sum(set => set.Count);

            var ivA = (first.Interval - new Interval(0, sumC)).Intersect(new Interval(0, sumA));
            var ivB = (second.Interval - new Interval(0, sumC)).Intersect(new Interval(0, sumB));
            var ivC = (first.Interval - ivA).Intersect(second.Interval - ivB);

            Func<List<BlockSet<T>>, Interval, bool> process
                = (sets, iv) =>
                  {
                      if (sets.Count == 0)
                          return false;
                      var sum = sets.Sum(s => s.Count);
                      if (sum == iv.MinInclusive)
                          foreach (var set in sets)
                          {
                              UpdateSubRestrain(set, new Interval(set.Count));
                              foreach (var block in set.Blocks)
                                  m_Manager.SetStatus(block, BlockStatus.Mine);
                          }
                      else if (iv.MaxInclusive == 0)
                          foreach (var set in sets)
                          {
                              UpdateSubRestrain(set, new Interval(0));
                              foreach (var block in set.Blocks)
                                  m_Manager.SetStatus(block, BlockStatus.Blank);
                          }
                      else
                      {
                          foreach (var set in sets)
                              UpdateSubRestrain(
                                                set,
                                                (new Interval(0, set.Count))
                                                    .Intersect(iv - new Interval(0, sum - set.Count)));
                          return false;
                      }
                      return true;
                  };

            var flag = false;
            flag |= process(lstA, ivA);
            flag |= process(lstB, ivB);
            flag |= process(lstC, ivC);

            return flag;
        }

        private void UpdateSubRestrain(BlockSet<T> set, Interval iv)
        {
            Interval ivO;
            if (m_SubRestrains.TryGetValue(set, out ivO))
                m_SubRestrains[set] = ivO.Intersect(iv);
            else
                m_SubRestrains[set] = iv;
        }

        private void ProcessSolutions()
        {
            var exp = new Dictionary<BlockSet<T>, BigInteger>();
            var total = BigInteger.Zero;
            foreach (var so in Solutions)
            {
                total += so.States;
                foreach (var s in m_SubRestrains.Keys)
                {
                    var v = so.States * so.Distribution[s];
                    BigInteger oldV;
                    if (exp.TryGetValue(s, out oldV))
                        exp[s] = oldV + v;
                    else
                        exp[s] = v;
                }
            }

            Expectation = new Dictionary<BlockSet<T>, double>();
            Probability = new Dictionary<T, double>();
            foreach (var block in m_Manager.Keys)
                if (m_Manager[block] == BlockStatus.Mine)
                    Probability[block] = 1;
                else if (m_Manager[block] == BlockStatus.Blank)
                    Probability[block] = 0;
            foreach (var kvp in exp)
            {
                if (kvp.Value == 0)
                    foreach (var block in kvp.Key.Blocks)
                        m_Manager.SetStatus(block, BlockStatus.Blank);
                else if (kvp.Value == total * kvp.Key.Count)
                    foreach (var block in kvp.Key.Blocks)
                        m_Manager.SetStatus(block, BlockStatus.Mine);

                Expectation.Add(kvp.Key, BigIntegerHelper.Ratio(kvp.Value, total));

                var p = BigIntegerHelper.Ratio(kvp.Value, total) / kvp.Key.Count;
                foreach (var block in kvp.Key.Blocks)
                    Probability.Add(block, p);
            }

            foreach (var so in Solutions)
                so.Ratio = BigIntegerHelper.Ratio(so.States, total);
        }

        /// <summary>
        ///     遍历解空间
        /// </summary>
        /// <param name="path">当前路径</param>
        /// <param name="states">当前状态数</param>
        /// <param name="subRestrains">待遍历解空间</param>
        private void Traversal(IDictionary<BlockSet<T>, int> path,
                               BigInteger states,
                               IDictionary<BlockSet<T>, Interval> subRestrains)
        {
            if (!subRestrains.Keys.Any())
            {
                Solutions.Add(
                              new Solution<T>
                                  {
                                      Distribution = path.ToDictionary(kvp => kvp.Key, kvp => kvp.Value),
                                      States = states
                                  });
                return;
            }

            var set = subRestrains.Keys.First();
            var iv = GetInterval(path, subRestrains, set);

            var newPath = new Dictionary<BlockSet<T>, int>(path);
            for (var mines = iv.MinInclusive; mines <= iv.MaxInclusive; mines++)
            {
                var dic = new Dictionary<BlockSet<T>, Interval>(subRestrains);
                dic.Remove(set);

                newPath[set] = mines;

                Traversal(newPath, states * m_BinomialHelper.Binomial(set.Count, mines), dic);
            }
        }

        private Interval GetInterval(IDictionary<BlockSet<T>, int> path,
                                     IDictionary<BlockSet<T>, Interval> subRestrains, BlockSet<T> set)
        {
            var iv = subRestrains[set];
            foreach (var r in m_Restrains.Where(r => r.TheBlocks.Contains(set)))
            {
                var ivN = new Interval(0, 0);
                foreach (var s in r.TheBlocks)
                {
                    if (s.Equals(set))
                        continue;
                    int val;
                    if (path.TryGetValue(s, out val))
                    {
                        ivN.MinInclusive += val;
                        ivN.MaxInclusive += val;
                    }
                    else
                    {
                        Interval ivO;
                        if (subRestrains.TryGetValue(s, out ivO))
                        {
                            ivN.MinInclusive += ivO.MinInclusive;
                            ivN.MaxInclusive += ivO.MaxInclusive;
                        }
                    }
                }
                iv = iv.Intersect(r.Interval - ivN);
            }
            return iv;
        }

        /// <summary>
        ///     重复化简约束直至无法继续
        /// </summary>
        private bool ReduceRestrainsExhaust()
        {
            var fflag = false;
            var flag = true;
            while (flag)
            {
                flag = false;
                for (var i = 0; i < m_Restrains.Count; i++)
                {
                    if (!ReduceRestrain(m_Restrains[i]))
                        continue;
                    m_Restrains.RemoveAt(i);
                    flag = true;
                    i--;
                }
                fflag |= flag;
            }
            return fflag;
        }

        /// <summary>
        ///     化简约束
        /// </summary>
        /// <param name="r">约束</param>
        /// <returns>是否引起了状态变化</returns>
        private bool ReduceRestrain(Restrain<T> r)
        {
            if (!r.Exist)
                throw new InvalidOperationException("无解");

            var mines = 0;
            for (var i = 0; i < r.TheBlocks.Count; i++)
            {
                var set = new HashSet<T>();
                foreach (var block in r.TheBlocks[i].Blocks)
                    switch (m_Manager[block])
                    {
                        case BlockStatus.Blank:
                            set.Add(block);
                            break;
                        case BlockStatus.Mine:
                            mines++;
                            set.Add(block);
                            break;
                    }
                if (!set.Any())
                    continue;

                var newSet = new BlockSet<T>(r.TheBlocks[i].Blocks.Except(set));
                r.TheBlocks.RemoveAt(i);
                if (newSet.Any)
                    r.TheBlocks.Insert(i, newSet);
                else
                    i--;
            }
            if (mines > 0)
                r.Interval = r.Interval - new Interval(mines);

            if (!r.Exist)
                throw new InvalidOperationException("无解");

            if (r.TheBlocks.CountBlock == 0)
                return true;

            if (!r.Unique)
                return false;

            if (r.MaxInclusive == 0)
                foreach (var block in r.TheBlocks.Blocks)
                    m_Manager.SetStatus(block, BlockStatus.Blank);
            else // if (r.MinInclusive == r.TheBlocks.CountBlock)
                foreach (var block in r.TheBlocks.Blocks)
                    m_Manager.SetStatus(block, BlockStatus.Mine);
            return true;
        }
    }
}
