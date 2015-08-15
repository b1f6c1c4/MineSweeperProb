using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Numerics;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using MineSweeperCalc;
using MineSweeperCalc.Solver;

namespace MineSweeper
{
    public partial class Form1 : Form
    {
        [Flags]
        private enum Mode
        {
            None = 0x0,
            Half = 0x1,
            Probability = 0x2 | Half,
            Automaic = 0x4 | Probability,
            Extended = 0x8 | Automaic
        }

        private Mode m_Mode;
        private GameMgr m_Mgr;
        private const float BlockSize = 25F;
        private readonly float m_ScaleFactor;
        private double m_AllLog2;
        private List<Block> m_Bests;
        private Dictionary<Block, Dictionary<int, double>> m_DegreeDist;
        private Dictionary<Block, double> m_Quantity;
        private readonly object m_Lock = new object();
        private bool m_Editing;
        private Task m_Backgrounding;
        private Block m_CurrentBlock;

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

        public Form1()
        {
            InitializeComponent();
            SetProcessDPIAware();

            var hdc = GetDC(IntPtr.Zero);
            m_ScaleFactor = GetDeviceCaps(hdc, 88) / 96F;
            ReleaseDC(IntPtr.Zero, hdc);

            m_Editing = false;

            Reset();
        }

        private void UpdateText()
        {
            var sb = new StringBuilder();
            sb.Append($"[{m_Mode}]");
            if (m_Mode.HasFlag(Mode.Probability) &&
                m_Mgr.Solver.Probability != null)
            {
                var curBits = m_Mgr.Solver.TotalStates.Log2();
                sb.Append($" {curBits:F2}/{m_AllLog2:F2}b, {1 - curBits / m_AllLog2:P0}");
                progressBar1.Value = (int)(2147483647 * (1 - curBits / m_AllLog2));
            }
            else
                sb.Append($" {m_AllLog2:F2}b");

            if (m_CurrentBlock != null)
            {
                if (m_Mode.HasFlag(Mode.Probability) &&
                    m_Mgr.Solver.Probability != null)
                {
                    sb.Append($" M{m_Mgr.Solver.Probability[m_CurrentBlock]:P2}");

                    if (m_Mode.HasFlag(Mode.Extended) &&
                        (m_DegreeDist != null &&
                         m_Quantity != null))
                    {
                        sb.Append($"Q{m_Quantity[m_CurrentBlock]:F2}b: ");

                        var dist = m_DegreeDist[m_CurrentBlock];
                        if (dist != null)
                        {
                            foreach (var kvp in dist.OrderBy(kvp => kvp.Key))
                                sb.Append($"{kvp.Value:P1}[{kvp.Key}],");
                            if (dist.Count > 0)
                                sb.Remove(sb.Length - 1, 1);
                        }
                    }
                }
            }

            if (!m_Mgr.Started)
                sb.Append(m_Mgr.Suceed ? " Suceed" : " Failed");

            if (m_Editing)
                sb.Append(" Running...");

            Text = sb.ToString();
        }

        private void RePaint(Graphics g = null)
        {
            lock (m_Lock)
            {
                g = g ?? CreateGraphics();

                var fontO = new Font("Consolas", 16 * m_ScaleFactor);
                var fontC = new Font("Consolas", 8 * m_ScaleFactor);
                var white = new SolidBrush(Color.White);
                var black = new SolidBrush(Color.Black);
                var red = new SolidBrush(Color.Red);
                var green = new SolidBrush(Color.Green);
                var gray = new SolidBrush(Color.DarkGray);

                var a = BlockSize * m_ScaleFactor;
                g.Clear(m_Editing ? Color.WhiteSmoke : Color.DimGray);
                if (m_Mgr.Started &&
                    m_Mode.HasFlag(Mode.Half) &&
                    !m_Editing)
                    for (var i = 0; i < m_Mgr.TotalWidth; i++)
                        for (var j = 0; j < m_Mgr.TotalHeight; j++)
                        {
                            var block = m_Mgr[i, j];
                            if (m_Mode.HasFlag(Mode.Probability) &&
                                m_Mgr.Solver.Probability != null)
                            {
                                var v = (int)((1 - m_Mgr.Solver.Probability[block]) * 255);
                                g.FillRectangle(new SolidBrush(Color.FromArgb(v, v, v)), i * a, j * a, a - 1, a - 1);
                            }
                            else if (m_Mgr.Solver[block] == BlockStatus.Mine)
                                g.FillRectangle(black, i * a, j * a, a - 1, a - 1);
                            else if (m_Mgr.Solver[block] == BlockStatus.Blank)
                                g.FillRectangle(white, i * a, j * a, a - 1, a - 1);
                            else
                                g.FillRectangle(gray, i * a, j * a, a - 1, a - 1);
                            if (m_Mgr.Solver[block] == BlockStatus.Mine)
                                g.DrawString("M", block.IsOpen ? fontO : fontC, white, i * a + 4, j * a);
                            else if (m_Mgr.Solver[block] == BlockStatus.Blank)
                                if (block.IsOpen)
                                {
                                    var str = block.Degree == 0 ? string.Empty : block.Degree.ToString();
                                    g.DrawString(str, fontO, black, i * a + 4, j * a);
                                }
                                else
                                    g.DrawString("B", fontC, black, i * a + 4, j * a);
                            if (m_Mode.HasFlag(Mode.Automaic) &&
                                m_Bests != null &&
                                m_Bests.Contains(block))
                                g.FillEllipse(green, i * a + a / 4, j * a + a / 4, a / 2 - 1, a / 2 - 1);
                        }
                else
                    for (var i = 0; i < m_Mgr.TotalWidth; i++)
                        for (var j = 0; j < m_Mgr.TotalHeight; j++)
                        {
                            var block = m_Mgr[i, j];
                            if (!m_Mgr.Started)
                                if (block.IsMine)
                                    g.FillRectangle(m_Mgr.Suceed ? green : red, i * a, j * a, a - 1, a - 1);
                            if (block.IsOpen)
                            {
                                if (!block.IsMine)
                                {
                                    g.FillRectangle(white, i * a, j * a, a - 1, a - 1);
                                    var str = block.Degree == 0 ? string.Empty : block.Degree.ToString();
                                    g.DrawString(str, fontO, black, i * a + 4, j * a);
                                }
                            }
                            else if (m_Mgr.Started)
                                g.FillRectangle(gray, i * a, j * a, a - 1, a - 1);
                        }
            }
        }

        private void Reset()
        {
            m_Mgr = new GameMgr(30, 16, 99, (int)DateTime.Now.Ticks, Strategies.MinProbMaxZeroProbMaxQuantity);

            var blocks = m_Mgr.TotalWidth * m_Mgr.TotalHeight;
            BinomialHelper.UpdateTo(blocks, m_Mgr.TotalMines);

            m_AllLog2 = BinomialHelper.Binomial(blocks, m_Mgr.TotalMines).Log2();

            ClientSize = new Size(
                (int)(m_Mgr.TotalWidth * BlockSize * m_ScaleFactor),
                (int)(m_Mgr.TotalHeight * BlockSize * m_ScaleFactor) + progressBar1.Height);

            Solve();
        }

        private void Form1_Paint(object sender, PaintEventArgs e) => RePaint(e.Graphics);

        private void Form1_MouseUp(object sender, MouseEventArgs e)
        {
            if (m_Mgr.Started)
                if (e.Button == MouseButtons.Left)
                {
                    var x = (int)(e.X / BlockSize / m_ScaleFactor);
                    var y = (int)(e.Y / BlockSize / m_ScaleFactor);
                    if (x < 0 ||
                        x >= m_Mgr.TotalWidth)
                        return;
                    if (y < 0 ||
                        y >= m_Mgr.TotalHeight)
                        return;
                    if (m_Mgr[x, y].IsOpen)
                        return;
                    m_Mgr.OpenBlock(x, y);
                    Solve();
                }
                else if (e.Button == MouseButtons.Right)
                {
                    m_Mgr.SemiAutomaticStep(m_Mode.HasFlag(Mode.Probability));
                    Solve();
                }
            RePaint();
        }

        private void Form1_MouseMove(object sender, MouseEventArgs e)
        {
            if (!m_Mgr.Started)
                return;

            if (!m_Mode.HasFlag(Mode.Probability))
                return;

            var x = (int)(e.X / BlockSize / m_ScaleFactor);
            var y = (int)(e.Y / BlockSize / m_ScaleFactor);
            if (x < 0 ||
                x >= m_Mgr.TotalWidth)
                return;
            if (y < 0 ||
                y >= m_Mgr.TotalHeight)
                return;
            if (m_Mgr[x, y].IsOpen)
                return;

            m_CurrentBlock = m_Mgr[x, y];
            UpdateText();
        }

        private void Form1_KeyUp(object sender, KeyEventArgs e)
        {
            switch (e.KeyCode)
            {
                case Keys.A:
                    if (m_Mgr.Started &&
                        m_Mode.HasFlag(Mode.Automaic))
                    {
                        var flag = false;
                        while (m_Mgr.SemiAutomaticStep())
                            flag = true;
                        if (!flag)
                            m_Mgr.AutomaticStep(true);
                        Solve();
                    }
                    break;
                case Keys.X:
                    if (m_Mgr.Started &&
                        m_Mode.HasFlag(Mode.Half))
                    {
                        m_Mgr.SemiAutomatic();
                        Solve();
                    }
                    break;
                case Keys.R:
                    Reset();
                    break;
                case Keys.D0:
                    m_Mode = Mode.None;
                    RePaint();
                    break;
                case Keys.D1:
                    m_Mode = Mode.Half;
                    Solve();
                    break;
                case Keys.D2:
                    m_Mode = Mode.Probability;
                    Solve();
                    break;
                case Keys.D3:
                    m_Mode = Mode.Automaic;
                    Solve();
                    break;
                case Keys.D4:
                    m_Mode = Mode.Extended;
                    Solve();
                    break;
                case Keys.C:
                    if (e.Control)
                    {
                        m_Backgrounding?.Dispose();
                        m_Editing = false;
                    }
                    RePaint();
                    break;
            }
        }

        private void Solve()
        {
            if (!m_Mgr.Started || !m_Mode.HasFlag(Mode.Half))
            {
                UpdateText();
                RePaint();
                return;
            }

            m_Backgrounding = new Task(
                () =>
                {
                    lock (m_Lock)
                    {
                        if (m_Editing)
                            return;
                        m_Editing = true;
                    }

                    m_Mgr.Solver.Solve(m_Mode.HasFlag(Mode.Probability));

                    if (m_Mode.HasFlag(Mode.Automaic))
                    {
                        m_Bests = new List<Block>();
                        for (var i = 0; i < m_Mgr.TotalWidth; i++)
                            for (var j = 0; j < m_Mgr.TotalHeight; j++)
                                if (!m_Mgr[i, j].IsOpen &&
                                    m_Mgr.Solver[m_Mgr[i, j]] == BlockStatus.Blank)
                                    m_Bests.Add(m_Mgr[i, j]);

                        var blks = new List<Block>();
                        for (var i = 0; i < m_Mgr.TotalWidth; i++)
                            for (var j = 0; j < m_Mgr.TotalHeight; j++)
                                if (m_Mgr.Solver[m_Mgr[i, j]] == BlockStatus.Unknown)
                                    blks.Add(m_Mgr[i, j]);
                        if (m_Bests.Count == 0)
                            m_Bests = m_Mgr.DecisionMaker(blks, m_Mgr, true).ToList();

                        if (m_Mode.HasFlag(Mode.Extended))
                        {
                            m_DegreeDist = new Dictionary<Block, Dictionary<int, double>>();
                            m_Quantity = new Dictionary<Block, double>();
                            for (var i = 0; i < m_Mgr.TotalWidth; i++)
                                for (var j = 0; j < m_Mgr.TotalHeight; j++)
                                    if (!m_Mgr[i, j].IsOpen)
                                    {
                                        var block = m_Mgr[i, j];
                                        var dic = m_Mgr.Solver.DistributionCond(
                                                                                block.Surrounding,
                                                                                new BlockSet<Block>(block),
                                                                                0);
                                        var total = dic.Aggregate(BigInteger.Zero, (cur, kvp) => cur + kvp.Value);
                                        var pDic = total.IsZero
                                                       ? new Dictionary<int, double>()
                                                       : dic.ToDictionary(kvp => kvp.Key, kvp => kvp.Value.Over(total));
                                        m_DegreeDist[block] = pDic;
                                        var p0 = (1 - m_Mgr.Solver.Probability[block]);
                                        var q0 = m_Mgr.Solver.TotalStates.Log2();
                                        var qu = -100D * Math.Pow(1 - p0, 2) +
                                                 dic.Sum(
                                                         kvp =>
                                                         {
                                                             if (kvp.Value == 0)
                                                                 return 0D;
                                                             var p = p0 * pDic[kvp.Key];
                                                             var q = q0 - kvp.Value.Log2();
                                                             return p * q;
                                                         });
                                        m_Quantity[block] = qu;
                                    }
                        }
                    }
                    lock (m_Lock)
                        m_Editing = false;
                });
            m_Backgrounding.ContinueWith(
                                         t => Invoke(
                                                     new UpdateDelegate(
                                                         () =>
                                                         {
                                                             UpdateText();
                                                             RePaint();
                                                         }),
                                                     new object[] { }));
            m_Backgrounding.Start();
        }
    }
}
