using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Numerics;
using System.Runtime.InteropServices;
using System.Windows.Forms;
using MineSweeperCalc;
using MineSweeperCalc.Solver;

namespace MineSweeper
{
    public partial class Form1 : Form
    {
        private GameMgr m_Mgr;
        private bool m_ShowProb;
        private const float BlockSize = 38F;
        private double m_AllLog2;
        private List<Block> m_Bests;
        private BigInteger m_Total;

        [DllImport("user32.dll")]
        private static extern bool SetProcessDPIAware();

        public Form1()
        {
            InitializeComponent();
            SetProcessDPIAware();
            Reset();
        }

        private void RePaint(Graphics g = null)
        {
            var fontO = new Font("Consolas", 24);
            var fontC = new Font("Consolas", 12);
            var white = new SolidBrush(Color.White);
            var black = new SolidBrush(Color.Black);
            var red = new SolidBrush(Color.Red);
            var green = new SolidBrush(Color.Green);
            var gray = new SolidBrush(Color.LightGray);

            g = g ?? CreateGraphics();
            if (m_Mgr.Started && m_ShowProb)
                for (var i = 0; i < m_Mgr.TotalWidth; i++)
                    for (var j = 0; j < m_Mgr.TotalHeight; j++)
                    {
                        var v = (int)((1 - m_Mgr.Solver.Probability[m_Mgr[i, j]]) * 255);
                        g.FillRectangle(
                                        new SolidBrush(Color.FromArgb(v, v, v)),
                                        i * BlockSize,
                                        j * BlockSize,
                                        BlockSize - 1,
                                        BlockSize - 1);
                        if (m_Mgr.Solver[m_Mgr[i, j]] == BlockStatus.Mine)
                            g.DrawString(
                                         "M",
                                         m_Mgr[i, j].IsOpen ? fontO : fontC,
                                         white,
                                         i * BlockSize,
                                         j * BlockSize);
                        else if (m_Mgr.Solver[m_Mgr[i, j]] == BlockStatus.Blank)
                            g.DrawString(
                                         "B",
                                         m_Mgr[i, j].IsOpen ? fontO : fontC,
                                         black,
                                         i * BlockSize,
                                         j * BlockSize);
                        if (m_Bests.Contains(m_Mgr[i, j]))
                            g.FillEllipse(
                                          green,
                                          i * BlockSize + BlockSize / 4,
                                          j * BlockSize + BlockSize / 4,
                                          BlockSize / 2 - 1,
                                          BlockSize / 2 - 1);
                    }
            else
                for (var i = 0; i < m_Mgr.TotalWidth; i++)
                    for (var j = 0; j < m_Mgr.TotalHeight; j++)
                    {
                        if (!m_Mgr.Started)
                            if (m_Mgr[i, j].IsMine)
                                g.FillRectangle(
                                                m_Mgr.Suceed ? green : red,
                                                i * BlockSize,
                                                j * BlockSize,
                                                BlockSize - 1,
                                                BlockSize - 1);
                        if (m_Mgr[i, j].IsOpen)
                        {
                            if (!m_Mgr[i, j].IsMine)
                            {
                                g.FillRectangle(
                                                white,
                                                i * BlockSize,
                                                j * BlockSize,
                                                BlockSize - 1,
                                                BlockSize - 1);
                                g.DrawString(
                                             m_Mgr[i, j].Degree.ToString(),
                                             fontO,
                                             black,
                                             i * BlockSize,
                                             j * BlockSize);
                            }
                        }
                        else if (m_Mgr.Started)
                            g.FillRectangle(
                                            gray,
                                            i * BlockSize,
                                            j * BlockSize,
                                            BlockSize - 1,
                                            BlockSize - 1);
                    }
        }

        private void Reset()
        {
            m_Mgr = new GameMgr(30, 16, 99, (int)DateTime.Now.Ticks);

            var blocks = m_Mgr.TotalWidth * m_Mgr.TotalHeight;
            BinomialHelper.UpdateTo(blocks, m_Mgr.TotalMines);
            m_AllLog2 = BinomialHelper.Binomial(blocks, m_Mgr.TotalMines).Log2();

            m_ShowProb = false;

            Width = (int)(m_Mgr.TotalWidth * BlockSize) + 22;
            Height = (int)(m_Mgr.TotalHeight * BlockSize) + 56 + progressBar1.Height;
        }

        private void Form1_Paint(object sender, PaintEventArgs e) => RePaint(e.Graphics);

        private void Form1_MouseUp(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Left)
                if (m_Mgr.Started)
                {
                    var x = (int)(e.X / BlockSize);
                    var y = (int)(e.Y / BlockSize);
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
                else
                    Reset();
            else if (e.Button == MouseButtons.Right)
                if (m_Mgr.Started)
                {
                    var flag = false;
                    for (var i = 0; i < m_Mgr.TotalWidth; i++)
                        for (var j = 0; j < m_Mgr.TotalHeight; j++)
                            if (!m_Mgr[i, j].IsOpen &&
                                m_Mgr.Solver[m_Mgr[i, j]] == BlockStatus.Blank)
                            {
                                m_Mgr.OpenBlock(i, j);
                                flag = true;
                            }
                    if (flag && m_Mgr.Started)
                        Solve();
                }
            RePaint();
        }

        private void Form1_KeyUp(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Space)
            {
                m_ShowProb ^= true;
                if (m_ShowProb)
                    Solve();
                RePaint();
            }
            else if (e.KeyCode == Keys.X)
            {
                if (m_Mgr.Started)
                    while (true)
                    {
                        var flag = false;
                        for (var i = 0; i < m_Mgr.TotalWidth; i++)
                            for (var j = 0; j < m_Mgr.TotalHeight; j++)
                                if (!m_Mgr[i, j].IsOpen &&
                                    m_Mgr.Solver[m_Mgr[i, j]] == BlockStatus.Blank)
                                {
                                    m_Mgr.OpenBlock(i, j);
                                    flag = true;
                                }
                        if (!flag ||
                            !m_Mgr.Started)
                            break;
                        Solve();
                    }
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
                    Text = $"Failed At {m_Total.Log2():F2}bits";
                return;
            }

            m_Total = m_Mgr.Solver.Solve();

            var curBits = m_Total.Log2();

            Text = $"Resume: {curBits:F2}bits";

            progressBar1.Value = (int)(2147483647 * (1 - curBits / m_AllLog2));

            m_Bests = new List<Block>();
            for (var i = 0; i < m_Mgr.TotalWidth; i++)
                for (var j = 0; j < m_Mgr.TotalHeight; j++)
                    if (!m_Mgr[i, j].IsOpen &&
                        m_Mgr.Solver[m_Mgr[i, j]] == BlockStatus.Blank)
                        m_Bests.Add(m_Mgr[i, j]);

            if (m_Bests.Count != 0)
                return;

            var prob = 1D;
            for (var i = 0; i < m_Mgr.TotalWidth; i++)
                for (var j = 0; j < m_Mgr.TotalHeight; j++)
                    if (!m_Mgr[i, j].IsOpen &&
                        m_Mgr.Solver.Probability[m_Mgr[i, j]] < prob &&
                        m_Mgr.Solver[m_Mgr[i, j]] == BlockStatus.Unknown)
                        prob = m_Mgr.Solver.Probability[m_Mgr[i, j]];

            var pOfZero = new Dictionary<Block, double>();
            var quantities = new Dictionary<Block, double>();
            for (var i = 0; i < m_Mgr.TotalWidth; i++)
                for (var j = 0; j < m_Mgr.TotalHeight; j++)
                    if (!m_Mgr[i, j].IsOpen &&
                        m_Mgr.Solver.Probability[m_Mgr[i, j]] <= prob)
                    {
                        var block = m_Mgr[i, j];
                        var theBlock = new BlockSet<Block>(new[] { block });
                        var dic = m_Mgr.Solver.DistributionConditioned(block.Surrounding, theBlock, 0);
                        var total = dic.Aggregate(BigInteger.Zero, (cur, kvp) => cur + kvp.Value);
                        if (total.IsZero)
                            throw new Exception();
                        pOfZero[block] = dic[dic.Keys.Min()].Over(total);
                        var q =
                            dic.Sum(
                                    kvp =>
                                    {
                                        if (kvp.Value == 0)
                                            return 0D;
                                        var p = kvp.Value.Over(total);
                                        return -p * Math.Log(p, 2);
                                    });
                        quantities[block] = q;
                    }

            var maxP = pOfZero.Values.Max();
            var lst = pOfZero.Where(kvp => kvp.Value >= maxP).Select(kvp => kvp.Key).ToList();
            var maxQ = lst.Select(b => quantities[b]).Max();
            m_Bests =
                quantities.Where(kvp => lst.Contains(kvp.Key) && kvp.Value >= maxQ).Select(kvp => kvp.Key).ToList();
        }
    }
}
