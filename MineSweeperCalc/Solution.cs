using System.Collections.Generic;
using System.Numerics;

namespace MineSweeperCalc
{
    /// <summary>
    ///     解
    /// </summary>
    /// <typeparam name="T">单个格的类型</typeparam>
    public class Solution<T>
        where T : IBlock<T>
    {
        internal Solution(List<int> dist) { Dist = dist; }

        /// <summary>
        ///     分布情况
        /// </summary>
        internal List<int> Dist { get; }

        /// <summary>
        ///     分布情况
        /// </summary>
        public IDictionary<BlockSet<T>, int> Distribution { get; internal set; }

        /// <summary>
        ///     对应微观状态数
        /// </summary>
        public BigInteger States { get; internal set; }

        /// <summary>
        ///     出现概率
        /// </summary>
        public double Ratio { get; internal set; }
    }
}
