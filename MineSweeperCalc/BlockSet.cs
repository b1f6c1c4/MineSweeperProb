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
    public struct BlockSet<T> : IEnumerable, IEquatable<BlockSet<T>>
        where T : IBlock<T>
    {
        /// <summary>
        ///     各个格
        /// </summary>
        public T[] Blocks { get; }

        private readonly int m_Hash;

        public BlockSet(T[] blocks)
        {
            Blocks = blocks;
            m_Hash = Blocks.Aggregate(0x00000000, (a, b) => a ^ b.GetHashCode());
        }

        public BlockSet(IEnumerable<T> blocks) : this(blocks.ToArray()) { }

        /// <inheritdoc />
        public int Count => Blocks.Length;

        public bool Any => Count > 0;

        /// <inheritdoc />
        public bool Equals(BlockSet<T> other)
        {
            if (Blocks.Length != other.Blocks.Length)
                return false;

            if (m_Hash != other.m_Hash)
                return false;


            for (var i = 0; i < Blocks.Length; i++)
            {
                int j;
                for (j = 0; j < other.Blocks.Length; j++)
                    if (Blocks[i].Equals(other.Blocks[j]))
                        break;
                if (j >= other.Blocks.Length)
                    return false;
            }

            return true;
        }

        /// <inheritdoc />
        public override int GetHashCode() => m_Hash;

        /// <inheritdoc />
        IEnumerator IEnumerable.GetEnumerator() => Blocks.GetEnumerator();

        public override string ToString()
        {
            var sb = new StringBuilder();
            sb.Append("{");
            foreach (var block in Blocks)
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
