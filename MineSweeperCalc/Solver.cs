using System;
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
        ///     待求解单元
        /// </summary>
        private readonly List<BlockSet<T>> m_BlockSets;

        /// <summary>
        ///     约束
        /// </summary>
        private readonly List<Restrain> m_Restrains;

        /// <summary>
        ///     约束
        /// </summary>
        private sealed class Restrain
        {
            public List<int> BlockSets { get; set; }
            public int Mines { get; set; }
        }

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

        public Solver(IEnumerable<T> blocks)
        {
            m_Manager = new BlockManager<T>(blocks);
            m_Restrains = new List<Restrain>();
            m_BlockSets = new List<BlockSet<T>>();
        }

        public Solver(Solver<T> other)
        {
            m_Manager = new BlockManager<T>(other.m_Manager);
            m_Restrains = new List<Restrain>(other.m_Restrains.Count);
            foreach (var restrain in other.m_Restrains)
                m_Restrains.Add(new Restrain { BlockSets = new List<int>(restrain.BlockSets), Mines = restrain.Mines });
            m_BlockSets = new List<BlockSet<T>>(other.m_BlockSets);
        }

        /// <summary>
        ///     准备约束
        /// </summary>
        /// <param name="set">格的集合</param>
        /// <returns>约束</returns>
        private List<int> OverlapRestrain(BlockSet<T> set)
        {
            var theSet = set;
            var count = m_BlockSets.Count;
            var blkLst = new List<int>(m_BlockSets.Count);
            for (var i = 0; i < count; i++)
            {
                var intersection = m_BlockSets[i].Blocks.Intersect(theSet.Blocks).ToArray();
                if (intersection.Length == 0)
                {
                    blkLst.Add(0);
                    continue;
                }

                var newA = new BlockSet<T>(m_BlockSets[i].Blocks.Except(intersection));
                var newB = new BlockSet<T>(theSet.Blocks.Except(intersection));
                var newC = new BlockSet<T>(intersection);

                theSet = newB;
                if (!newA.Any)
                {
                    m_BlockSets[i] = newC;
                    blkLst.Add(1);
                }
                else
                {
                    m_BlockSets[i] = newA;
                    blkLst.Add(0);
                    if (newC.Any)
                    {
                        m_BlockSets.Add(newC);
                        foreach (var restrain in m_Restrains)
                            restrain.BlockSets.Add(restrain.BlockSets[i]);
                    }
                }
                if (!theSet.Any)
                    break;
            }

            for (var i = blkLst.Count; i < count; i++)
                blkLst.Add(0); // oldA
            for (var i = count; i < m_BlockSets.Count; i++)
                blkLst.Add(1); // newC
            if (theSet.Any)
            {
                m_BlockSets.Add(theSet);
                foreach (var restrain in m_Restrains)
                    restrain.BlockSets.Add(0);
                blkLst.Add(1); // theSet
            }

            return blkLst;
        }

        /// <summary>
        ///     添加约束
        /// </summary>
        /// <param name="set">格的集合</param>
        /// <param name="mines">数</param>
        public void AddRestrain(BlockSet<T> set, int mines)
        {
            int dMines, dBlanks;
            var setN = OverlapRestrain(new BlockSet<T>(ReduceSet(set, out dMines, out dBlanks)));
            m_Restrains.Add(new Restrain { BlockSets = setN, Mines = mines - dMines });
        }

        public BlockStatus this[T key] => m_Manager[key];

        /// <summary>
        ///     求解
        /// </summary>
        public BigInteger Solve()
        {
            while (ReduceRestrains()) { }

            var augmentedMatrixT = GenerateMatrix();
            var minors = Gauss(augmentedMatrixT);

            if (minors[minors.Count - 1] == m_BlockSets.Count)
                minors.RemoveAt(minors.Count - 1);
            else
                throw new ApplicationException("无解");

            var result = EnumerateSolutions(minors, augmentedMatrixT);
            if (result.Count == 0)
                throw new ApplicationException("无解");

            Solutions = result;

            return ProcessSolutions();
        }

        /// <summary>
        ///     化简格的集合
        /// </summary>
        /// <param name="set">格的集合</param>
        /// <param name="mines">雷数</param>
        /// <param name="blanks">空格数</param>
        /// <returns></returns>
        private List<T> ReduceSet(BlockSet<T> set, out int mines, out int blanks)
        {
            mines = 0;
            blanks = 0;
            var lst = new List<T>();
            foreach (var block in set.Blocks)
                switch (m_Manager[block])
                {
                    case BlockStatus.Mine:
                        mines++;
                        break;
                    case BlockStatus.Blank:
                        blanks++;
                        break;
                    default:
                        lst.Add(block);
                        break;
                }
            return lst;
        }

        /// <summary>
        ///     化简约束
        /// </summary>
        /// <returns>是否成功化简</returns>
        private bool ReduceRestrains()
        {
            var flag = false;

            var cnts = m_BlockSets.Select(b => b.Count).ToArray();
            for (var i = 0; i < m_Restrains.Count; i++)
                if (m_Restrains[i].Mines == 0)
                {
                    for (var j = 0; j < m_BlockSets.Count; j++)
                        if (m_Restrains[i].BlockSets[j] != 0)
                            foreach (var block in m_BlockSets[j].Blocks)
                                m_Manager.SetStatus(block, BlockStatus.Blank);
                    m_Restrains.RemoveAt(i--);
                }
                else if (m_Restrains[i].Mines == m_Restrains[i].BlockSets.Zip(cnts, (c, cn) => c * cn).Sum())
                {
                    for (var j = 0; j < m_BlockSets.Count; j++)
                        if (m_Restrains[i].BlockSets[j] != 0)
                            foreach (var block in m_BlockSets[j].Blocks)
                                m_Manager.SetStatus(block, BlockStatus.Mine);
                    m_Restrains.RemoveAt(i--);
                }

            for (var i = 0; i < m_BlockSets.Count; i++)
            {
                int mines, blanks;
                var lst = ReduceSet(m_BlockSets[i], out mines, out blanks);
                if (m_BlockSets[i].Count == blanks + mines)
                {
                    foreach (var restrain in m_Restrains)
                    {
                        if (restrain.BlockSets[i] != 0)
                            restrain.Mines -= mines;
                        restrain.BlockSets.RemoveAt(i);
                    }
                    m_BlockSets.RemoveAt(i--);
                    continue;
                }
                if (mines == 0 &&
                    blanks == 0)
                    continue;
                flag = true;
                m_BlockSets[i] = new BlockSet<T>(lst);
                foreach (var restrain in m_Restrains)
                    if (restrain.BlockSets[i] != 0)
                        restrain.Mines -= mines;
            }

            return flag;
        }

        /// <summary>
        ///     生成矩阵
        /// </summary>
        /// <returns>增广矩阵的转置</returns>
        private double[,] GenerateMatrix()
        {
            var m = m_Restrains.Count;
            var n = m_BlockSets.Count;

            var argMat = new double[n + 1, m];
            for (var row = 0; row < m; row++)
            {
                for (var col = 0; col < n; col++)
                    argMat[col, row] = m_Restrains[row].BlockSets[col];
                argMat[n, row] = m_Restrains[row].Mines;
            }
            return argMat;
        }

        /// <summary>
        ///     对矩阵进行高斯消元
        /// </summary>
        /// <param name="matrix">矩阵</param>
        /// <returns>非主元列</returns>
        private static List<int> Gauss(double[,] matrix)
        {
            var n = matrix.GetLength(0);
            var m = matrix.GetLength(1);
            var minorCol = new List<int>();
            var major = 0;
            for (var col = 0; col < n; col++)
            {
                int biasRow;
                for (biasRow = major; biasRow < m; biasRow++)
                    if (Math.Abs(matrix[col, biasRow]) >= 1E-8)
                        break;
                if (biasRow >= m)
                {
                    minorCol.Add(col);
                    continue;
                }

                var vec = new double[m];
                var theBiasInv = 1 / matrix[col, biasRow];
                for (var row = 0; row < m; row++)
                {
                    if (row != biasRow)
                        vec[row] = -matrix[col, row] * theBiasInv;
                    if (row == major)
                        matrix[col, row] = 1;
                    else
                        matrix[col, row] = 0;
                }

                for (var co = col + 1; co < n; co++)
                {
                    var swap = matrix[co, major];
                    var bias = matrix[co, biasRow];
                    for (var row = 0; row < m; row++)
                        if (row == major)
                            matrix[co, row] = bias * theBiasInv;
                        else if (row == biasRow)
                            matrix[co, row] = swap + vec[major] * bias;
                        else
                            matrix[co, row] += vec[row] * bias;
                }
                major++;
            }
            return minorCol;
        }

        /// <summary>
        ///     遍历解空间
        /// </summary>
        /// <param name="minors">非主元列</param>
        /// <param name="augmentedMatrixT">增广矩阵的转置</param>
        /// <returns>所有解</returns>
        private List<Solution<T>> EnumerateSolutions(IReadOnlyList<int> minors, double[,] augmentedMatrixT)
        {
            var n = m_BlockSets.Count;
            var counts = m_BlockSets.Select(b => b.Count).ToArray();

            var res = new List<Solution<T>>();
            if (minors.Count == 0)
            {
                var lst = new List<int>(n);
                for (var i = 0; i < n; i++)
                    lst.Add((int)Math.Round(augmentedMatrixT[n, i]));
                res.Add(new Solution<T>(lst));
            }
            else
            {
                var mR = n - minors.Count;
                var majors = new int[mR];
                var cnts = new int[mR];
                var sums = new double[mR];
                {
                    var minorID = 0;
                    var mainRow = 0;
                    for (var col = 0; col < n; col++)
                        if (minorID < minors.Count &&
                            col == minors[minorID])
                            minorID++;
                        else // major
                        {
                            majors[mainRow] = col;
                            cnts[mainRow] = counts[col];
                            sums[mainRow] = augmentedMatrixT[n, mainRow];
                            mainRow++;
                        }
                }
                Action<int, int> aggr =
                    (minor, val) =>
                    {
                        for (var mainRow = 0; mainRow < mR; mainRow++)
                            sums[mainRow] -= val * augmentedMatrixT[minors[minor], mainRow];
                    };
                var stack = new List<int>(minors.Count) { 0 }; // aggr(0, 0);
                while (true)
                    if (stack.Count == minors.Count)
                        if (stack[stack.Count - 1] <= counts[minors[stack.Count - 1]])
                        {
                            var lst = new int[n];
                            var flag = true;
                            for (var mainRow = 0; mainRow < mR; mainRow++)
                            {
                                var val = (int)Math.Round(sums[mainRow]);
                                if (val < 0 ||
                                    val > cnts[mainRow])
                                {
                                    flag = false;
                                    break;
                                }
                                lst[majors[mainRow]] = val;
                            }
                            if (flag)
                            {
                                for (var minorID = 0; minorID < minors.Count; minorID++)
                                    lst[minors[minorID]] = stack[minorID];
                                res.Add(new Solution<T>(lst.ToList()));
                            }

                            aggr(stack.Count - 1, 1);
                            stack[stack.Count - 1]++;
                        }
                        else
                        {
                            aggr(stack.Count - 1, -stack[stack.Count - 1]);
                            stack.RemoveAt(stack.Count - 1);
                            if (stack.Count == 0)
                                break;
                            aggr(stack.Count - 1, 1);
                            stack[stack.Count - 1]++;
                        }
                    else if (stack[stack.Count - 1] <= counts[minors[stack.Count - 1]])
                        stack.Add(0); // aggr(stack.Count - 1, 0);
                    else
                    {
                        aggr(stack.Count - 1, -stack[stack.Count - 1]);
                        stack.RemoveAt(stack.Count - 1);
                        if (stack.Count == 0)
                            break;
                        aggr(stack.Count - 1, 1);
                        stack[stack.Count - 1]++;
                    }
            }
            return res;
        }

        /// <summary>
        ///     处理解及解空间
        /// </summary>
        private BigInteger ProcessSolutions()
        {
            var exp = new Dictionary<BlockSet<T>, BigInteger>();
            var total = BigInteger.Zero;
            foreach (var so in Solutions)
            {
                so.States = BigInteger.One;
                so.Distribution = new Dictionary<BlockSet<T>, int>(m_BlockSets.Count);
                for (var i = 0; i < m_BlockSets.Count; i++)
                {
                    so.States *= BinomialHelper.Binomial(m_BlockSets[i].Count, so.Dist[i]);
                    so.Distribution.Add(m_BlockSets[i], so.Dist[i]);
                }
                total += so.States;
                foreach (var s in m_BlockSets)
                {
                    var v = so.States * so.Distribution[s];
                    BigInteger oldV;
                    if (exp.TryGetValue(s, out oldV))
                        exp[s] = oldV + v;
                    else
                        exp[s] = v;
                }
            }
            foreach (var so in Solutions)
                so.Ratio = so.States.Over(total);

            Probability = new Dictionary<T, double>();
            foreach (var block in m_Manager.Keys)
                if (m_Manager[block] == BlockStatus.Mine)
                    Probability[block] = 1;
                else if (m_Manager[block] == BlockStatus.Blank)
                    Probability[block] = 0;

            Expectation = new Dictionary<BlockSet<T>, double>();
            foreach (var kvp in exp)
            {
                if (kvp.Value == 0)
                    foreach (var block in kvp.Key.Blocks)
                        m_Manager.SetStatus(block, BlockStatus.Blank);
                else if (kvp.Value == total * kvp.Key.Count)
                    foreach (var block in kvp.Key.Blocks)
                        m_Manager.SetStatus(block, BlockStatus.Mine);

                var ratio = kvp.Value.Over(total);
                Expectation.Add(kvp.Key, ratio);

                var p = ratio / kvp.Key.Count;
                foreach (var block in kvp.Key.Blocks)
                    Probability.Add(block, p);
            }

            return total;
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
                var ins = m_BlockSets[i].Blocks.Intersect(lst).ToArray();
                lst.RemoveAll(ins.Contains);
                sets[i] = ins.Length;
            }
            return sets;
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
                var ins1 = m_BlockSets[i].Blocks.Intersect(lst1).ToArray();
                var ins2 = m_BlockSets[i].Blocks.Intersect(lst2).ToArray();
                var ins3 = ins1.Intersect(ins2).ToArray();
                lst1.RemoveAll(ins1.Contains);
                lst2.RemoveAll(ins2.Contains);
                sets1[i] = ins1.Length - ins3.Length;
                sets2[i] = ins2.Length - ins3.Length;
                sets3[i] = ins3.Length;
            }
        }

        /// <summary>
        ///     计算指定格的集合的雷数的分布
        /// </summary>
        /// <param name="set">格的集合，必须是完整的</param>
        /// <returns>分布</returns>
        public IDictionary<int, BigInteger> Distribution(BlockSet<T> set)
        {
            int dMines, dBlanks;
            var lst = ReduceSet(set, out dMines, out dBlanks);
            var sets = GetIntersectionCounts(new BlockSet<T>(lst));

            var dic = new Dictionary<int, BigInteger>();
            foreach (var solution in Solutions)
            {
                var dicT = new Dictionary<int, BigInteger> { { 0, 1 } };
                for (var i = 0; i < m_BlockSets.Count; i++)
                {
                    var n = m_BlockSets[i].Count;
                    var a = sets[i];
                    var m = solution.Dist[i];

                    var cases = new BigInteger[Math.Min(m, a) + 1];
                    for (var j = 0; j <= m && j <= a; j++)
                        cases[j] = BinomialHelper.Binomial(a, j) * BinomialHelper.Binomial(n - a, m - j);

                    dicT = Add(dicT, cases);
                }
                Merge(dicT, dic);
            }

            if (!dic.ContainsKey(0))
                dic.Add(0, 0);

            return dic.ToDictionary(kvp => kvp.Key + dMines, kvp => kvp.Value);
        }

        private static void Merge(IDictionary<int, BigInteger> from, IDictionary<int, BigInteger> to)
        {
            foreach (var kvp in from)
            {
                BigInteger old;
                if (to.TryGetValue(kvp.Key, out old))
                    to[kvp.Key] = old + kvp.Value;
                else
                    to.Add(kvp.Key, kvp.Value);
            }
        }

        private static Dictionary<int, BigInteger> Add(IDictionary<int, BigInteger> from,
                                                       IReadOnlyList<BigInteger> cases)
        {
            var dicN = new Dictionary<int, BigInteger>(from.Count);
            foreach (var kvp in from)
                for (var j = 0; j < cases.Count; j++)
                {
                    var key = kvp.Key + j;
                    BigInteger old;
                    if (dicN.TryGetValue(key, out old))
                        dicN[key] = old + kvp.Value * cases[j];
                    else
                        dicN.Add(key, kvp.Value * cases[j]);
                }
            return dicN;
        }

        /// <summary>
        ///     计算在一定条件下指定格的集合的雷数的分布
        /// </summary>
        /// <param name="set">格的集合，必须是完整的</param>
        /// <param name="setCond">用作条件的格的集合，必须是完整的</param>
        /// <param name="mines">用作条件的格的集合中的雷数</param>
        /// <returns>分布</returns>
        public IDictionary<int, BigInteger> DistributionConditioned(BlockSet<T> set, BlockSet<T> setCond, int mines)
        {
            int dMines, dBlanks;
            var lst = ReduceSet(set, out dMines, out dBlanks);

            int dMinesCond, dBlanksCond;
            var lstCond = ReduceSet(setCond, out dMinesCond, out dBlanksCond);

            if (lstCond.Count == 0)
            {
                if (dMinesCond == mines)
                    return new Dictionary<int, BigInteger> { { dMines, 1 } };
                return new Dictionary<int, BigInteger>();
            }

            int[] sets1, sets2, sets3;
            GetIntersectionCounts(
                                  new BlockSet<T>(lst),
                                  new BlockSet<T>(lstCond),
                                  out sets1,
                                  out sets2,
                                  out sets3);

            var dic = new Dictionary<int, BigInteger>();
            foreach (var solution in Solutions)
            {
                var max = new int[m_BlockSets.Count];
                for (var i = 0; i < m_BlockSets.Count; i++)
                    max[i] = Math.Min(solution.Dist[i], sets2[i] + sets3[i]);

                var stack = new List<int>(m_BlockSets.Count);
                if (m_BlockSets.Count > 1)
                    stack.Add(0);
                while (true)
                    if (stack.Count == m_BlockSets.Count - 1)
                    {
                        var mns = mines - (stack.Sum() + dMinesCond);
                        if (mns >= 0 &&
                            mns <= max[m_BlockSets.Count - 1])
                        {
                            stack.Add(mns);

                            var dicT = new Dictionary<int, BigInteger> { { 0, 1 } };
                            for (var i = 0; i < m_BlockSets.Count; i++)
                            {
                                var n = m_BlockSets[i].Count;
                                var a = sets1[i];
                                var b = sets2[i];
                                var c = sets3[i];
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

            if (!dic.ContainsKey(0))
                dic.Add(0, 0);

            return dic.ToDictionary(kvp => kvp.Key + dMines, kvp => kvp.Value);
        }
    }
}
