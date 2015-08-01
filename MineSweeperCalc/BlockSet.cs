using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace MineSweeperCalc
{
    /// <summary>
    ///     格的集合
    /// </summary>
    /// <typeparam name="T">单个格的类型</typeparam>
    public struct BlockSet<T> : IReadOnlyCollection<T>, IEquatable<BlockSet<T>>
        where T : IBlock<T>
    {
        /// <summary>
        ///     各个格
        /// </summary>
        private readonly List<T> m_Blocks;

        public BlockSet(IEnumerable<T> blocks) { m_Blocks = new List<T>(blocks); }

        /// <inheritdoc />
        public int Count => m_Blocks.Count;

        /// <inheritdoc />
        public bool Equals(BlockSet<T> other) => !(other.Except(this).Any() || this.Except(other).Any());

        /// <inheritdoc />
        public override int GetHashCode() => m_Blocks.Aggregate(0x00000000, (a, b) => a ^ b.GetHashCode());

        /// <inheritdoc />
        public IEnumerator<T> GetEnumerator() => m_Blocks.GetEnumerator();

        /// <inheritdoc />
        IEnumerator IEnumerable.GetEnumerator() => m_Blocks.GetEnumerator();

        public override string ToString()
        {
            var sb = new StringBuilder();
            sb.Append("{");
            foreach (var block in m_Blocks)
            {
                sb.Append(block);
                sb.Append(",");
            }
            sb.Remove(sb.Length - 1, 1);
            sb.Append("}");
            return sb.ToString();
        }
    }
}
