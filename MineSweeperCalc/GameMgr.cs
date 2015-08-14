using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using MineSweeperCalc.Solver;

namespace MineSweeperCalc
{
    /// <summary>
    ///     半自动扫雷游戏
    /// </summary>
    public sealed class GameMgr
    {
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
        public bool Suceed { get; private set; }

        /// <summary>
        ///     待翻开格数
        /// </summary>
        public int ToOpen { get; private set; }

        /// <summary>
        ///     随机发生器种子
        /// </summary>
        private readonly int m_Seed;

        /// <summary>
        ///     求解器
        /// </summary>
        public Solver<Block> Solver { get; }

        public GameMgr(int width, int height, int totalMines, int seed)
        {
            m_Blocks = new Block[width, height];
            TotalWidth = width;
            TotalHeight = height;
            TotalMines = totalMines;
            m_Seed = seed;

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
            Suceed = false;
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
            var rnd = new Random(m_Seed);
            while (totalMines > 0)
            {
                var x = rnd.Next(TotalWidth);
                var y = rnd.Next(TotalHeight);
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

            Solver.AddRestrain(new BlockSet<Block>(new[] { block }), 0);
            if (block.Degree == 0)
                foreach (var b in block.Surrounding.Blocks.Where(b => !b.IsOpen))
                    OpenBlock(b.X, b.Y);
            else
                Solver.AddRestrain(block.Surrounding, block.Degree);

            if (--ToOpen == 0)
            {
                Started = false;
                Suceed = true;
            }
        }
    }
}
