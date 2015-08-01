using System;

namespace MineSweeperCalc
{
    /// <summary>
    ///     格
    /// </summary>
    /// <typeparam name="T">自身的类型</typeparam>
    public interface IBlock<T> : IEquatable<T>
        where T : IBlock<T>
    {
        ///// <summary>
        /////     周围的格
        ///// </summary>
        //BlockSet<T> Surrounding { get; }
    }
}
