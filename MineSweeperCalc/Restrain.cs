namespace MineSweeperCalc
{
    /// <summary>
    ///     区间
    /// </summary>
    public struct Interval
    {
        /// <summary>
        ///     下确界
        /// </summary>
        public int MinInclusive { get; set; }

        /// <summary>
        ///     上确界
        /// </summary>
        public int MaxInclusive { get; set; }
        
        /// <summary>
        ///     是否有解
        /// </summary>
        public bool Exist
            => MinInclusive <= MaxInclusive;

        /// <summary>
        ///     解是否唯一
        /// </summary>
        public bool Unique
            => MinInclusive == MaxInclusive;

        public Interval(int num)
        {
            MinInclusive = num;
            MaxInclusive = num;
        }

        public Interval(int min, int max)
        {
            MinInclusive = min;
            MaxInclusive = max;
        }

        public override string ToString() => $"{MinInclusive}-{MaxInclusive}";

        public static Interval operator -(Interval one, Interval other)
            => new Interval(one.MinInclusive - other.MaxInclusive, one.MaxInclusive - other.MinInclusive);
    }

    /// <summary>
    ///     约束
    /// </summary>
    public sealed class Restrain<T>
        where T : IBlock<T>
    {
        /// <summary>
        ///     区间
        /// </summary>
        public Interval Interval { get; set; }

        /// <summary>
        ///     下确界
        /// </summary>
        public int MinInclusive => Interval.MinInclusive;

        /// <summary>
        ///     上确界
        /// </summary>
        public int MaxInclusive => Interval.MaxInclusive;

        public BlockFamily<T> TheBlocks { get; }

        public Restrain(BlockFamily<T> family) { TheBlocks = family; }

        public Restrain(int num, BlockSet<T> set)
        {
            TheBlocks = new BlockFamily<T>(set);
            Interval = new Interval(num);
        }

        /// <summary>
        ///     是否有解
        /// </summary>
        public bool Exist
            => MinInclusive <= TheBlocks.CountBlock && 0 <= MaxInclusive;

        /// <summary>
        ///     解是否唯一
        /// </summary>
        public bool Unique
            => TheBlocks.CountBlock == MinInclusive || MaxInclusive == 0;

        public override string ToString() => $"{Interval} : {TheBlocks}";
    }
}
