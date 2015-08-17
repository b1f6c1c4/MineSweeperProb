using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using Node = MineSweeperCalc.Solver.OrthogonalList<double>.Node;

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
                            if (restrain.BlockSets.BinarySearch(i) >= 0)
                                restrain.BlockSets.Add(m_BlockSets.Count);
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
        private static void Overlap<TElement>(IReadOnlyList<TElement> setA, IReadOnlyList<TElement> setB,
                                              out List<TElement> exceptA, out List<TElement> exceptB,
                                              out List<TElement> intersection)
            where TElement : IComparable<TElement>
        {
            int p = 0, q = 0;
            intersection = new List<TElement>();
            exceptA = new List<TElement>();
            exceptB = new List<TElement>();
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
        /// <param name="withProb">求解概率</param>
        public void Solve(bool withProb)
        {
            Solutions = null;
            Probability = null;
            Expectation = null;
            TotalStates = BigInteger.MinusOne;

            m_DistCache.Clear();
            m_DistCacheLock.Clear();
            m_DistCondCache.Clear();
            m_DistCondCacheLock.Clear();

            var flag = true;
            while (flag)
            {
                while (ReduceRestrains()) { }

                flag = false;
                for (var i = 0; i < m_Restrains.Count - 1; i++)
                {
                    for (var j = i + 1; j < m_Restrains.Count; j++)
                        if (SimpleOverlap(m_Restrains[i], m_Restrains[j]))
                        {
                            flag = true;
                            break;
                        }
                    if (flag)
                        break;
                }
            }

            if (!withProb)
                return;

            if (m_BlockSets.Count == 0)
            {
                TotalStates = BigInteger.One;
                ProcessSolutions(new List<Solution<T>> { new Solution<T>(new List<int>()) });
                return;
            }

            var augmentedMatrix = GenerateMatrix();
            var minors = Gauss(augmentedMatrix);

            if (minors.Count > 0 &&
                minors[minors.Count - 1] == m_BlockSets.Count)
                minors.RemoveAt(minors.Count - 1);
            else
            {
                TotalStates = BigInteger.Zero;
                return;
            }

            var result = EnumerateSolutions(minors, augmentedMatrix);
            if (result.Count == 0)
            {
                TotalStates = BigInteger.Zero;
                return;
            }

            ProcessSolutions(result);
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
                else if (m_Restrains[i].Mines == m_Restrains[i].BlockSets.Select(index => cnts[index]).Sum())
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
        ///     两两化简约束
        /// </summary>
        /// <param name="first">约束</param>
        /// <param name="second">约束</param>
        /// <returns>是否成功化简</returns>
        private bool SimpleOverlap(Restrain first, Restrain second)
        {
            List<int> exceptA, exceptB, intersection;
            Overlap(first.BlockSets, second.BlockSets, out exceptA, out exceptB, out intersection);

            var sumA = exceptA.Select(index => m_BlockSets[index].Count).Sum();
            var sumB = exceptB.Select(index => m_BlockSets[index].Count).Sum();
            var sumC = intersection.Select(index => m_BlockSets[index].Count).Sum();

            var aL = Math.Max(0, first.Mines - sumC);
            var aU = Math.Min(sumA, first.Mines);
            var bL = Math.Max(0, second.Mines - sumC);
            var bU = Math.Min(sumB, second.Mines);
            var cL = 0;
            var cU = sumC;
            cL = Math.Max(cL, Math.Max(first.Mines - aU, second.Mines - bU));
            cU = Math.Min(cU, Math.Min(first.Mines - aL, second.Mines - bL));
            aL = Math.Max(aL, Math.Max(0, first.Mines - cU));
            aU = Math.Min(aU, Math.Min(sumA, first.Mines - cL));
            bL = Math.Max(bL, Math.Max(0, second.Mines - cU));
            bU = Math.Min(bU, Math.Min(sumB, second.Mines - cL));
            cL = Math.Max(cL, Math.Max(first.Mines - aU, second.Mines - bU));
            cU = Math.Min(cU, Math.Min(first.Mines - aL, second.Mines - bL));

            Func<List<int>, int, int, int, bool> process
                = (sets, sum, lb, ub) =>
                  {
                      if (sets.Count == 0)
                          return false;
                      if (sum == lb)
                          foreach (var block in sets.SelectMany(index => m_BlockSets[index].Blocks))
                              m_Manager.SetStatus(block, BlockStatus.Mine);
                      else if (ub == 0)
                          foreach (var block in sets.SelectMany(index => m_BlockSets[index].Blocks))
                              m_Manager.SetStatus(block, BlockStatus.Blank);
                      else
                          return false;
                      return true;
                  };

            var flag = false;
            flag |= process(exceptA, sumA, aL, aU);
            flag |= process(exceptB, sumB, bL, bU);
            flag |= process(intersection, sumC, cL, cU);

            return flag;
        }

        /// <summary>
        ///     生成矩阵
        /// </summary>
        /// <returns>增广矩阵</returns>
        private OrthogonalList<double> GenerateMatrix()
        {
            var m = m_Restrains.Count;
            var n = m_BlockSets.Count;

            var argMat = new OrthogonalList<double>(n + 1, m);
            for (var row = 0; row < m; row++)
            {
                var nr = argMat[row, -1];
                foreach (var index in m_Restrains[row].BlockSets)
                {
                    Node nc;
                    argMat.SeekDown(row, index, out nc);
                    nr = nc.Down = nr.Right = new Node(row, index, 1D) { Left = nr, Up = nc };
                }
                {
                    Node nc;
                    argMat.SeekDown(row, n, out nc);
                    nc.Down = nr.Right = new Node(row, n, m_Restrains[row].Mines) { Left = nr, Up = nc };
                }
            }
            return argMat;
        }

        /// <summary>
        ///     对矩阵进行高斯消元
        /// </summary>
        /// <param name="matrix">矩阵</param>
        /// <returns>非主元列</returns>
        private static List<int> Gauss(OrthogonalList<double> matrix)
        {
            var n = matrix.Width;
            var m = matrix.Height;
            var minorCol = new List<int>();
            var major = 0;
            for (var col = 0; col < n; col++)
            {
                var biasNode =
                    matrix.GetCol(col)
                          .Where(node => node.Row >= major)
                          .FirstOrDefault(node => !(Math.Abs(node.Val) < 1E-14));
                if (biasNode == null)
                {
                    minorCol.Add(col);
                    continue;
                }

                var vec = new double[m];
                var theBiasInv = 1 / biasNode.Val;
                vec[biasNode.Row] = theBiasInv;
                foreach (var node in matrix.GetCol(col))
                    if (node.Row != biasNode.Row)
                    {
                        vec[node.Row] = -node.Val * theBiasInv;
                        matrix.Remove(node);
                    }
                    else
                        node.Val = 1D;

                var bias = biasNode.Right;
                while (bias != null)
                {
                    var biasVal = bias.Val;
                    var nc = matrix[-1, bias.Col];
                    for (var row = 0; row < m; row++)
                    {
                        double oldV;
                        if (nc.Down == null ||
                            nc.Down.Row > row)
                            oldV = 0D;
                        else
                            oldV = nc.Down.Val;

                        var val = (row != biasNode.Row ? oldV : 0D) + vec[row] * biasVal;

                        if (nc.Down == null ||
                            nc.Down.Row > row)
                        {
                            if (Math.Abs(val) < 1E-14)
                                continue;

                            Node nr;
                            matrix.SeekRight(row, bias.Col, out nr);

                            var node = new Node(row, bias.Col, val)
                                           {
                                               Left = nr,
                                               Right = nr.Right,
                                               Up = nc,
                                               Down = nc.Down
                                           };
                            if (nr.Right != null)
                                nr.Right.Left = node;
                            if (nc.Down != null)
                                nc.Down.Up = node;
                            nc.Down = nr.Right = node;
                            nc = node;
                        }
                        else if (Math.Abs(val) < 1E-14)
                            matrix.Remove(nc.Down);
                        else
                        {
                            nc = nc.Down;
                            nc.Val = val;
                        }
                    }
                    bias = bias.Right;
                }

                if (major != biasNode.Row)
                {
                    var majNode = matrix[major, -1];
                    var biaNode = matrix[biasNode.Row, -1];
                    while (true)
                    {
                        var majID = majNode.Right?.Col ?? matrix.Width;
                        var biaID = biaNode.Right?.Col ?? matrix.Width;
                        // ReSharper disable PossibleNullReferenceException
                        if (majID < biaID)
                        {
                            biaNode = matrix.AddOrUpdate(biasNode.Row, majID, majNode.Right.Val, null);
                            matrix.Remove(majNode.Right);
                        }
                        else if (majID > biaID)
                        {
                            majNode = matrix.AddOrUpdate(major, biaID, biaNode.Right.Val, null);
                            matrix.Remove(biaNode.Right);
                        }
                        else if (majNode.Right != null)
                        {
                            majNode = majNode.Right;
                            biaNode = biaNode.Right;
                            var t = biaNode.Val;
                            biaNode.Val = majNode.Val;
                            majNode.Val = t;
                        }
                        else
                            break;
                        // ReSharper restore PossibialeNullReferenceException
                    }
                }
                major++;
            }
            return minorCol;
        }

        /// <summary>
        ///     遍历解空间
        /// </summary>
        /// <param name="minors">非主元列</param>
        /// <param name="augmentedMatrix">增广矩阵</param>
        /// <returns>所有解</returns>
        private List<Solution<T>> EnumerateSolutions(IReadOnlyList<int> minors, OrthogonalList<double> augmentedMatrix)
        {
            var n = m_BlockSets.Count;
            var counts = m_BlockSets.Select(b => b.Count).ToArray();

            var res = new List<Solution<T>>();
            if (minors.Count == 0)
            {
                var lst = new int[n];
                foreach (var node in augmentedMatrix.GetCol(n))
                    lst[node.Row] = (int)Math.Round(node.Val);
                res.Add(new Solution<T>(lst.ToList()));
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
                            sums[mainRow] = augmentedMatrix[mainRow, n]?.Val ?? 0;
                            mainRow++;
                        }
                }
                Action<int, int> aggr =
                    (minor, val) =>
                    {
                        foreach (var node in augmentedMatrix.GetCol(minors[minor]))
                            sums[node.Row] -= val * node.Val;
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
