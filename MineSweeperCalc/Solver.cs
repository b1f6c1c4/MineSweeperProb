using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Threading;
using System.Threading.Tasks;

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
        ///     待更新的格集合
        /// </summary>
        private ConcurrentQueue<BlockSet<T>> m_Updating;

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
        public void Solve(int numProc)
        {
            ReduceRestrainsExhaust();

            SetupSubRestrains();

            var runningThreadCount = 0;
            var lockObj = new object();

            Action proc = () => WorkingProcess(ref runningThreadCount, lockObj);

            if (numProc > 0)
            {
                var f = new TaskFactory();
                var tasks = new Task[numProc];
                for (var i = 0; i < numProc; i++)
                    tasks[i] = f.StartNew(proc);
                Task.WaitAll(tasks);
            }
            else
                proc();

            if (m_Updating.Any())
                throw new Exception("WTF");

            Solutions = new List<Solution<T>>();
            Traversal(new KeyValuePair<BlockSet<T>, int>[] { }, BigInteger.One, m_SubRestrains);

            ProcessSolutions();
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
                    if (exp.ContainsKey(s))
                        exp[s] += v;
                    else
                        exp[s] = v;
                }
            }

            Expectation = new Dictionary<BlockSet<T>, double>();
            Probability = new Dictionary<T, double>();
            foreach (var block in m_Manager.Keys)
            {
                if (m_Manager[block] == BlockStatus.Mine)
                    Probability[block] = 1;
                else if (m_Manager[block] == BlockStatus.Blank)
                    Probability[block] = 0;
            }
            foreach (var kvp in exp)
            {
                if (kvp.Value == 0)
                    foreach (var block in kvp.Key)
                        m_Manager.SetStatus(block, BlockStatus.Blank);
                else if (kvp.Value == total * kvp.Key.Count)
                    foreach (var block in kvp.Key)
                        m_Manager.SetStatus(block, BlockStatus.Mine);

                Expectation.Add(kvp.Key, BigIntegerHelper.Ratio(kvp.Value, total));

                var p = BigIntegerHelper.Ratio(kvp.Value, total) / kvp.Key.Count;
                foreach (var block in kvp.Key)
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
        private void Traversal(IReadOnlyCollection<KeyValuePair<BlockSet<T>, int>> path,
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

            for (var mines = iv.MinInclusive; mines <= iv.MaxInclusive; mines++)
            {
                var dic = new Dictionary<BlockSet<T>, Interval>(subRestrains);
                dic.Remove(set);

                var newPath = path.Concat(new[] { new KeyValuePair<BlockSet<T>, int>(set, mines) }).ToArray();

                Traversal(newPath, states * m_BinomialHelper.Binomial(set.Count, mines), dic);
            }
        }

        private Interval GetInterval(IReadOnlyCollection<KeyValuePair<BlockSet<T>, int>> path, IDictionary<BlockSet<T>, Interval> subRestrains, BlockSet<T> set)
        {
            var iv = subRestrains[set];
            foreach (var r in m_Restrains.Where(r => r.TheBlocks.Contains(set)))
            {
                var lb = 0;
                var ub = 0;
                foreach (var s in r.TheBlocks)
                    if (path.Any(kvp => kvp.Key.Equals(s)))
                    {
                        var val = path.First(kvp => kvp.Key.Equals(s)).Value;
                        lb += val;
                        ub += val;
                    }
                    else if (!s.Equals(set) &&
                             subRestrains.ContainsKey(s))
                    {
                        var ivO = subRestrains[s];
                        lb += ivO.MinInclusive;
                        ub += ivO.MaxInclusive;
                    }
                iv = iv.Intersect(
                                  new Interval
                                      {
                                          MinInclusive = r.MinInclusive - ub,
                                          MaxInclusive = r.MaxInclusive - lb
                                      });
            }
            return iv;
        }

        /// <summary>
        ///     工作进程的入口
        /// </summary>
        /// <param name="runningThreadCount">正在运行的进程数</param>
        /// <param name="lockObj">锁</param>
        private void WorkingProcess(ref int runningThreadCount, object lockObj)
        {
            while (true)
            {
                BlockSet<T> set;
                if (m_Updating.TryDequeue(out set))
                {
                    Interlocked.Increment(ref runningThreadCount);
                    Update(set);
                    Interlocked.Decrement(ref runningThreadCount);
                }
                else
                {
                    lock (lockObj)
                    {
                        if (runningThreadCount != 0)
                            continue;
                        if (!m_Updating.TryDequeue(out set))
                            break;
                    }
                    Interlocked.Increment(ref runningThreadCount);
                    Update(set);
                    Interlocked.Decrement(ref runningThreadCount);
                }
            }
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
                foreach (var block in r.TheBlocks[i])
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

                var newSet = new BlockSet<T>(r.TheBlocks[i].Except(set));
                r.TheBlocks.RemoveAt(i);
                if (newSet.Any())
                    r.TheBlocks.Insert(i, newSet);
                else
                    i--;
            }
            if (mines > 0)
                r.Interval = new Interval
                                 {
                                     MinInclusive = r.MinInclusive - mines,
                                     MaxInclusive = r.MaxInclusive - mines
                                 };

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

        /// <summary>
        ///     生成原始子约束
        /// </summary>
        private void SetupSubRestrains()
        {
            m_SubRestrains = new ConcurrentDictionary<BlockSet<T>, Interval>();
            m_Updating = new ConcurrentQueue<BlockSet<T>>();
            foreach (var r in m_Restrains)
                foreach (var set in r.TheBlocks)
                {
                    var iv =
                        Interval.From(set)
                                .Intersect(
                                           new Interval
                                               {
                                                   MinInclusive = r.MinInclusive - (r.TheBlocks.CountBlock - set.Count),
                                                   MaxInclusive = r.MaxInclusive
                                               });
                    m_SubRestrains.AddOrUpdate(set, iv, (s, ivO) => ivO.Intersect(iv));
                    m_Updating.Enqueue(set);
                }
        }

        /// <summary>
        ///     更新待更新的格集合
        ///     <remarks>注意：此方法在工作线程上运行，不可以写<c>m_Manager</c>、<c>m_Restrains</c>。</remarks>
        /// </summary>
        /// <param name="set">格集合</param>
        private void Update(BlockSet<T> set)
        {
            var iv = m_SubRestrains[set];

            var updating = new HashSet<BlockSet<T>>();
            foreach (var r in m_Restrains)
            {
                if (!r.TheBlocks.Contains(set))
                    continue;

                var lcnt = 0;
                var ucnt = 0;
                foreach (var s in r.TheBlocks)
                {
                    if (s.Equals(set))
                        continue;

                    updating.Add(s);
                    var ivS = m_SubRestrains[s];
                    lcnt += ivS.MinInclusive;
                    ucnt += ivS.MaxInclusive;
                }

                var ivN = new Interval
                              {
                                  MinInclusive = r.MinInclusive - ucnt,
                                  MaxInclusive = r.MaxInclusive - lcnt
                              };
                iv = iv.Intersect(ivN);
            }

            var flag = false;
            m_SubRestrains.AddOrUpdate(
                                       set,
                                       iv,
                                       (s, ivO) =>
                                       {
                                           if (iv.MinInclusive <= ivO.MinInclusive &&
                                               iv.MaxInclusive >= ivO.MaxInclusive)
                                               return ivO;

                                           flag = true;
                                           return iv.Intersect(ivO);
                                       });
            if (!flag)
                return;

            foreach (var s in updating)
                m_Updating.Enqueue(s);
        }
    }
}
