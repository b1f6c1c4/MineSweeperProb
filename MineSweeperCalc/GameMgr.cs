using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Numerics;
using MineSweeperCalc.Solver;

namespace MineSweeperCalc
{
    /// <summary>
    ///     自动扫雷游戏
    /// </summary>
    [Serializable]
    public class GameMgr
    {
        /// <summary>
        ///     进行决策
        /// </summary>
        /// <param name="blocks">格</param>
        /// <param name="mgr">游戏</param>
        /// <returns>最优格</returns>
        public delegate IEnumerable<Block> DecideDelegate(
            List<Block> blocks, GameMgr mgr, bool multiThread,
            IDictionary<Block, IDictionary<int, BigInteger>> dist = null);

        /// <summary>
        ///     格
        /// </summary>
        private readonly Block[,] m_Blocks;

        /// <summary>
        ///     宽度
        /// </summary>
        public int TotalWidth { get; }

        /// <summary>
        ///     高度
        /// </summary>
        public int TotalHeight { get; }

        /// <summary>
        ///     总雷数
        /// </summary>
        public int TotalMines { get; }

        /// <summary>
        ///     已布雷
        /// </summary>
        private bool m_Settled;

        /// <summary>
        ///     是否已开始
        /// </summary>
        public bool Started { get; private set; }

        /// <summary>
        ///     是否胜利
        /// </summary>
        public bool Succeed { get; private set; }

        /// <summary>
        ///     待翻开格数
        /// </summary>
        // ReSharper disable once MemberCanBePrivate.Global
        public int ToOpen { get; private set; }

        /// <summary>
        ///     求解器
        /// </summary>
        public Solver<Block> Solver { get; }

        /// <summary>
        ///     概率
        /// </summary>
        public virtual IDictionary<Block, double> Probability => Solver.Probability;

        /// <summary>
        ///     总状态数
        /// </summary>
        public virtual BigInteger TotalStates => Solver.TotalStates;

        /// <summary>
        ///     决策器
        /// </summary>
        // ReSharper disable once MemberCanBePrivate.Global
        public DecideDelegate DecisionMaker { get; set; }

        /// <summary>
        ///     随机数发生器
        /// </summary>
        private readonly Random m_Random;

        public GameMgr(int width, int height, int totalMines, int seed, DecideDelegate decisionMaker = null)
        {
            m_Blocks = new Block[width, height];
            TotalWidth = width;
            TotalHeight = height;
            TotalMines = totalMines;
            m_Random = new Random(seed);
            DecisionMaker = decisionMaker;

            var set = new Collection<Block>();

            for (var i = 0; i < width; i++)
                for (var j = 0; j < height; j++)
                {
                    var block = new Block(i, j, this);
                    set.Add(block);
                    m_Blocks[i, j] = block;
                }

            for (var i = 0; i < width; i++)
                for (var j = 0; j < height; j++)
                {
                    var lst = new List<Block>(8);
                    for (var di = -1; di <= 1; di++)
                        if (i + di >= 0 &&
                            i + di < width)
                            for (var dj = -1; dj <= 1; dj++)
                                if (j + dj >= 0 &&
                                    j + dj < height)
                                    if (di != 0 ||
                                        dj != 0)
                                        lst.Add(m_Blocks[i + di, j + dj]);
                    m_Blocks[i, j].Surrounding = new BlockSet<Block>(lst);
                }

            Solver = new Solver<Block>(set);
            Solver.AddRestrain(new BlockSet<Block>(set), TotalMines);

            Started = true;
            Succeed = false;
            ToOpen = TotalWidth * TotalHeight - TotalMines;
        }

        /// <summary>
        ///     避开某个格布雷
        /// </summary>
        /// <param name="initX">避开的格的横坐标</param>
        /// <param name="initY">避开的格的纵坐标</param>
        private void SettleMines(int initX, int initY)
        {
            var totalMines = TotalMines;
            while (totalMines > 0)
            {
                var x = m_Random.Next(TotalWidth);
                var y = m_Random.Next(TotalHeight);
                if (x == initX &&
                    y == initY)
                    continue;
                if (m_Blocks[x, y].IsMineInternal())
                    continue;
                m_Blocks[x, y].IsMine = true;
                totalMines--;
            }
            m_Settled = true;
        }

        /// <summary>
        ///     获取某一位置的格
        /// </summary>
        /// <param name="x">横坐标</param>
        /// <param name="y">纵坐标</param>
        /// <returns>格</returns>
        public Block this[int x, int y] => m_Blocks[x, y];

        /// <summary>
        ///     翻开某一位置，如翻出0则自动翻开周边位置
        /// </summary>
        /// <param name="x">横坐标</param>
        /// <param name="y">纵坐标</param>
        public void OpenBlock(int x, int y)
        {
            if (!Started)
                throw new InvalidOperationException("游戏已结束");
            if (!m_Settled)
                SettleMines(x, y);

            var block = m_Blocks[x, y];
            if (block.IsOpen)
                throw new InvalidOperationException("此格已翻开");
            block.IsOpen = true;

            if (block.IsMine)
            {
                Started = false;
                return;
            }

            Solver.AddRestrain(new BlockSet<Block>(block), 0);
            if (block.Degree == 0)
                foreach (var b in block.Surrounding.Blocks.Where(b => !b.IsOpen))
                    OpenBlock(b.X, b.Y);
            else
                Solver.AddRestrain(block.Surrounding, block.Degree);

            if (--ToOpen == 0)
            {
                Started = false;
                Succeed = true;
            }
        }

        /// <summary>
        ///     获取确定无雷的可翻开的格
        /// </summary>
        /// <returns>确定无雷的可翻开的格</returns>
        public IEnumerable<Block> CanOpenForSureBlocks()
        {
            for (var i = 0; i < TotalWidth; i++)
                for (var j = 0; j < TotalHeight; j++)
                    if (!m_Blocks[i, j].IsOpen &&
                        Solver[m_Blocks[i, j]] == BlockStatus.Blank)
                        yield return m_Blocks[i, j];
        }

        /// <summary>
        ///     获取不一定无雷的可翻开的格
        /// </summary>
        /// <returns>不一定无雷的可翻开的格</returns>
        public IEnumerable<Block> CanOpenNotSureBlocks()
        {
            for (var i = 0; i < TotalWidth; i++)
                for (var j = 0; j < TotalHeight; j++)
                    if (!m_Blocks[i, j].IsOpen &&
                        Solver[m_Blocks[i, j]] == BlockStatus.Unknown)
                        yield return m_Blocks[i, j];
        }

        /// <summary>
        ///     半自动操作一步
        /// </summary>
        /// <param name="withProb">求解概率</param>
        /// <returns>可以继续半自动操作</returns>
        public bool SemiAutomaticStep(bool withProb)
        {
            if (!Started)
                return false;
            Solver.Solve(withProb);
            var flag = false;
            foreach (var block in CanOpenForSureBlocks())
            {
                OpenBlock(block.X, block.Y);
                flag = true;
                if (Started)
                    continue;
                if (!Succeed)
                    throw new ApplicationException("判断错误");
                break;
            }
            return flag && Started;
        }

        /// <summary>
        ///     半自动操作
        /// </summary>
        /// <returns>需要进行决策</returns>
        public virtual bool SemiAutomatic()
        {
            if (!Started)
                return false;
            while (true)
            {
                while (SemiAutomaticStep(false)) { }
                if (!SemiAutomaticStep(true))
                    break;
            }
            return Started;
        }

        /// <summary>
        ///     按特定策略自动操作一次
        /// </summary>
        /// <param name="multiThread"></param>
        public virtual void AutomaticStep(bool multiThread)
        {
            if (!Started)
                return;

            if (DecisionMaker == null)
            {
                Started = false;
                return;
            }

            var ary = DecisionMaker(CanOpenNotSureBlocks().ToList(), this, multiThread).ToArray();
            var blk = ary[m_Random.Next(ary.Length)];
            OpenBlock(blk.X, blk.Y);
        }

        /// <summary>
        ///     按特定策略自动操作
        /// </summary>
        /// <param name="multiThread"></param>
        public virtual void Automatic(bool multiThread)
        {
            while (Started)
                if (SemiAutomatic())
                    AutomaticStep(multiThread);
        }
    }
}
