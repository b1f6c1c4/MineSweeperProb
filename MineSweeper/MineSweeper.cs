using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Windows.Forms;
using MineSweeperCalc;
using MineSweeperCalc.Solver;

namespace MineSweeper
{
    public partial class MineSweeper : Form
    {
        private GameMgrBuffered m_Mgr;
        private readonly float m_ScaleFactor;
        private double m_AllLog2;
        private Block m_CurrentBlock;
        private List<UIBlock> m_UIBlocks;

        private delegate void UpdateDelegate();

        [DllImport("user32.dll")]
        private static extern bool SetProcessDPIAware();

        [DllImport("gdi32.dll")]
        private static extern int GetDeviceCaps(IntPtr hdc, int nIndex);

        [DllImport("user32.dll")]
        private static extern IntPtr GetDC(IntPtr hWnd);

        [DllImport("user32.dll")]
        // ReSharper disable once InconsistentNaming
        private static extern bool ReleaseDC(IntPtr hWnd, IntPtr hDC);

        public MineSweeper()
        {
            SetProcessDPIAware();
            InitializeComponent();

            var hdc = GetDC(IntPtr.Zero);
            m_ScaleFactor = GetDeviceCaps(hdc, 88) / 96F;
            ReleaseDC(IntPtr.Zero, hdc);

            Scale(new SizeF(m_ScaleFactor, m_ScaleFactor));
        }

        private void MineSweeper_Load(object sender, EventArgs e) { Reset(); }

        private void UpdateAll()
        {
            m_Mgr.EnterReadLock();
            try
            {
                var sb = new StringBuilder();
                sb.Append($"[{m_Mgr.Mode}]");
                if (m_Mgr.Mode.HasFlag(SolvingMode.Probability) &&
                    m_Mgr.Solver.Probability != null)
                {
                    var curBits = m_Mgr.Solver.TotalStates.Log2();
                    sb.Append($" {curBits:F2}/{m_AllLog2:F2}b, {1 - curBits / m_AllLog2:P0}");
                    progressBar1.Value = (int)(2147483647 * (1 - curBits / m_AllLog2));
                }
                else
                    sb.Append($" {m_AllLog2:F2}b");

                if (m_CurrentBlock != null)
                    if (m_Mgr.Mode.HasFlag(SolvingMode.Probability) &&
                        m_Mgr.Solver.Probability != null)
                    {
                        sb.Append($" M{m_Mgr.Solver.Probability[m_CurrentBlock]:P2}");

                        if (m_Mgr.Mode.HasFlag(SolvingMode.Extended) &&
                            (m_Mgr.DegreeDist != null &&
                             m_Mgr.Quantity != null))
                        {
                            sb.Append($"Q{m_Mgr.Quantity[m_CurrentBlock]:F2}b: ");

                            var dist = m_Mgr.DegreeDist[m_CurrentBlock];
                            if (dist != null)
                            {
                                foreach (var kvp in dist.OrderBy(kvp => kvp.Key))
                                    sb.Append($"{kvp.Value:P1}[{kvp.Key}],");
                                if (dist.Count > 0)
                                    sb.Remove(sb.Length - 1, 1);
                            }
                        }
                    }

                if (!m_Mgr.Started)
                    sb.Append(m_Mgr.Succeed ? " Succeed" : " Failed");

                if (m_Mgr.Solving)
                    sb.Append(" Running...");

                Text = sb.ToString();
            }
            finally
            {
                m_Mgr.ExitReadLock();
            }

            foreach (var ub in m_UIBlocks)
                ub.FetchState();
        }

        private void Reset()
        {
            var mode = m_Mgr?.Mode ?? SolvingMode.Probability;
            m_Mgr = new GameMgrBuffered(30, 16, 99, (int)DateTime.Now.Ticks, Strategies.MinProbMaxZeroProbMaxQuantity)
                        { Mode = mode };

            if (m_UIBlocks != null)
                foreach (var ub in m_UIBlocks)
                {
                    Controls.Remove(ub);
                    ub.Dispose();
                }

            m_UIBlocks = new List<UIBlock>();
            for (var i = 0; i < m_Mgr.TotalWidth; i++)
                for (var j = 0; j < m_Mgr.TotalHeight; j++)
                {
                    var ub = new UIBlock
                                 {
                                     Location = new Point(i * 25, j * 25),
                                     TheMgr = m_Mgr,
                                     TheBlock = m_Mgr[i, j],
                                     Font = new Font("Consolas", 16F)
                                 };
                    ub.Scale(new SizeF(m_ScaleFactor, m_ScaleFactor));
                    ub.MouseClick += Block_Click;
                    ub.MouseEnter += Block_Enter;
                    Controls.Add(ub);
                    m_UIBlocks.Add(ub);
                }

            var blocks = m_Mgr.TotalWidth * m_Mgr.TotalHeight;
            BinomialHelper.UpdateTo(blocks, m_Mgr.TotalMines);

            m_AllLog2 = BinomialHelper.Binomial(blocks, m_Mgr.TotalMines).Log2();

            ClientSize = new Size(
                (int)(m_Mgr.TotalWidth * 25 * m_ScaleFactor),
                (int)(m_Mgr.TotalHeight * 25 * m_ScaleFactor) + progressBar1.Height);

            Solve();
        }

        private void Block_Enter(object sender, EventArgs e)
        {
            m_CurrentBlock = ((UIBlock)sender).TheBlock;
            UpdateAll();
        }

        private void Block_Click(object sender, MouseEventArgs e)
        {
            var block = (UIBlock)sender;
            if (e.Button != MouseButtons.Left)
                return;

            if (m_Mgr.Solving ||
                !m_Mgr.Started ||
                block.TheBlock.IsOpen)
                return;

            m_Mgr.OpenBlock(block.TheBlock.X, block.TheBlock.Y);
            Solve();
        }

        private void Form1_KeyUp(object sender, KeyEventArgs e)
        {
            switch (e.KeyCode)
            {
                case Keys.S:
                    if (m_Mgr.Started)
                    {
                        m_Mgr.SemiAutomaticStep();
                        Solve();
                    }
                    break;
                case Keys.X:
                    if (m_Mgr.Started)
                    {
                        m_Mgr.SemiAutomatic();
                        Solve();
                    }
                    break;
                case Keys.A:
                    if (m_Mgr.Started &&
                        m_Mgr.Mode.HasFlag(SolvingMode.Automatic))
                    {
                        if (!m_Mgr.SemiAutomaticStep())
                            m_Mgr.AutomaticStep(true);
                        Solve();
                    }
                    break;
                case Keys.Z:
                    if (m_Mgr.Started &&
                        m_Mgr.Mode.HasFlag(SolvingMode.Automatic))
                    {
                        var flag = false;
                        while (m_Mgr.SemiAutomaticStep())
                            flag = true;
                        if (!flag)
                            m_Mgr.AutomaticStep(true);
                        Solve();
                    }
                    break;
                case Keys.R:
                    Reset();
                    break;
                case Keys.D0:
                    m_Mgr.Mode = SolvingMode.None;
                    Solve();
                    break;
                case Keys.D1:
                    m_Mgr.Mode = SolvingMode.Half;
                    Solve();
                    break;
                case Keys.D2:
                    m_Mgr.Mode = SolvingMode.Probability;
                    Solve();
                    break;
                case Keys.D3:
                    m_Mgr.Mode = SolvingMode.Automatic;
                    Solve();
                    break;
                case Keys.D4:
                    m_Mgr.Mode = SolvingMode.Extended;
                    Solve();
                    break;
                case Keys.C:
                    m_Mgr.Cancel();
                    break;
            }
        }

        private void Solve()
        {
            UpdateAll();
            m_Mgr.Solve().ContinueWith(t => Invoke(new UpdateDelegate(UpdateAll), new object[] { }));
        }
    }
}
