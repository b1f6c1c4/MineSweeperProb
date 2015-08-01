using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using MineSweeperCalc;

namespace MineSweeper
{
    internal sealed class BlockMgr
    {
        private readonly Block[,] m_Blocks;
        public int TotalWidth { get; }
        public int TotalHeight { get; }
        public int TotalMines { get; }
        private bool m_Settled;

        public BlockMgr(int width, int height, int totalMines)
        {
            m_Blocks = new Block[width, height];
            TotalWidth = width;
            TotalHeight = height;
            TotalMines = totalMines;

            var col = new Collection<Block>();

            for (var i = 0; i < width; i++)
                for (var j = 0; j < height; j++)
                {
                    var block = new Block(i, j);
                    col.Add(block);
                    m_Blocks[i, j] = block;
                }

            Blocks = col;

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
        }

        public IReadOnlyCollection<Block> Blocks { get; }

        private void SettleMines(int initX, int initY)
        {
            var totalMines = TotalMines;
            var rnd = new Random();
            while (totalMines > 0)
            {
                var x = rnd.Next(TotalWidth);
                var y = rnd.Next(TotalHeight);
                if (x == initX &&
                    y == initY)
                    continue;
                if (m_Blocks[x, y].IsMine)
                    continue;
                m_Blocks[x, y].IsMine = true;
                totalMines--;
            }
            m_Settled = true;
        }

        public Block this[int x, int y] => m_Blocks[x, y];

        public Block OpenBlock(int x, int y)
        {
            if (!m_Settled)
                SettleMines(x, y);
            m_Blocks[x, y].IsOpen = true;
            return m_Blocks[x, y];
        }
    }

    internal sealed class Block : IBlock<Block>
    {
        public int X { get; }

        public int Y { get; }

        public bool IsMine { get; internal set; }

        public bool IsOpen { get; internal set; }

        public BlockSet<Block> Surrounding { get; internal set; }

        public int Degree => Surrounding.Count(b => b.IsMine);

        public Block(int x, int y)
        {
            X = x;
            Y = y;
        }

        public bool Equals(Block other) => X == other.X && Y == other.Y;

        public override int GetHashCode() => X << 3 + Y;

        public override string ToString() => $"({X} {Y})";
    }
}
