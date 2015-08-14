using System;
using System.Collections.Concurrent;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using MineSweeperCalc;
using MineSweeperCalc.Solver;

namespace MineSweeperAnalyzer
{
    internal class Program
    {
        private static int N;
        private static int Par;

        private static void Main(string[] args)
        {
            Console.WriteLine("Threads: ");
            Par = Convert.ToInt32(Console.ReadLine());
            Console.WriteLine("N: ");
            N = Convert.ToInt32(Console.ReadLine());
            BinomialHelper.UpdateTo(30 * 16, 99);

#if FALSE
            var dic = MultiProcess();
            using (var sw = new StreamWriter(@"output.txt"))
                foreach (var kvp in dic)
                    sw.WriteLine("{0}\t{1}\t{2:R}\t{3}", kvp.Key.Item1, kvp.Key.Item2, kvp.Key.Item3, kvp.Value);
#else
            var dic = Process(6, 6);
            using (var sw = new StreamWriter(@"output.txt"))
            foreach (var kvp in dic.OrderBy(kvp => kvp.Key))
                sw.WriteLine("{0:R}\t{1}", kvp.Key, kvp.Value);
#endif
        }

        private static ConcurrentDictionary<double, int> Process(int x, int y)
        {
            var dic = new ConcurrentDictionary<double, int>();
            var resume = N;
            var seed = (int)DateTime.Now.Ticks;

            var ths = new Thread[Par];
            for (var i = 0; i < Par; i++)
            {
                ths[i] = new Thread(() =>
                {
                    while (true)
                    {
                        if (Interlocked.Decrement(ref resume) < 0)
                            break;

                        var blockMgr = new GameMgr(30, 16, 99, Interlocked.Increment(ref seed));
                        var tester = new Tester(blockMgr, Interlocked.Increment(ref seed));
                        var bit = tester.Execute(x, y);
                        dic.AddOrUpdate(bit, 1, (key, val) => val + 1);
                    }
                });
                ths[i].Start();
            }
            for (var i = 0; i < Par; i++)
                ths[i].Join();

            return dic;
        }

        private static ConcurrentDictionary<Tuple<int,int, double>, int> MultiProcess()
        {
            var stuff = new ConcurrentDictionary<Tuple<int, int>, int>();
            for (var x = 0; x < 15; x++)
                for (var y = 0; y < 8; y++)
                    stuff[new Tuple<int, int>(x, y)] = N;

            var dic = new ConcurrentDictionary<Tuple<int, int, double>, int>();
            var seed = (int)DateTime.Now.Ticks;

            var ths = new Thread[Par];
            for (var i = 0; i < Par; i++)
            {
                ths[i] = new Thread(() =>
                {
                    while (true)
                    {
                        Tuple<int, int> flag = null;
                        for (var x = 0; x < 15; x++)
                        {
                            for (var y = 0; y < 8; y++)
                            {
                                var t = new Tuple<int, int>(x, y);
                                var val = -1;
                                stuff.AddOrUpdate(t, -1,
                                                  (tt, old) =>
                                                  {
                                                      val = old >= 1 ? old - 1 : -1;
                                                      return val;
                                                  });
                                if (val >= 0)
                                {
                                    flag = t;
                                    break;
                                }
                            }
                            if (flag != null)
                                break;
                        }
                        if (flag == null)
                            break;

                        var blockMgr = new GameMgr(30, 16, 99, Interlocked.Increment(ref seed));
                        var tester = new Tester(blockMgr, Interlocked.Increment(ref seed));
                        var bit = tester.Execute(flag.Item1,flag.Item2);
                        dic.AddOrUpdate(
                                        new Tuple<int, int, double>(flag.Item1, flag.Item2, bit),
                                        1,
                                        (key, val) => val + 1);
                    }
                });
                ths[i].Start();
            }
            for (var i = 0; i < Par; i++)
                ths[i].Join();

            return dic;
        }
    }
}
