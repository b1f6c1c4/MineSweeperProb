using System;
using System.Collections.Generic;
using System.Linq;

namespace MineSweeperCalc
{
    internal static class Manipulator
    {
        public static BlockSet<T> Intersect<T>(this BlockSet<T> one, BlockSet<T> another)
            where T : IBlock<T>
            => new BlockSet<T>(one.Blocks.Intersect(another.Blocks));

        public static BlockSet<T> Except<T>(this BlockSet<T> one, BlockSet<T> another)
            where T : IBlock<T>
            => new BlockSet<T>(one.Blocks.Except(another.Blocks));

        public static void Overlap<T>(BlockSet<T> set1, BlockSet<T> set2,
                                      out BlockSet<T> oSet1, out BlockSet<T> oSet2, out BlockSet<T> oSet0)
            where T : IBlock<T>
        {
            oSet0 = set1.Intersect(set2);
            oSet1 = set1.Except(set2);
            oSet2 = set2.Except(set1);
        }

        public static void Coordinate<T>(this BlockFamily<T> newOne, IEnumerable<BlockFamily<T>> oldOnes)
            where T : IBlock<T>
        {
            for (var j = 0; j < newOne.Count; j++)
                if (newOne[j].Count == 0)
                    newOne.RemoveAt(j--);
            foreach (var one in oldOnes)
            {
                for (var i = 0; i < one.Count; i++)
                {
                    for (var j = 0; j < newOne.Count; j++)
                    {
                        BlockSet<T> oSet, oSetN, oSetI;
                        Overlap(one[i], newOne[j], out oSet, out oSetN, out oSetI);

                        one.RemoveAt(i);
                        if (oSetI.Any)
                            one.Insert(i, oSetI);
                        if (oSet.Any)
                            one.Insert(i, oSet);

                        newOne.RemoveAt(j);
                        if (oSetI.Any)
                            newOne.Insert(j, oSetI);
                        if (oSetN.Any)
                            newOne.Insert(j, oSetN);

                        if (i >= one.Count)
                            break;
                    }
                }
            }
        }

        public static Interval Intersect(this Interval one, Interval another)
            => new Interval
                   {
                       MinInclusive = Math.Max(one.MinInclusive, another.MinInclusive),
                       MaxInclusive = Math.Min(one.MaxInclusive, another.MaxInclusive)
                   };
    }
}
