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
            var stuff =
                new ConcurrentDictionary<GameMgr.DecideDelegate, int>
                    {
                        [Strategies.MinProb] = n,
                        [Strategies.MinProbMaxZeroProb] = n,
                        [Strategies.MinProbMaxZeroCount] = n,
                        [Strategies.MinProbMaxQuantity] = n,
                        [Strategies.MinProbMinQuantity] = n,
                        [Strategies.MinProbMaxQuantityMaxZeroCount] = n,
                        [Strategies.MinProbMaxQuantityMaxZeroProb] = n,
                        [Strategies.MinProbMinQuantityMaxZeroCount] = n,
                        [Strategies.MinProbMinQuantityMaxZeroProb] = n,
                        [Strategies.MinProbMaxZeroCountMaxQuantity] = n,
                        [Strategies.MinProbMaxZeroCountMinQuantity] = n,
                        [Strategies.MinProbMaxZeroProbMaxQuantity] = n,
                        [Strategies.MinProbMaxZeroProbMinQuantity] = n,
                        [Strategies.MixProbZeroProb] = n
                    };
            Process(
                    stuff,
                    d =>
                    {
                        var game = new GameMgr(30, 16, 99, Interlocked.Increment(ref seed), d);
                        game.Automatic();
                        game.Solver.Solve();
                        return game.Solver.TotalStates.Log2();
                    },
                    dic =>
                    {
                        using (var sw = new StreamWriter(@"output.txt"))
                            foreach (var kvp in dic.OrderBy(kvp => kvp.Key.Item2))
                                sw.WriteLine($"{kvp.Key.Item1.GetMethodInfo().Name}\t{kvp.Key.Item2:R}\t{kvp.Value}");
                    });
        }

        private static ConcurrentDictionary<Tuple<T, TResult>, int> Process<T, TResult>(
            ConcurrentDictionary<T, int> stuff, Func<T, TResult> action, Action<ConcurrentDictionary<Tuple<T, TResult>, int>> damp)
            where T : class
        {
            var dic = new ConcurrentDictionary<Tuple<T, TResult>, int>();
            var keys = stuff.Keys.ToArray();

            var sum = stuff.Values.Sum();
            var todo = sum;
            var doing = 0L;
            var done = 0L;
            var ths = new Thread[m_Par];
            var locks = new object[m_Par];
            var timeouts = new long[m_Par];
            Action<int> proc =
                index =>
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
                                Interlocked.Decrement(ref todo);
                                break;
                            }
                        }
                        if (flag == null)
                            break;

                        lock (locks[index])
                            timeouts[index] = 100;
                        Interlocked.Increment(ref doing);
                        var result = action(flag);
                        dic.AddOrUpdate(new Tuple<T, TResult>(flag, result), 1, (key, val) => val + 1);
                        Interlocked.Decrement(ref doing);
                        Interlocked.Increment(ref done);
                    }
                    lock (locks[index])
                        timeouts[index] = 0;
                };
            for (var i = 0; i < m_Par; i++)
            {
                var index = i;
                locks[index] = new object();
                lock (locks[index])
                    timeouts[index] = 100;
                ths[index] = new Thread(() => proc(index));
                ths[index].Start();
            }
            var elap = 0;
            while (true)
            {
                Console.Write($"{todo}-{doing}-{done} ");

                var count = 0;
                for (var i = 0; i < m_Par; i++)
                {
                    lock (locks[i])
                    {
                        timeouts[i] -= 1;
                        Console.Write($"{i}:{timeouts[i]} ");
                        if (timeouts[i] > 0)
                            continue;
                        timeouts[i] = 100;
                    }
                    var index = i;
                    if (ths[index].ThreadState == ThreadState.Stopped)
                        count++;
                    ths[index].Abort();
                    ths[index] = new Thread(() => proc(index));
                    ths[index].Start();
                }
                if (count == m_Par)
                    break;

                if (done > 0)
                {
                    var s = (int)(elap * 1D / done * (todo + doing));
                    Console.WriteLine($"{s / 60 / 60}:{(s / 60) % 60}:{s % 60} {DateTime.Now.AddSeconds(s):HH:mm:ss}");
                }
                else
                    Console.WriteLine();

                if (++elap % 60 == 0)
                    damp(dic);
                Thread.Sleep(1000);
            }
            damp(dic);

            return dic;
        }
    }
}
