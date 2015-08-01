using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace MineSweeperCalc
{
    /// <summary>
    ///     格的无交集合的族
    /// </summary>
    /// <typeparam name="T">单个格的类型</typeparam>
    public sealed class BlockFamily<T> : IList<BlockSet<T>>, IEquatable<BlockFamily<T>>
        where T : IBlock<T>
    {
        /// <summary>
        ///     各个格的集合
        /// </summary>
        private readonly List<BlockSet<T>> m_BlockSets;

        public BlockFamily(BlockSet<T> sets) { m_BlockSets = new List<BlockSet<T>> { sets }; }

        public BlockFamily(IEnumerable<BlockSet<T>> sets)
        {
            m_BlockSets = sets as List<BlockSet<T>> ?? sets.ToList();
            var blocks = Blocks.ToList();
            if (blocks.Distinct().Count() < blocks.Count)
                throw new ArgumentException("集合之交非空", nameof(sets));
        }

        /// <summary>
        ///     各个格
        /// </summary>
        public IEnumerable<T> Blocks => m_BlockSets.SelectMany(set => set.Blocks);

        /// <inheritdoc />
        public void Add(BlockSet<T> item) => m_BlockSets.Add(item);

        /// <inheritdoc />
        public void Clear() => m_BlockSets.Clear();

        /// <inheritdoc />
        public bool Contains(BlockSet<T> item) => m_BlockSets.Contains(item);

        /// <inheritdoc />
        public void CopyTo(BlockSet<T>[] array, int arrayIndex) => m_BlockSets.CopyTo(array, arrayIndex);

        /// <inheritdoc />
        public bool Remove(BlockSet<T> item) => m_BlockSets.Remove(item);

        /// <inheritdoc />
        public int Count => m_BlockSets.Count;

        /// <inheritdoc />
        public bool IsReadOnly => false;

        /// <summary>
        ///     格的个数
        /// </summary>
        public int CountBlock => Blocks.Count();

        /// <inheritdoc />
        public bool Equals(BlockFamily<T> other) => !(other.Except(this).Any() || this.Except(other).Any());

        /// <inheritdoc />
        public IEnumerator<BlockSet<T>> GetEnumerator() => m_BlockSets.GetEnumerator();

        /// <inheritdoc />
        IEnumerator IEnumerable.GetEnumerator() => m_BlockSets.GetEnumerator();

        /// <inheritdoc />
        public int IndexOf(BlockSet<T> item) => m_BlockSets.IndexOf(item);

        /// <inheritdoc />
        public void Insert(int index, BlockSet<T> item) => m_BlockSets.Insert(index, item);

        /// <inheritdoc />
        public void RemoveAt(int index) => m_BlockSets.RemoveAt(index);

        /// <inheritdoc />
        public BlockSet<T> this[int index] { get { return m_BlockSets[index]; } set { m_BlockSets[index] = value; } }

        public override string ToString()
        {
            var sb = new StringBuilder();
            sb.Append("{");
            foreach (var set in m_BlockSets)
            {
                sb.Append(set);
                sb.Append(", ");
            }
            sb.Append("}");
            return sb.ToString();
        }
    }
}
