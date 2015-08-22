using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using MineSweeperCalc.Solver;

namespace MineSweeperCalc
{
    public class Drainer
    {
        private MicroSituation[] m_Micros;
        private List<MacroSituation> m_Macros;
        private List<Block> m_BlkDic;
        public Dictionary<Block, double> Prob { get; private set; }

        public static IEnumerable<IEnumerable<bool>> Combinations(int n, int m)
        {
            var count = 0;
            var stack = new List<bool>(n);
            Func<bool> add =
                () =>
                {
                    if (count < m)
                    {
                        if (stack.Count < n - 1)
                        {
                            var b = m - count >= n - stack.Count;
                            stack.Add(b);
                            if (b)
                                count++;
                            return false;
                        }
                        if (stack.Count == 0)
                            return true;
                        if (stack[stack.Count - 1])
                        {
                            while (stack.Count > 0 &&
                                   stack[stack.Count - 1])
                            {
                                stack.RemoveAt(stack.Count - 1);
                                count--;
                            }
                            if (stack.Count == 0)
                                return true;
                        }
                        stack[stack.Count - 1] = true;
                        count++;
                        return false;
                    }
                    if (stack.Count < n - 1)
                    {
                        stack.Add(false);
                        return false;
                    }
                    while (true)
                    {
                        while (stack.Count > 0 &&
                               stack[stack.Count - 1])
                        {
                            stack.RemoveAt(stack.Count - 1);
                            count--;
                        }
                        if (stack.Count == 0)
                            return true;
                        if (count < m)
                        {
                            stack[stack.Count - 1] = true;
                            count++;
                            return false;
                        }
                        while (stack.Count > 0 &&
                               !stack[stack.Count - 1])
                            stack.RemoveAt(stack.Count - 1);
                        if (stack.Count == 0)
                            return true;
                    }
                };
            while (true)
            {
                if (stack.Count < n - 1)
                {
                    if (add())
                        break;
                    continue;
                }
                Debug.Assert(count == m - 1 || count == m);
                stack.Add(count == m - 1);
                yield return stack;

                stack.RemoveAt(stack.Count - 1);
                if (add())
                    break;
            }
        }

        public IEnumerable<Block> Drain(GameMgr mgr)
        {
            m_BlkDic = mgr.CanOpenNotSureBlocks().ToList();
            m_BlkDic.Sort();

            GenerateMicros(mgr);

            var allBlockNotOpen = new int[m_BlkDic.Count];
            for (var i = 0; i < m_BlkDic.Count; i++)
                allBlockNotOpen[i] = -1;
            var solver = Solver<int>.ConvertFrom(mgr.Solver, b => m_BlkDic.BinarySearch(b), id=>id >= 0);
            var initialMacro = new MacroSituation(allBlockNotOpen, solver) { ToOpen = mgr.ToOpen };
            foreach (var micro in m_Micros)
                initialMacro.Micros.Add(micro);

            m_Macros = new List<MacroSituation>
                           {
                               initialMacro,
                               MacroSituation.Fail(),
                               MacroSituation.Succeed()
                           };
            m_Macros.Sort();

            foreach (var micro in m_Micros)
                SolveMicro(micro, initialMacro);

            foreach (var macro in m_Macros)
                foreach (var micro in macro.Micros)
                    if (micro.Transfer.ContainsKey(macro))
                        foreach (var ma in micro.Transfer[macro].Values)
                            if (ma.Dest.BinarySearch(macro) < 0)
                            {
                                ma.Dest.Add(macro);
                                ma.Dest.Sort();
                            }

            var initID = m_Macros.BinarySearch(initialMacro);
            if (initID != 0)
            {
                var t = m_Macros[0];
                m_Macros[0] = m_Macros[initID];
                m_Macros[initID] = t;
            }
            for (var i = 1; i < m_Macros.Count; i++)
                for (var j = i; j < m_Macros.Count; j++)
                {
                    var fflag = -1;
                    for (var id = 0; id < m_Macros[j].Dest.Count; id++)
                    {
                        var macro = m_Macros[j].Dest[id];
                        var flag = false;
                        for (var k = 0; k < i; k++)
                            if (macro.Equals(m_Macros[k]))
                            {
                                flag = true;
                                break;
                            }
                        if (flag)
                            continue;
                        fflag = id;
                        break;
                    }
                    if (fflag >= 0)
                        continue;
                    if (i == j)
                        break;
                    var t = m_Macros[i];
                    m_Macros[i] = m_Macros[j];
                    m_Macros[j] = t;
                    break;
                }

            Debug.Assert(initialMacro.Equals(m_Macros[0]));

            for (var i = m_Macros.Count - 1; i >= 0; i--)
                m_Macros[i].CollectProb();

            Prob = new Dictionary<Block, double>();
            for (var i = 0; i < m_BlkDic.Count; i++)
                Prob.Add(m_BlkDic[i], initialMacro.Probability[i]);

            return initialMacro.BestBlock.Select(id => m_BlkDic[id]);
        }

        private void GenerateMicros(GameMgr mgr)
        {
            m_Micros = new MicroSituation[(int)mgr.TotalStates];
            var nMicros = 0;

            var sets = mgr.Solver.Solutions.First().Distribution.Keys.ToList();
            var indexes = sets.Select(s => s.Blocks.Select(b => m_BlkDic.BinarySearch(b)).ToList()).ToList();
            var dicc = new List<IDictionary<int, List<IDictionary<int, bool>>>>();
            for (var i = 0; i < sets.Count; i++)
                dicc.Add(new Dictionary<int, List<IDictionary<int, bool>>>());
            foreach (var solution in mgr.Solver.Solutions)
            {
                var ddic = new List<List<IDictionary<int, bool>>>();
                for (var i = 0; i < sets.Count; i++)
                {
                    var m = solution.Distribution[sets[i]];
                    List<IDictionary<int, bool>> lst;
                    if (!dicc[i].TryGetValue(m, out lst))
                    {
                        lst = new List<IDictionary<int, bool>>();
                        foreach (var comb in Combinations(sets[i].Count, m))
                        {
                            var l = comb.ToList();
                            var d = new Dictionary<int, bool>();
                            for (var j = 0; j < sets[i].Count; j++)
                                d[indexes[i][j]] = l[j];
                            lst.Add(d);
                        }
                        dicc[i].Add(m, lst);
                    }
                    ddic.Add(lst);
                }

                var stack = new List<int>(sets.Count) { 0 };
                while (true)
                    if (stack.Count == sets.Count)
                        if (stack[stack.Count - 1] < ddic[stack.Count - 1].Count)
                        {
                            var lst = new bool[m_BlkDic.Count];
                            for (var i = 0; i < stack.Count; i++)
                                foreach (var kvp in ddic[i][stack[i]])
                                    lst[kvp.Key] = kvp.Value;
                            m_Micros[nMicros++] = new MicroSituation(lst);

                            stack[stack.Count - 1]++;
                        }
                        else
                        {
                            stack.RemoveAt(stack.Count - 1);
                            if (stack.Count == 0)
                                break;
                            stack[stack.Count - 1]++;
                        }
                    else if (stack[stack.Count - 1] < ddic[stack.Count - 1].Count)
                        stack.Add(0);
                    else
                    {
                        stack.RemoveAt(stack.Count - 1);
                        if (stack.Count == 0)
                            break;
                        stack[stack.Count - 1]++;
                    }
            }
        }

        private void SolveMicro(MicroSituation micro, MacroSituation macro)
        {
            for (var i = 0; i < macro.Degrees.Length; i++)
            {
                if (macro.Degrees[i] != -1 ||
                    macro.Solver[i] != BlockStatus.Unknown)
                    continue;

                var ma = SolveMicro(micro, macro, i);
                var id = m_Macros.BinarySearch(ma);
                if (id < 0)
                {
                    m_Macros.Add(ma);
                    m_Macros.Sort();
                }
                else
                    ma = m_Macros[id];

                IDictionary<int, MacroSituation> dic;
                if (!micro.Transfer.TryGetValue(macro, out dic))
                {
                    dic = new Dictionary<int, MacroSituation>();
                    micro.Transfer.Add(macro, dic);
                }
                if (!dic.ContainsKey(i))
                    dic.Add(i, ma);
                else
                    Debug.Assert(dic[i].Equals(ma));

                if (ma.Degrees == null)
                    continue;

                ma.Micros.Add(micro);
                SolveMicro(micro, ma);
            }
        }


        private MacroSituation SolveMicro(MicroSituation micro, MacroSituation macroOld, int blk)
        {
            if (micro.Dist[blk])
                return MacroSituation.Fail();

            var macro = new MacroSituation(macroOld.Degrees.ToArray(), new Solver<int>(macroOld.Solver))
                            { ToOpen = macroOld.ToOpen };

            OpenBlock(micro, macro, blk);
            if (macro.ToOpen == 0)
                return MacroSituation.Succeed();

            var flag = true;
            var withProb = false;
            while (flag || !withProb)
            {
                withProb = !flag;
                flag = false;
                macro.Solver.Solve(withProb);
                for (var i = 0; i < macro.Degrees.Length; i++)
                    if (macro.Degrees[i] == -1 &&
                        macro.Solver[i] == BlockStatus.Blank)
                    {
                        OpenBlock(micro, macro, i);
                        flag = true;
                    }

                if (macro.ToOpen == 0)
                    return MacroSituation.Succeed();
            }

            return macro;
        }

        private void OpenBlock(MicroSituation micro, MacroSituation macro, int blk)
        {
            macro.Solver.AddRestrain(new BlockSet<int>(blk), 0);
            var blks = m_BlkDic[blk].Surrounding.Blocks.Select(b => m_BlkDic.FindIndex(b.Equals)).Where(id => id > 0).ToList();
            var degree = blks.Count(id => micro.Dist[id]);
            macro.Degrees[blk] = degree;
            if (degree == 0)
                foreach (var id in blks.Where(id => macro.Degrees[id] == -1))
                    OpenBlock(micro, macro, id);
            else
                macro.Solver.AddRestrain(new BlockSet<int>(blks), degree);

            macro.ToOpen--;
        }
    }

    /// <summary>
    ///     微观状态
    /// </summary>
    internal class MicroSituation
    {
        public MicroSituation(bool[] dist)
        {
            Dist = dist;
            Transfer = new Dictionary<MacroSituation, IDictionary<int, MacroSituation>>();
        }

        /// <summary>
        ///     是否有雷
        /// </summary>
        public bool[] Dist { get; }

        /// <summary>
        ///     翻开某格导致的宏观状态
        /// </summary>
        public IDictionary<MacroSituation, IDictionary<int, MacroSituation>> Transfer { get; }

        public override string ToString()
        {
            var sb = new StringBuilder();
            foreach (var b in Dist)
                sb.Append(b ? "1" : "0");
            return sb.ToString();
        }
    }

    /// <summary>
    ///     宏观状态
    /// </summary>
    internal class MacroSituation : IEquatable<MacroSituation>, IComparable<MacroSituation>
    {
        public static MacroSituation Succeed() => new MacroSituation(1);

        public static MacroSituation Fail() => new MacroSituation(0);

        private MacroSituation(int prob) : this(null, null) { BestProb = prob; }

        public MacroSituation(int[] degrees, Solver<int> solver)
        {
            Degrees = degrees;
            Micros = new List<MicroSituation>();
            Solver = solver;
            Dest = new List<MacroSituation>();
        }

        /// <summary>
        ///     各格的周围雷数
        ///     若未翻开，则为<c>-1</c>
        /// </summary>
        public int[] Degrees { get; }

        /// <summary>
        ///     对应的所有微观状态
        /// </summary>
        public List<MicroSituation> Micros { get; }

        public Solver<int> Solver { get; }

        /// <summary>
        ///     对应的所有微观状态可能导致的宏观状态
        /// </summary>
        public List<MacroSituation> Dest { get; set; }

        /// <summary>
        ///     最大获胜概率
        /// </summary>
        public Rational BestProb { get; private set; }

        /// <summary>
        ///     各格最大获胜概率
        /// </summary>
        public Rational[] Probability { get; private set; }

        /// <summary>
        ///     最大获胜概率对应的块
        /// </summary>
        public List<int> BestBlock { get; private set; }

        public int ToOpen { get; set; }

        public void CollectProb()
        {
            if (Degrees == null)
                return;

            Probability = new Rational[Degrees.Length];
            BestProb = 0;
            for (var i = 0; i < Degrees.Length; i++)
            {
                if (Degrees[i] != -1 ||
                    Solver[i] != BlockStatus.Unknown)
                    continue;
                var prob = Micros.Aggregate((Rational)0, (c, m) => c + m.Transfer[this][i].BestProb) / Micros.Count;
                Probability[i] = prob;
                if (prob > BestProb)
                    BestProb = prob;
            }

            BestBlock = new List<int>();
            for (var i = 0; i < Degrees.Length; i++)
                if (Probability[i] >= BestProb)
                    BestBlock.Add(i);
        }

        public bool Equals(MacroSituation other)
        {
            if (GetHashCode() != other.GetHashCode())
                return false;

            if (Degrees == null ||
                other.Degrees == null)
                return Degrees == null &&
                       other.Degrees == null;

            return Degrees.SequenceEqual(other.Degrees);
        }

        public override int GetHashCode()
        {
            if (Degrees == null)
                // ReSharper disable once NonReadonlyMemberInGetHashCode
                return BestProb.IsZero ? 0x00000000 : 0x7fffffff;

            return Degrees.Aggregate(5381, (c, v) => (c << 5) + c + v);
        }

        public int CompareTo(MacroSituation other)
        {
            if (Degrees == null)
            {
                if (other.Degrees != null)
                    return 1;
                if (BestProb.IsZero)
                    return other.BestProb.IsZero ? 0 : -1;
                return !other.BestProb.IsZero ? 0 : 1;
            }
            if (other.Degrees == null)
                return -1;
            return Degrees.Select((t, i) => t.CompareTo(other.Degrees[i])).FirstOrDefault(cmp => cmp != 0);
        }

        public override string ToString()
        {
            if (Degrees == null)
                return BestProb.IsZero ? "Succeed" : "Fail";

            var sb = new StringBuilder();
            foreach (var b in Degrees)
                sb.Append($"{b} ");
            return sb.ToString();
        }
    }
}
