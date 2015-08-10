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
            m_Restrains = new List<Restrain>();
            m_BlockSets = new List<BlockSet<T>>();
        }

        /// <summary>
        ///     添加约束
        /// </summary>
        /// <param name="set">格的集合</param>
        /// <param name="mines">数</param>
        public void AddRestrain(BlockSet<T> set, int mines)
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
            m_Restrains.Add(
                            new Restrain
                                {
                                    BlockSets = blkLst,
                                    Mines = mines
                                });
        }

        public BlockStatus this[T key] => m_Manager[key];

        /// <summary>
        ///     求解
        /// </summary>
        public BigInteger Solve()
        {
            while (ReduceRestrains()) { }

            var augmentedMatrix = GenerateMatrix();
            var minors = Gauss(augmentedMatrix);

            if (minors[minors.Count - 1] == m_BlockSets.Count)
                minors.RemoveAt(minors.Count - 1);
            else
                throw new ApplicationException("无解");

            var result = EnumerateSolutions(minors, augmentedMatrix);
            if (result.Count == 0)
                throw new ApplicationException("无解");

            Solutions = result;

            return ProcessSolutions();
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
                var mines = 0;
                var blanks = 0;
                foreach (var block in m_BlockSets[i].Blocks)
                    switch (m_Manager[block])
                    {
                        case BlockStatus.Mine:
                            mines++;
                            break;
                        case BlockStatus.Blank:
                            blanks++;
                            break;
                    }
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
                m_BlockSets[i] =
                    new BlockSet<T>(m_BlockSets[i].Blocks.Where(b => m_Manager[b] == BlockStatus.Unknown));
                foreach (var restrain in m_Restrains)
                    if (restrain.BlockSets[i] != 0)
                        restrain.Mines -= mines;
            }

            return flag;
        }

        /// <summary>
        ///     生成矩阵
        /// </summary>
        /// <returns></returns>
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
        /// <param name="augmentedMatrix"></param>
        /// <returns>所有解</returns>
        private List<Solution<T>> EnumerateSolutions(IReadOnlyList<int> minors, double[,] augmentedMatrix)
        {
            var n = m_BlockSets.Count;
            var counts = m_BlockSets.Select(b => b.Count).ToArray();

            var res = new List<Solution<T>>();
            if (minors.Count == 0)
            {
                var lst = new List<int>(n);
                for (var i = 0; i < n; i++)
                    lst.Add((int)Math.Round(augmentedMatrix[n, i]));
                res.Add(new Solution<T>(lst));
            }
            else
            {
                var stack = new List<int>(minors.Count) { 0 };
                while (true)
                    if (stack.Count == minors.Count)
                        if (stack[stack.Count - 1] <= counts[minors[stack.Count - 1]])
                        {
                            var lst = new List<int>(n);
                            var minorID = 0;
                            var mainRow = 0;
                            for (var col = 0; col < n; col++)
                                if (minorID < minors.Count &&
                                    col == minors[minorID])
                                    lst.Add(stack[minorID++]);
                                else // major
                                {
                                    var sum = augmentedMatrix[n, mainRow];
                                    for (var i = 0; i < minors.Count; i++)
                                        sum -= augmentedMatrix[minors[i], mainRow] * stack[i];
                                    var val = (int)Math.Round(sum);
                                    if (val < 0 ||
                                        val > counts[col])
                                        break;
                                    lst.Add(val);
                                    mainRow++;
                                }
                            if (lst.Count == m_BlockSets.Count)
                                res.Add(new Solution<T>(lst));

                            stack[stack.Count - 1]++;
                        }
                        else
                        {
                            stack.RemoveAt(stack.Count - 1);
                            if (stack.Count == 0)
                                break;
                            stack[stack.Count - 1]++;
                        }
                    else if (stack[stack.Count - 1] <= counts[minors[stack.Count - 1]])
                        stack.Add(0);
                    else
                    {
                        stack.RemoveAt(stack.Count - 1);
                        if (stack.Count == 0)
                            break;
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
                    so.States *= m_BinomialHelper.Binomial(m_BlockSets[i].Count, so.Dist[i]);
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
                so.Ratio = BigIntegerHelper.Ratio(so.States, total);

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

                var ratio = BigIntegerHelper.Ratio(kvp.Value, total);
                Expectation.Add(kvp.Key, ratio);

                var p = ratio / kvp.Key.Count;
                foreach (var block in kvp.Key.Blocks)
                    Probability.Add(block, p);
            }

            return total;
        }
    }
}
