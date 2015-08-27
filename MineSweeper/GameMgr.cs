using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Threading;

namespace MineSweeper
{
    /// <summary>
    ///     模式
    /// </summary>
    [Flags]
    public enum SolvingMode
    {
        /// <summary>
        ///     不自动求解
        /// </summary>
        None = 0x0,

        /// <summary>
        ///     自动半程求解
        /// </summary>
        Half = 0x1,

        /// <summary>
        ///     自动概率求解
        /// </summary>
        Probability = 0x2 | Half,

        /// <summary>
        ///     自动决策求解
        /// </summary>
        Automatic = 0x4 | Probability,

        /// <summary>
        ///     自动扩展求解
        /// </summary>
        Extended = 0x8 | Automatic
    }

    public sealed class Block : IComparable<Block>
    {
        /// <summary>
        ///     横坐标
        /// </summary>
        public int X { get; }

        /// <summary>
        ///     纵坐标
        /// </summary>
        public int Y { get; }

        /// <summary>
        ///     是否翻开
        /// </summary>
        public bool IsOpen { get; internal set; }

        /// <summary>
        ///     是否有雷
        /// </summary>
        private bool m_IsMine;

        /// <summary>
        ///     是否有雷，只能在翻开或游戏结束后查看
        /// </summary>
        public bool IsMine
        {
            get
            {
                if (!IsOpen &&
                    m_Mgr.Started)
                    throw new InvalidOperationException("此格尚未翻开，不能查看是否有雷");
                return m_IsMine;
            }
            internal set { m_IsMine = value; }
        }

        /// <summary>
        ///     是否有雷
        /// </summary>
        internal bool IsMineInternal() => m_IsMine;

        /// <summary>
        ///     周围的格中雷数
        /// </summary>
        private int m_Degree;

        /// <summary>
        ///     周围的格中雷数，只能在翻开或游戏结束后查看
        /// </summary>
        public int Degree
        {
            get
            {
                if (!IsOpen &&
                    m_Mgr.Started)
                    throw new InvalidOperationException("此格尚未翻开，不能查看周围的格中雷数");
                return m_Degree;
            }
            internal set { m_Degree = value; }
        }

        public int Index => X * m_Mgr.TotalHeight + Y;

        /// <summary>
        ///     游戏
        /// </summary>
        private readonly GameMgr m_Mgr;

        internal Block(int x, int y, GameMgr mgr)
        {
            X = x;
            Y = y;
            m_Mgr = mgr;
        }

        public int CompareTo(Block other) => Index.CompareTo(other.Index);
    }


    public enum BlockStatus
    {
        Unknown = -127,
        Mine = -1,
        Blank = -2
    };

    public sealed class GameMgr : IDisposable
    {
        /// <summary>
        ///     状态已更新
        /// </summary>
        public delegate void StatusUpdatedEventHandler();

        public event StatusUpdatedEventHandler StatusUpdated;

        /// <summary>
        ///     模式
        /// </summary>
        private SolvingMode m_Mode;

        /// <summary>
        ///     模式
        /// </summary>
        public SolvingMode Mode
        {
            get { return m_Mode; }
            set
            {
                if (m_Mode == value)
                    return;

                Cancel();
                m_Mode = value;
                if (!m_Mode.HasFlag(SolvingMode.Extended))
                {
                    //DegreeDist = null;
                    //Quantity = null;
                }
                if (!m_Mode.HasFlag(SolvingMode.Automatic)) { }
                if (!m_Mode.HasFlag(SolvingMode.Probability))
                {
                    Probabilities = null;
                    Bits = double.NaN;
                }
                if (!m_Mode.HasFlag(SolvingMode.Half))
                {
                    PreferredBlocks = null;
                    InferredStatuses = null;
                }
            }
        }

        /// <summary>
        ///     格
        /// </summary>
        private readonly List<Block> m_Blocks;

        /// <summary>
        ///     宽度
        /// </summary>
        public int TotalWidth { get; }

        /// <summary>
        ///     高度
        /// </summary>
        public int TotalHeight { get; }

        /// <summary>
        ///     总雷数
        /// </summary>
        public int TotalMines { get; }

        /// <summary>
        ///     是否已开始
        /// </summary>
        public bool Started { get; private set; }

        /// <summary>
        ///     是否胜利
        /// </summary>
        public bool Succeed { get; private set; }

        /// <summary>
        ///     待翻开格数
        /// </summary>
        // ReSharper disable once MemberCanBePrivate.Global
        public int ToOpen { get; private set; }

        /// <summary>
        ///     推测的状态
        /// </summary>
        public List<BlockStatus> InferredStatuses;

        /// <summary>
        ///     概率
        /// </summary>
        public List<double> Probabilities;

        /// <summary>
        ///     总信息量
        /// </summary>
        public double Bits { get; private set; }

        /// <summary>
        ///     初始信息量
        /// </summary>
        public double AllBits { get; private set; }

        /// <summary>
        ///     确定最佳格
        /// </summary>
        public List<Block> BestBlocks { get; private set; }

        /// <summary>
        ///     最佳格
        /// </summary>
        /// l
        public List<Block> PreferredBlocks { get; set; }

        /// <summary>
        ///     锁
        /// </summary>
        private readonly ReaderWriterLockSlim m_Lock = new ReaderWriterLockSlim();

        public void EnterReadLock() => m_Lock.EnterReadLock();
        public void ExitReadLock() => m_Lock.ExitReadLock();

        /// <summary>
        ///     是否正在求解
        /// </summary>
        public bool Solving { get; private set; }

        /// <summary>
        ///     工作线程
        /// </summary>
        private Thread m_Backgrounding;

        public GameMgr(int width, int height, int totalMines)
        {
            TotalWidth = width;
            TotalHeight = height;
            TotalMines = totalMines;
            CacheBinomials(width * height, totalMines);
            m_NativeObject = CreateGameMgr(width, height, totalMines);

            m_Blocks = new List<Block>();
            for (var i = 0; i < width; i++)
                for (var j = 0; j < height; j++)
                    m_Blocks.Add(new Block(i, j, this));
        }

        /// <summary>
        ///     获取某一位置的格
        /// </summary>
        /// <param name="x">横坐标</param>
        /// <param name="y">纵坐标</param>
        /// <returns>格</returns>
        public Block this[int x, int y] => m_Blocks[x * TotalHeight + y];

        /// <summary>
        ///     翻开某一位置，如翻出0则自动翻开周边位置
        /// </summary>
        /// <param name="x">横坐标</param>
        /// <param name="y">纵坐标</param>
        public void OpenBlock(int x, int y)
        {
            if (Solving)
                return;

            if (!Started)
                return;

            OpenBlock(m_NativeObject, x, y);
            FetchStatus();
            Solve();
        }

        /// <summary>
        ///     求解
        /// </summary>
        public void Solve()
        {
            if (Solving)
                return;

            if (!Started)
                return;

            if (!Mode.HasFlag(SolvingMode.Half))
                return;

            m_Backgrounding = new Thread(ProcessSolve);
            Solving = true;
            m_Backgrounding.Start();
        }

        /// <summary>
        ///     取消后台求解任务
        /// </summary>
        public void Cancel()
        {
            if (!Solving)
                return;

            if (m_Backgrounding == null)
                return;

            m_Backgrounding.Abort();
            m_Backgrounding = null;

            Solving = false;
        }

        /// <summary>
        ///     半自动操作一步
        /// </summary>
        public void SemiAutomaticStep()
        {
            if (Solving)
                return;

            if (!Started)
                return;

            if (!Mode.HasFlag(SolvingMode.Half))
                return;

            m_Backgrounding = new Thread(ProcessSemiAutomaticStep);
            Solving = true;
            m_Backgrounding.Start();
        }

        /// <summary>
        ///     半自动操作
        /// </summary>
        public void SemiAutomatic()
        {
            if (Solving)
                return;

            if (!Started)
                return;

            if (!Mode.HasFlag(SolvingMode.Half))
                return;

            m_Backgrounding = new Thread(ProcessSemiAutomatic);
            Solving = true;
            m_Backgrounding.Start();
        }

        /// <summary>
        ///     按特定策略自动操作一次
        /// </summary>
        public void AutomaticStep()
        {
            if (Solving)
                return;

            if (!Started)
                return;

            if (!Mode.HasFlag(SolvingMode.Automatic))
                return;

            m_Backgrounding = new Thread(ProcessAutomaticStep);
            Solving = true;
            m_Backgrounding.Start();
        }

        #region PInvokes

        [DllImport("MineSweeperSolver.dll")]
        private static extern void CacheBinomials(int n, int m);

        [DllImport("MineSweeperSolver.dll")]
        private static extern IntPtr CreateGameMgr(int width, int height, int totalMines);

        [DllImport("MineSweeperSolver.dll")]
        private static extern void DisposeGameMgr(IntPtr mgr);

        [DllImport("MineSweeperSolver.dll")]
        private static extern void OpenBlock(IntPtr mgr, int x, int y);

        [DllImport("MineSweeperSolver.dll")]
        private static extern void Solve(IntPtr mgr, bool withProb);

        [DllImport("MineSweeperSolver.dll")]
        private static extern IntPtr GetGameStatus(IntPtr mgr);

        [DllImport("MineSweeperSolver.dll")]
        private static extern void ReleaseGameStatus(IntPtr status);

        [DllImport("MineSweeperSolver.dll")]
        private static extern bool SemiAutomaticStep(IntPtr mgr, bool withProb);

        [DllImport("MineSweeperSolver.dll")]
        private static extern bool SemiAutomatic(IntPtr mgr, bool withProb);

        [DllImport("MineSweeperSolver.dll")]
        private static extern bool AutomaticStep(IntPtr mgr);

        [DllImport("MineSweeperSolver.dll")]
        private static extern bool Automatic(IntPtr mgr);

        [DllImport("MineSweeperSolver.dll")]
        private static extern bool OpenOptimalBlocks(IntPtr mgr);

        #endregion PInvokes

        #region Wrapper

        /// <summary>
        ///     非托管对象
        /// </summary>
        private IntPtr m_NativeObject;

        [StructLayout(LayoutKind.Sequential)]
        private struct BlockProperty
        {
            // ReSharper disable FieldCanBeMadeReadOnly.Local
            // ReSharper disable MemberCanBePrivate.Local
            [MarshalAs(UnmanagedType.U4)] public int Index;
            [MarshalAs(UnmanagedType.U4)] public int X;
            [MarshalAs(UnmanagedType.U4)] public int Y;
            [MarshalAs(UnmanagedType.U4)] public int Degree;
            [MarshalAs(UnmanagedType.U4)] public bool IsOpen;
            [MarshalAs(UnmanagedType.U4)] public bool IsMine;
            // ReSharper restore MemberCanBePrivate.Local
            // ReSharper restore FieldCanBeMadeReadOnly.Local
        }

        [StructLayout(LayoutKind.Sequential)]
        private struct GameStatus
        {
            // ReSharper disable FieldCanBeMadeReadOnly.Local
            // ReSharper disable MemberCanBePrivate.Local
            [MarshalAs(UnmanagedType.U4)] public int TotalWidth;
            [MarshalAs(UnmanagedType.U4)] public int TotalHeight;
            [MarshalAs(UnmanagedType.U4)] public int TotalBlocks;
            [MarshalAs(UnmanagedType.U4)] public int TotalMines;
            [MarshalAs(UnmanagedType.U4)] public bool Started;
            [MarshalAs(UnmanagedType.U4)] public bool Succeed;
            [MarshalAs(UnmanagedType.U8)] public double Bits;
            [MarshalAs(UnmanagedType.U8)] public double AllBits;
            [MarshalAs(UnmanagedType.U4)] public int ToOpen;

            [MarshalAs(UnmanagedType.U8)] public IntPtr Blocks;
            [MarshalAs(UnmanagedType.U8)] public IntPtr InferredStatus;
            [MarshalAs(UnmanagedType.U8)] public IntPtr Probabilities;


            [MarshalAs(UnmanagedType.U4)] public int BestBlockCount;
            [MarshalAs(UnmanagedType.U8)] public IntPtr BestBlocks;
            [MarshalAs(UnmanagedType.U4)] public int PreferredBlockCount;
            [MarshalAs(UnmanagedType.U8)] public IntPtr PreferredBlocks;
            // ReSharper restore MemberCanBePrivate.Local
            // ReSharper restore FieldCanBeMadeReadOnly.Local
        };

        public void FetchStatus()
        {
            m_Lock.EnterWriteLock();
            try
            {
                UpdateStatus();
            }
            finally
            {
                Solving = false;
                m_Lock.ExitWriteLock();
            }
            OnStatusUpdated();
        }

        private void UpdateStatus()
        {
            var ptr = GetGameStatus(m_NativeObject);
            try
            {
                unsafe
                {
                    var st = *(GameStatus*)ptr.ToPointer();
                    Started = st.Started;
                    Succeed = st.Succeed;
                    ToOpen = st.ToOpen;
                    AllBits = st.AllBits;
                    Bits = st.Bits;
                    InferredStatuses = new List<BlockStatus>(st.TotalBlocks);
                    Probabilities = new List<double>(st.TotalBlocks);
                    var pProp = (BlockProperty*)st.Blocks.ToPointer();
                    var pInf = (BlockStatus*)st.InferredStatus.ToPointer();
                    var pProb = (double*)st.Probabilities.ToPointer();
                    for (var i = 0; i < st.TotalBlocks; i++)
                    {
                        var property = *pProp++;
                        m_Blocks[i].IsOpen = property.IsOpen;
                        m_Blocks[i].IsMine = property.IsMine;
                        m_Blocks[i].Degree = property.Degree;
                        InferredStatuses.Add(*pInf++);
                        Probabilities.Add(*pProb++);
                    }
                    BestBlocks = new List<Block>(st.BestBlockCount);
                    var pBest = (int*)st.BestBlocks.ToPointer();
                    for (var i = 0; i < st.BestBlockCount; i++)
                        BestBlocks.Add(m_Blocks[*pBest++]);
                    PreferredBlocks = new List<Block>(st.PreferredBlockCount);
                    var pPref = (int*)st.PreferredBlocks.ToPointer();
                    for (var i = 0; i < st.PreferredBlockCount; i++)
                        PreferredBlocks.Add(m_Blocks[*pPref++]);
                }
            }
            finally
            {
                ReleaseGameStatus(ptr);
            }
        }

        private void ProcessSolve()
        {
            Solve(m_NativeObject, Mode.HasFlag(SolvingMode.Probability));
            FetchStatus();
        }

        private void ProcessSemiAutomaticStep()
        {
            if (BestBlocks.Any())
            {
                OpenOptimalBlocks(m_NativeObject);
                ProcessSolve();
            }
            else
            {
                SemiAutomaticStep(m_NativeObject, Mode.HasFlag(SolvingMode.Probability));
                FetchStatus();
            }
        }

        private void ProcessSemiAutomatic()
        {
            if (BestBlocks.Any())
                OpenOptimalBlocks(m_NativeObject);
            SemiAutomatic(m_NativeObject, Mode.HasFlag(SolvingMode.Probability));
            ProcessSolve();
        }

        private void ProcessAutomaticStep()
        {
            if (BestBlocks.Any() ||
                PreferredBlocks.Any())
            {
                OpenOptimalBlocks(m_NativeObject);
                ProcessSolve();
            }
            else
            {
                AutomaticStep(m_NativeObject);
                FetchStatus();
            }
        }

        public void Dispose() => Dispose(true);

        private void Dispose(bool bDisposing)
        {
            if (m_NativeObject != IntPtr.Zero)
            {
                DisposeGameMgr(m_NativeObject);
                m_NativeObject = IntPtr.Zero;
            }

            if (bDisposing)
                GC.SuppressFinalize(this);
        }

        ~GameMgr() { Dispose(false); }

        #endregion

        private void OnStatusUpdated() { StatusUpdated?.Invoke(); }
    }
}
