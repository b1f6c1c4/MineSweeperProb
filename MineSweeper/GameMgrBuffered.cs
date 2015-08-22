using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Runtime.Serialization;
using System.Threading;
using System.Threading.Tasks;
using MineSweeperCalc;
using MineSweeperCalc.Solver;

namespace MineSweeper
{
    /// <summary>
    ///     模式
    /// </summary>
    [Flags]
    public enum SolvingMode
    {
        /// <summary>
        ///     不自动求解
        /// </summary>
        None = 0x0,

        /// <summary>
        ///     自动半程求解
        /// </summary>
        Half = 0x1,

        /// <summary>
        ///     自动概率求解
        /// </summary>
        Probability = 0x2 | Half,

        /// <summary>
        ///     自动决策求解
        /// </summary>
        Automatic = 0x4 | Probability,

        /// <summary>
        ///     自动扩展求解
        /// </summary>
        Extended = 0x8 | Automatic
    }

    [Serializable]
    public sealed class GameMgrBuffered : GameMgr
    {
        /// <summary>
        ///     模式
        /// </summary>
        private SolvingMode m_Mode;

        /// <summary>
        ///     模式
        /// </summary>
        public SolvingMode Mode
        {
            get { return m_Mode; }
            set
            {
                if (m_Mode == value)
                    return;

                Cancel();
                m_Mode = value;
                if (!m_Mode.HasFlag(SolvingMode.Extended))
                {
                    DegreeDist = null;
                    Quantity = null;
                }
                if (!m_Mode.HasFlag(SolvingMode.Automatic))
                    Bests = CanOpenForSureBlocks().ToList();
                if (!m_Mode.HasFlag(SolvingMode.Probability))
                {
                    m_Probability = null;
                    m_TotalStates = BigInteger.MinusOne;
                }
                if (!m_Mode.HasFlag(SolvingMode.Half))
                {
                    Bests = null;
                    InferredStatuses = null;
                }
            }
        }

        /// <summary>
        ///     是否正在求解
        /// </summary>
        public bool Solving { get; private set; }

        /// <summary>
        ///     求解任务
        /// </summary>
        [NonSerialized]
        private Task m_Backgrounding;

        /// <summary>
        ///     概率
        /// </summary>
        private IDictionary<Block, double> m_Probability;

        /// <summary>
        ///     概率
        /// </summary>
        public override IDictionary<Block, double> Probability => m_Probability;

        /// <summary>
        ///     总状态数
        /// </summary>
        private BigInteger m_TotalStates;

        /// <summary>
        ///     总状态数
        /// </summary>
        public override BigInteger TotalStates => m_TotalStates;

        /// <summary>
        ///     确定最佳格
        /// </summary>
        public List<Block> BestsForSure { get; private set; }

        /// <summary>
        ///     最佳格
        /// </summary>
        public List<Block> Bests { get; set; }

        /// <summary>
        ///     概率
        /// </summary>
        public IDictionary<Block, double> DrainProbability;

        /// <summary>
        ///     周围雷数分布
        /// </summary>
        public IDictionary<Block, IDictionary<int, double>> DegreeDist { get; private set; }

        /// <summary>
        ///     信息量
        /// </summary>
        public Dictionary<Block, double> Quantity { get; private set; }

        /// <summary>
        ///     推测的状态
        /// </summary>
        public Dictionary<Block, BlockStatus> InferredStatuses { get; private set; }

        /// <summary>
        ///     锁
        /// </summary>
        [NonSerialized]
        private ReaderWriterLockSlim m_Lock = new ReaderWriterLockSlim();

        [OnDeserialized]
        private void SetValuesOnDeserialized(StreamingContext context) => m_Lock = new ReaderWriterLockSlim();

        public void EnterReadLock() => m_Lock.EnterReadLock();
        public void ExitReadLock() => m_Lock.ExitReadLock();

        public GameMgrBuffered(int width, int height, int totalMines, int seed, DecideDelegate decisionMaker = null)
            : base(width, height, totalMines, seed, decisionMaker)
        {
            m_TotalStates = BigInteger.MinusOne;
        }

        /// <summary>
        ///     启动求解
        /// </summary>
        /// <returns>后台求解任务</returns>
        public async Task Solve()
        {
            if (Solving)
                throw new InvalidOperationException("上一次求解未完成");

            if (!Started)
                return;

            if (!Mode.HasFlag(SolvingMode.Half))
                return;

            m_Backgrounding = Task.Run(() => Process());

            await m_Backgrounding;
        }

        /// <summary>
        ///     求解
        /// </summary>
        private void Process()
        {
            Solving = true;

            Solver.Solve(Mode.HasFlag(SolvingMode.Probability));

            var bests = CanOpenForSureBlocks().ToList();
            List<Block> bestsN = null;
            Dictionary<Block, IDictionary<int, double>> degreeDist = null;
            Dictionary<Block, double> quantity = null;

            if (!Mode.HasFlag(SolvingMode.Probability))
                goto saveResult;

            if (!Mode.HasFlag(SolvingMode.Automatic))
            {
                if (bests.Count == 0)
                    bestsN = Strategies.MinProb(CanOpenNotSureBlocks().ToList(), this, false).ToList();
                goto saveResult;
            }

            if (!Mode.HasFlag(SolvingMode.Extended))
            {
                if (bests.Count == 0)
                {
                    var lst = CanOpenNotSureBlocks().ToList();
                    bestsN = (DecisionMaker?.Invoke(lst, this, false) ?? Strategies.MinProb(lst, this, false)).ToList();
                }
                goto saveResult;
            }

            var dist = new Dictionary<Block, IDictionary<int, BigInteger>>();
            degreeDist = new Dictionary<Block, IDictionary<int, double>>();
            quantity = new Dictionary<Block, double>();
            var q0 = Solver.TotalStates.Log2();
            foreach (var block in CanOpenNotSureBlocks())
            {
                var dic = Solver.DistributionCond(block.Surrounding, new BlockSet<Block>(block), 0);
                dist[block] = dic;
                var total = dic.Aggregate(BigInteger.Zero, (cur, kvp) => cur + kvp.Value);
                var pDic = total.IsZero
                               ? new Dictionary<int, double>()
                               : dic.ToDictionary(kvp => kvp.Key, kvp => kvp.Value.Over(total));
                degreeDist[block] = pDic;
                var qu = dic.Sum(
                                 kvp =>
                                 {
                                     if (kvp.Value == 0)
                                         return 0D;
                                     var p = pDic[kvp.Key];
                                     var q = q0 - kvp.Value.Log2();
                                     return p * q;
                                 });
                quantity[block] = qu;
            }
            if (bests.Count == 0)
            {
                var lst = CanOpenNotSureBlocks().ToList();
                bestsN =
                    (DecisionMaker?.Invoke(lst, this, false, dist) ?? Strategies.MinProb(lst, this, false, dist)).ToList
                        ();
            }

            saveResult:
            bests.Sort();
            m_Lock.EnterWriteLock();
            try
            {
                if (Mode.HasFlag(SolvingMode.Extended))
                {
                    Quantity = quantity;
                    DegreeDist = degreeDist;
                }

                if (Mode.HasFlag(SolvingMode.Probability))
                {
                    m_Probability = Solver.Probability;
                    m_TotalStates = Solver.TotalStates;
                }

                BestsForSure = bests;
                Bests = bestsN;
                InferredStatuses = new Dictionary<Block, BlockStatus>();
                for (var i = 0; i < TotalWidth; i++)
                    for (var j = 0; j < TotalHeight; j++)
                        InferredStatuses[this[i, j]] = Solver[this[i, j]];
                Solving = false;
            }
            finally
            {
                m_Lock.ExitWriteLock();
            }
        }

        /// <summary>
        ///     取消后台求解任务
        /// </summary>
        public void Cancel()
        {
            if (!Solving)
                return;

            if (m_Backgrounding == null)
                return;

            m_Backgrounding.Dispose();
            m_Backgrounding = null;

            Solving = false;
        }

        /// <summary>
        ///     半自动操作一步
        /// </summary>
        /// <returns>可以继续半自动操作</returns>
        public bool SemiAutomaticStep() => SemiAutomaticStep(Mode.HasFlag(SolvingMode.Probability));

        /// <inheritdoc />
        public override bool SemiAutomatic()
        {
            if (!Started)
                return false;
            while (true)
            {
                while (SemiAutomaticStep(false)) { }
                if (!Mode.HasFlag(SolvingMode.Probability) ||
                    !SemiAutomaticStep())
                    break;
            }
            return Started;
        }

        /// <inheritdoc />
        public override void Automatic(bool multiThread, double threshold = 0D)
        {
            if (Mode.HasFlag(SolvingMode.Automatic))
                base.Automatic(multiThread, threshold);
            else
                SemiAutomatic();
        }
    }
}
