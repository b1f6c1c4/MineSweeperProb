using System;
using System.Collections.Concurrent;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Threading;
using MineSweeperCalc;
using MineSweeperCalc.Solver;

namespace MineSweeperAnalyzer
{
    internal class Program
    {
        private static int m_Par;

        private static void Main(string[] args)
        {
            Console.WriteLine("Threads: ");
            m_Par = Convert.ToInt32(Console.ReadLine());
            Console.WriteLine("N: ");
            var n = Convert.ToInt32(Console.ReadLine());
            BinomialHelper.UpdateTo(30 * 16, 99);

            var seed = (int)DateTime.Now.Ticks;
            var stuff = new ConcurrentDictionary<GameMgr.DecideDelegate, int>
                            {
                                [Strategies.MinProb] = n,
                                [Strategies.MinProbMaxZeroProb] = n,
                                [Strategies.MinProbMaxZeroCount] = n,
                                [Strategies.MinProbMaxQuantity] = n
                            };
            var dic = Process(
                              stuff,
                              d =>
                              {
                                  var game = new GameMgr(30, 16, 99, Interlocked.Increment(ref seed), d);
                                  game.Automatic();
                                  game.Solver.Solve();
                                  return game.Solver.TotalStates.Log2();
                              });
            using (var sw = new StreamWriter(@"output.txt"))
                foreach (var kvp in dic.OrderBy(kvp => kvp.Key.Item2))
                    sw.WriteLine($"{kvp.Key.Item1.GetMethodInfo().Name}\t{kvp.Key.Item2:R}\t{kvp.Value}");
        }

        private static ConcurrentDictionary<Tuple<T, TResult>, int> Process<T, TResult>(
            ConcurrentDictionary<T, int> stuff, Func<T, TResult> action)
            where T : class
        {
            var dic = new ConcurrentDictionary<Tuple<T, TResult>, int>();
            var keys = stuff.Keys.ToArray();

            var ths = new Thread[m_Par];
            for (var i = 0; i < m_Par; i++)
            {
                ths[i] = new Thread(
                    () =>
                    {
                        while (true)
                        {
                            T flag = null;
                            foreach (var t in keys)
                            {
                                var val = -1;
                                stuff.AddOrUpdate(
                                                  t,
                                                  -1,
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
                            if (flag == null)
                                break;

                            dic.AddOrUpdate(new Tuple<T, TResult>(flag, action(flag)), 1, (key, val) => val + 1);
                        }
                    });
                ths[i].Start();
            }
            for (var i = 0; i < m_Par; i++)
                ths[i].Join();

            return dic;
        }
    }
}
