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
            var seed = (int)DateTime.Now.Ticks;
#if FALSE
            var stuff = new ConcurrentDictionary<GameMgr.DecideDelegate, int>();
            using (var sr = new StreamReader(@"config.txt"))
            {
                m_Par = Convert.ToInt32(sr.ReadLine());
                var n = Convert.ToInt32(sr.ReadLine());
                while (!sr.EndOfStream)
                {
                    var s = sr.ReadLine();
                    if (s == null)
                        break;
                    var del = typeof(Strategies).GetMethod(s).CreateDelegate(typeof(GameMgr.DecideDelegate))
                              as GameMgr.DecideDelegate;
                    if (del != null)
                        stuff[del] = n;
                }
            }
            BinomialHelper.UpdateTo(30 * 16, 99);
            using (var sw = new StreamWriter(@"output.txt", true))
                Process(
                    stuff,
                    d =>
                    {
                        var game = new GameMgr(30, 16, 99, Interlocked.Increment(ref seed), d);
                        game.Automatic(true);
                        game.Solver.Solve(true);
                        return game.Solver.TotalStates.Log2();
                    },
                    dic =>
                    {
                            foreach (var kvp in dic.OrderBy(kvp => kvp.Key.Item2))
                                sw.WriteLine($"{kvp.Key.Item1.GetMethodInfo().Name}\t{kvp.Key.Item2:R}\t{kvp.Value}");
                            sw.Flush();
                    });
#elif FALSE
            var stuff = new ConcurrentDictionary<int, int>();
            using (var sr = new StreamReader(@"config.txt"))
            {
                m_Par = Convert.ToInt32(sr.ReadLine());
                var n = Convert.ToInt32(sr.ReadLine());
                var m = Convert.ToInt32(sr.ReadLine());
                for (var i = 1; i <= m; i++)
                    stuff[i] = n;
            }
            BinomialHelper.UpdateTo(30 * 16, 200);
            using (var sw = new StreamWriter(@"output.txt", true))
                Process(
                    stuff,
                    d =>
                    {
                        var game = new GameMgr(30, 16, d, Interlocked.Increment(ref seed), Strategies.MinProbMaxZeroProbMaxQuantity);
                        game.Automatic(true);
                        game.Solver.Solve(true);
                        return game.Solver.TotalStates.Log2();
                    },
                    dic =>
                    {
                        foreach (var kvp in dic.OrderBy(kvp => kvp.Key.Item2))
                            sw.WriteLine($"{kvp.Key.Item1}\t{kvp.Key.Item2:R}\t{kvp.Value}");
                        sw.Flush();
                    });
#elif TRUE
#endif
            var stuff = new ConcurrentDictionary<double, int>();
            using (var sr = new StreamReader(@"config.txt"))
            {
                m_Par = Convert.ToInt32(sr.ReadLine());
                var n = Convert.ToInt32(sr.ReadLine());
                while (!sr.EndOfStream)
                {
                    var s = sr.ReadLine();
                    if (s == null)
                        break;
                    stuff[Convert.ToDouble(s)] = n;
                }
            }
            BinomialHelper.UpdateTo(30 * 16, 99);
            using (var sw = new StreamWriter(@"output.txt", true))
                Process(
                    stuff,
                    d =>
                    {
                        var game = new GameMgr(30, 16, 99, Interlocked.Increment(ref seed), Strategies.MinProbMaxZeroProbMaxQuantity);
                        game.Automatic(true, 3D);
                        game.Solver.Solve(true);
                        return game.TotalStates.Log2();
                    },
                    dic =>
                    {
                        foreach (var key in stuff.Keys)
                        {
                            var sum = dic.Where(kvp => kvp.Key.Item1 == key).Sum(kvp => kvp.Value);
                            if (sum == 0)
                                continue;
                            int v;
                            if (!dic.TryGetValue(new Tuple<double, double>(key, 0D), out v))
                                v = 0;
                            sw.WriteLine($"{key}\t{v}\t{sum}");
                        }
                        sw.Flush();
                    });
        }

        private static void Process<T, TResult>(ConcurrentDictionary<T, int> stuff, Func<T, TResult> action,
                                                Action<ConcurrentDictionary<Tuple<T, TResult>, int>> damp)
        {
            var rwLock = new ReaderWriterLockSlim();
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
                        var flag = false;
                        var par = default(T);
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
                                par = t;
                                flag = true;
                                Interlocked.Decrement(ref todo);
                                break;
                            }
                        }
                        if (flag == false)
                            break;

                        lock (locks[index])
                            timeouts[index] = 100;

                        Interlocked.Increment(ref doing);
                        var result = action(par);
                        Interlocked.Decrement(ref doing);

                        rwLock.EnterReadLock();
                        dic.AddOrUpdate(new Tuple<T, TResult>(par, result), 1, (key, val) => val + 1);
                        rwLock.ExitReadLock();

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
                        Console.Write($"{timeouts[i]} ");
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
                {
                    rwLock.EnterWriteLock();
                    damp(dic);
                    dic.Clear();
                    rwLock.ExitWriteLock();
                }
                Thread.Sleep(1000);
            }
            rwLock.EnterWriteLock();
            damp(dic);
            dic.Clear();
            rwLock.ExitWriteLock();
        }
    }
}
