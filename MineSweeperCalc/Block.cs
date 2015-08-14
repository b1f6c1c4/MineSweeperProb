using System;
using System.Linq;
using MineSweeperCalc.Solver;

namespace MineSweeperCalc
{
    /// <summary>
    ///     实际的格
    /// </summary>
    public sealed class Block : IBlock<Block>
    {
        /// <summary>
        ///     横坐标
        /// </summary>
        public int X { get; }

        /// <summary>
        ///     纵坐标
        /// </summary>
        public int Y { get; }

        /// <summary>
        ///     Hash缓存
        /// </summary>
        private readonly int m_Hash;

        /// <summary>
        ///     是否有雷
        /// </summary>
        private bool m_IsMine;

        /// <summary>
        ///     是否有雷，只能在翻开或游戏结束后查看
        /// </summary>
        public bool IsMine
        {
            get
            {
                if (!IsOpen &&
                    m_Mgr.Started)
                    throw new InvalidOperationException("此格尚未翻开，不能查看是否有雷");
                return m_IsMine;
            }
            internal set { m_IsMine = value; }
        }

        /// <summary>
        ///     是否有雷
        /// </summary>
        internal bool IsMineInternal() => m_IsMine;

        /// <summary>
        ///     是否翻开
        /// </summary>
        public bool IsOpen { get; internal set; }

        /// <summary>
        ///     周围的格
        /// </summary>
        public BlockSet<Block> Surrounding { get; internal set; }

        /// <summary>
        ///     周围的格中雷数，只能在翻开或游戏结束后查看
        /// </summary>
        public int Degree
        {
            get
            {
                if (!IsOpen &&
                    m_Mgr.Started)
                    throw new InvalidOperationException("此格尚未翻开，不能查看周围的格中雷数");
                return Surrounding.Cast<Block>().Count(block => block.IsMineInternal());
            }
        }

        /// <summary>
        ///     游戏
        /// </summary>
        private readonly GameMgr m_Mgr;

        internal Block(int x, int y, GameMgr mgr)
        {
            X = x;
            Y = y;
            m_Mgr = mgr;
            m_Hash = ((0x0000ffff & Hash(X)) << 16) + (0x0000ffff & Hash(Y));
        }

        /// <inheritdoc />
        public bool Equals(Block other) => m_Hash == other.m_Hash; //X == other.X && Y == other.Y;

        /// <inheritdoc />
        public override int GetHashCode() => m_Hash;

        /// <summary>
        ///     计算Hash
        /// </summary>
        /// <param name="v">坐标</param>
        /// <returns>Hash</returns>
        private static int Hash(int v)
        {
            var h = 5381;
            while (v > 0)
            {
                h = (h << 5) + h + (v % 2 + 50);
                v /= 2;
            }
            return h;
        }

        public int CompareTo(Block other)
        {
            if (X > other.X)
                return 1;
            if (X < other.X)
                return -1;
            if (Y > other.Y)
                return 1;
            if (Y < other.Y)
                return -1;
            return 0;
        }

        /// <inheritdoc />
        public override string ToString() => $"({X} {Y})";
    }
}
