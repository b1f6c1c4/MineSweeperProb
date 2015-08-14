using System.Collections;
using System.Collections.Generic;
using System.Linq;

namespace MineSweeperCalc.Solver
{
    /// <summary>
    ///     当前状态
    /// </summary>
    /// <typeparam name="T">单个格的类型</typeparam>
    internal sealed class BlockManager<T> : IReadOnlyDictionary<T, BlockStatus>
        where T : IBlock<T>
    {
        /// <summary>
        ///     各个格的状态
        /// </summary>
        private readonly Dictionary<T, BlockStatus> m_Dic;

        public BlockManager(IEnumerable<T> blocks)
        {
            m_Dic = blocks.ToDictionary(kvp => kvp, kvp => BlockStatus.Unknown);
        }

        public BlockManager(BlockManager<T> other) { m_Dic = new Dictionary<T, BlockStatus>(other.m_Dic); }

        /// <inheritdoc />
        public bool ContainsKey(T key) => m_Dic.ContainsKey(key);

        /// <inheritdoc />
        public bool TryGetValue(T key, out BlockStatus value) => m_Dic.TryGetValue(key, out value);

        /// <inheritdoc />
        public BlockStatus this[T key] => m_Dic[key];

        /// <inheritdoc />
        public IEnumerable<T> Keys => m_Dic.Keys;

        /// <inheritdoc />
        public IEnumerable<BlockStatus> Values => m_Dic.Values;

        /// <inheritdoc />
        public IEnumerator<KeyValuePair<T, BlockStatus>> GetEnumerator() => m_Dic.GetEnumerator();

        /// <inheritdoc />
        IEnumerator IEnumerable.GetEnumerator() => m_Dic.GetEnumerator();

        /// <inheritdoc />
        public int Count => m_Dic.Count;

        /// <summary>
        ///     设置格的状态
        /// </summary>
        /// <param name="key">格</param>
        /// <param name="value">状态</param>
        public void SetStatus(T key, BlockStatus value)
        {
            if (!m_Dic.ContainsKey(key))
                throw new KeyNotFoundException();
            m_Dic[key] = value;
        }
    }
}
