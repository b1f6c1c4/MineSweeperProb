using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace MineSweeperCalc.Solver
{
    /// <summary>
    ///     格的集合
    /// </summary>
    /// <typeparam name="T">单个格的类型</typeparam>
    public class BlockSet<T> : IEnumerable, IEquatable<BlockSet<T>>
        where T : IBlock<T>
    {
        /// <summary>
        ///     各个格
        /// </summary>
        public T[] Blocks { get; }

        private readonly int m_Hash;

        public BlockSet(T[] blocks)
        {
            var flag = true;
            for (var i = 0; i < blocks.Length - 1; i++)
                if (blocks[i].CompareTo(blocks[i + 1]) > 0)
                {
                    flag = false;
                    break;
                }
            if (!flag)
            {
                var lst = blocks.ToList();
                lst.Sort();
                Blocks = lst.ToArray();
            }
            else
                Blocks = blocks;

            m_Hash = Blocks.Aggregate(5381, (h, t) => (h << 5) + h + t.GetHashCode());
        }

        public BlockSet(IEnumerable<T> blocks) : this(blocks.ToArray()) { }

        public BlockSet(T block) : this(new[] { block }) { }

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

            return !Blocks.Where((t, i) => !t.Equals(other.Blocks[i])).Any();
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

        public static BlockSet<T> Empty { get; }

        static BlockSet() { Empty = new BlockSet<T>(Enumerable.Empty<T>()); }
    }
}
