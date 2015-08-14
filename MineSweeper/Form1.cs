using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Numerics;
using System.Runtime.InteropServices;
using System.Text;
using System.Windows.Forms;
using MineSweeperCalc;
using MineSweeperCalc.Solver;

namespace MineSweeper
{
    public partial class Form1 : Form
    {
        private GameMgr m_Mgr;
        private bool m_ShowProb;
        private const float BlockSize = 25F;
        private readonly float m_ScaleFactor;
        private double m_AllLog2;
        private List<Block> m_Bests;
        private Dictionary<Block, Dictionary<int, double>> m_DegreeDist;
        private Dictionary<Block, double> m_Quantity;

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

            Reset();
        }

        private void RePaint(Graphics g = null)
        {
            var fontO = new Font("Consolas", 16 * m_ScaleFactor);
            var fontC = new Font("Consolas", 8 * m_ScaleFactor);
            var white = new SolidBrush(Color.White);
            var black = new SolidBrush(Color.Black);
            var red = new SolidBrush(Color.Red);
            var green = new SolidBrush(Color.Green);
            var gray = new SolidBrush(Color.DarkGray);

            var a = BlockSize * m_ScaleFactor;
            g = g ?? CreateGraphics();
            g.ResetClip();
            g.Clear(Color.WhiteSmoke);
            if (m_Mgr.Started && m_ShowProb)
                for (var i = 0; i < m_Mgr.TotalWidth; i++)
                    for (var j = 0; j < m_Mgr.TotalHeight; j++)
                    {
                        var v = (int)((1 - m_Mgr.Solver.Probability[m_Mgr[i, j]]) * 255);
                        g.FillRectangle(new SolidBrush(Color.FromArgb(v, v, v)), i * a, j * a, a - 1, a - 1);
                        if (m_Mgr.Solver[m_Mgr[i, j]] == BlockStatus.Mine)
                            g.DrawString("M", m_Mgr[i, j].IsOpen ? fontO : fontC, white, i * a + 4, j * a);
                        else if (m_Mgr.Solver[m_Mgr[i, j]] == BlockStatus.Blank)
                            g.DrawString("B", m_Mgr[i, j].IsOpen ? fontO : fontC, black, i * a + 4, j * a);
                        if (m_Bests.Contains(m_Mgr[i, j]))
                            g.FillEllipse(green, i * a + a / 4, j * a + a / 4, a / 2 - 1, a / 2 - 1);
                    }
            else
                for (var i = 0; i < m_Mgr.TotalWidth; i++)
                    for (var j = 0; j < m_Mgr.TotalHeight; j++)
                    {
                        if (!m_Mgr.Started)
                            if (m_Mgr[i, j].IsMine)
                                g.FillRectangle(m_Mgr.Suceed ? green : red, i * a, j * a, a - 1, a - 1);
                        if (m_Mgr[i, j].IsOpen)
                        {
                            if (!m_Mgr[i, j].IsMine)
                            {
                                g.FillRectangle(white, i * a, j * a, a - 1, a - 1);
                                var str = m_Mgr[i, j].Degree == 0 ? string.Empty : m_Mgr[i, j].Degree.ToString();
                                g.DrawString(str, fontO, black, i * a + 4, j * a);
                            }
                        }
                        else if (m_Mgr.Started)
                            g.FillRectangle(gray, i * a, j * a, a - 1, a - 1);
                    }
        }

        private void Reset()
        {
            m_Mgr = new GameMgr(30, 16, 99, (int)DateTime.Now.Ticks);

            var blocks = m_Mgr.TotalWidth * m_Mgr.TotalHeight;
            BinomialHelper.UpdateTo(blocks, m_Mgr.TotalMines);

            m_ShowProb = false;
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
                    m_Mgr.SemiAutomaticStep();
                    Solve();
                }
            RePaint();
        }

        private void Form1_MouseMove(object sender, MouseEventArgs e)
        {
            if (m_DegreeDist == null ||
                m_Quantity == null)
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

            var sb = new StringBuilder();
            foreach (var kvp in m_DegreeDist[m_Mgr[x, y]].OrderBy(kvp => kvp.Key))
                sb.Append($"{kvp.Value:P1}[{kvp.Key}],");
            if (sb.Length > 0)
                sb.Remove(sb.Length - 1, 1);

            var curBits = m_Mgr.Solver.TotalStates.Log2();

            Text =
                $"{curBits:F2}/{m_AllLog2:F2} bits, {1 - curBits / m_AllLog2:P0}; M{m_Mgr.Solver.Probability[m_Mgr[x, y]]:P2}; Q{m_Quantity[m_Mgr[x, y]]:F2}b: {sb}";
        }

        private void Form1_KeyUp(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Space)
            {
                m_ShowProb ^= true;
                RePaint();
            }
            else if (e.KeyCode == Keys.X)
            {
                if (m_Mgr.Started)
                {
                    m_Mgr.SemiAutomatic();
                    Solve();
                }
                RePaint();
            }
            else if (e.KeyCode == Keys.R)
            {
                Reset();
                RePaint();
            }
        }

        private void Solve()
        {
            if (!m_Mgr.Started)
            {
                if (m_Mgr.Suceed)
                {
                    Text = "Suceed";
                    progressBar1.Value = 2147483647;
                }
                else
                    Text = $"Failed At {m_Mgr.Solver.TotalStates.Log2():F2}bits";
                return;
            }

            m_Mgr.Solver.Solve();

            var curBits = m_Mgr.Solver.TotalStates.Log2();

            Text = $"{curBits:F2}/{m_AllLog2:F2} bits, {1 - curBits / m_AllLog2:P0}";

            progressBar1.Value = (int)(2147483647 * (1 - curBits / m_AllLog2));

            m_Bests = new List<Block>();
            for (var i = 0; i < m_Mgr.TotalWidth; i++)
                for (var j = 0; j < m_Mgr.TotalHeight; j++)
                    if (!m_Mgr[i, j].IsOpen &&
                        m_Mgr.Solver[m_Mgr[i, j]] == BlockStatus.Blank)
                        m_Bests.Add(m_Mgr[i, j]);

            m_DegreeDist = new Dictionary<Block, Dictionary<int, double>>();
            m_Quantity = new Dictionary<Block, double>();
            for (var i = 0; i < m_Mgr.TotalWidth; i++)
                for (var j = 0; j < m_Mgr.TotalHeight; j++)
                    if (!m_Mgr[i, j].IsOpen)
                    {
                        var block = m_Mgr[i, j];
                        var theBlock = new BlockSet<Block>(block);
                        var dic = m_Mgr.Solver.DistributionCond(block.Surrounding, theBlock, 0);
                        var total = dic.Aggregate(BigInteger.Zero, (cur, kvp) => cur + kvp.Value);
                        var pDic = total.IsZero
                                       ? new Dictionary<int, double>()
                                       : dic.ToDictionary(kvp => kvp.Key, kvp => kvp.Value.Over(total));
                        m_DegreeDist[block] = pDic;
                        var q = dic.Sum(
                                        kvp =>
                                        {
                                            if (kvp.Value == 0)
                                                return 0D;
                                            var p = pDic[kvp.Key];
                                            return -p * Math.Log(p, 2);
                                        });
                        m_Quantity[block] = q;
                    }
        }
    }
}
