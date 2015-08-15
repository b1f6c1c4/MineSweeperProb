using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;

namespace MineSweeperCalc.Solver
{
    /// <summary>
    ///     求解器
    /// </summary>
    /// <typeparam name="T">单个格的类型</typeparam>
    public sealed partial class Solver<T>
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
            /// <summary>
            ///     组成部分
            /// </summary>
            public List<int> BlockSets { get; set; }

            /// <summary>
            ///     总雷数
            /// </summary>
            public int Mines { get; set; }
        }

        /// <summary>
        ///     解空间
        /// </summary>
        // ReSharper disable once MemberCanBePrivate.Global
        public List<Solution<T>> Solutions { get; private set; }

        /// <summary>
        ///     数学期望
        /// </summary>
        // ReSharper disable once MemberCanBePrivate.Global
        // ReSharper disable once CollectionNeverQueried.Global
        public IDictionary<BlockSet<T>, double> Expectation { get; private set; }

        /// <summary>
        ///     概率
        /// </summary>
        public IDictionary<T, double> Probability { get; private set; }

        /// <summary>
        ///     总状态数
        /// </summary>
        public BigInteger TotalStates { get; private set; }

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
                List<T> exceptA, exceptB, intersection;
                Overlap(m_BlockSets[i], theSet, out exceptA, out exceptB, out intersection);
                if (intersection.Count == 0)
                    continue;

                var newA = new BlockSet<T>(exceptA);
                var newB = new BlockSet<T>(exceptB);
                var newC = new BlockSet<T>(intersection);

                theSet = newB;
                if (!newA.Any)
                {
                    m_BlockSets[i] = newC;
                    blkLst.Add(i);
                }
                else
                {
                    m_BlockSets[i] = newA;
                    if (newC.Any)
                    {
                        foreach (var restrain in m_Restrains)
                        {
                            if (restrain.BlockSets.BinarySearch(i) >= 0)
                                restrain.BlockSets.Add(m_BlockSets.Count);
                        }
                        m_BlockSets.Add(newC);
                    }
                }
                if (!theSet.Any)
                    break;
            }
            
            for (var i = count; i < m_BlockSets.Count; i++)
                blkLst.Add(i);
            if (theSet.Any)
            {
                blkLst.Add(m_BlockSets.Count);
                m_BlockSets.Add(theSet);
            }

            return blkLst;
        }

        /// <summary>
        ///     交叉
        /// </summary>
        /// <param name="setA">格的集合A</param>
        /// <param name="setB">格的集合B</param>
        /// <param name="exceptA">A差B</param>
        /// <param name="exceptB">B差A</param>
        /// <param name="intersection">A交B</param>
        private static void Overlap(IReadOnlyList<T> setA, IReadOnlyList<T> setB,
                                    out List<T> exceptA, out List<T> exceptB, out List<T> intersection)
        {
            int p = 0, q = 0;
            intersection = new List<T>();
            exceptA = new List<T>();
            exceptB = new List<T>();
            for (; p < setA.Count && q < setB.Count;)
            {
                var cmp = setA[p].CompareTo(setB[q]);
                if (cmp < 0)
                    exceptA.Add(setA[p++]);
                else if (cmp > 0)
                    exceptB.Add(setB[q++]);
                else
                {
                    intersection.Add(setB[q]);
                    p++;
                    q++;
                }
            }
            while (p < setA.Count)
                exceptA.Add(setA[p++]);
            while (q < setB.Count)
                exceptB.Add(setB[q++]);
        }

        /// <summary>
        ///     交叉
        /// </summary>
        /// <param name="setA">格的集合A</param>
        /// <param name="setB">格的集合B</param>
        /// <param name="exceptA">A差B</param>
        /// <param name="exceptB">B差A</param>
        /// <param name="intersection">A交B</param>
        private static void Overlap(BlockSet<T> setA, BlockSet<T> setB,
                                    out List<T> exceptA, out List<T> exceptB, out List<T> intersection)
            => Overlap(setA.Blocks, setB.Blocks, out exceptA, out exceptB, out intersection);

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
        public void Solve()
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

            ProcessSolutions(result);

            m_DistCache.Clear();
            m_DistCacheLock.Clear();
            m_DistCondCache.Clear();
            m_DistCondCacheLock.Clear();
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
                        if (m_Restrains[i].BlockSets.BinarySearch(j) >= 0)
                            foreach (var block in m_BlockSets[j].Blocks)
                                m_Manager.SetStatus(block, BlockStatus.Blank);
                    m_Restrains.RemoveAt(i--);
                }
                else if (m_Restrains[i].Mines == m_Restrains[i].BlockSets.Select(index=>cnts[index]).Sum())
                {
                    for (var j = 0; j < m_BlockSets.Count; j++)
                        if (m_Restrains[i].BlockSets.BinarySearch(j) >= 0)
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
                        for (var j = 0; j < restrain.BlockSets.Count; j++)
                            if (restrain.BlockSets[j] == i)
                            {
                                restrain.Mines -= mines;
                                restrain.BlockSets.RemoveAt(j--);
                            }
                            else if (restrain.BlockSets[j] > i)
                                restrain.BlockSets[j]--;
                    m_BlockSets.RemoveAt(i--);
                    continue;
                }
                if (mines == 0 &&
                    blanks == 0)
                    continue;
                flag = true;
                m_BlockSets[i] = new BlockSet<T>(lst);
                foreach (var restrain in m_Restrains)
                    if (restrain.BlockSets.BinarySearch(i) >= 0)
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
                foreach (var index in m_Restrains[row].BlockSets)
                    argMat[index, row] = 1;
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
        private void ProcessSolutions(List<Solution<T>> solutions)
        {
            var exp = new Dictionary<BlockSet<T>, BigInteger>();
            var total = BigInteger.Zero;
            foreach (var so in solutions)
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
            foreach (var so in solutions)
                so.Ratio = so.States.Over(total);

            Solutions = solutions;

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

            TotalStates = total;
        }
    }
}
