using System.Collections.Generic;
using System.Numerics;

namespace MineSweeperCalc
{
    public class Solution<T>
        where T : IBlock<T>
    {
        public IDictionary<BlockSet<T>, int> Distribution { get; internal set; }

        public BigInteger States { get; internal set; }

        public double Ratio { get; internal set; }
    }
}
