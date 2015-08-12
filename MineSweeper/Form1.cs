using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Numerics;
using System.Runtime.InteropServices;
using System.Windows.Forms;
using MineSweeperCalc;

namespace MineSweeper
{
    public partial class Form1 : Form
    {
        private bool m_Started;
        private BlockMgr m_Mgr;
        private Solver<Block> m_Solver;
        private bool m_ShowProb;
        private const float BlockSize = 38F;
        private double m_Sig;
        private int m_Exp;
        private int m_ToOpen;
        private bool m_Suceed;
        private Dictionary<Block, double> m_Quantities;
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
            if (m_Started && m_ShowProb)
                for (var i = 0; i < m_Mgr.TotalWidth; i++)
                    for (var j = 0; j < m_Mgr.TotalHeight; j++)
                    {
                        var v = (int)((1 - m_Solver.Probability[m_Mgr[i, j]]) * 255);
                        g.FillRectangle(
                                        new SolidBrush(Color.FromArgb(v, v, v)),
                                        i * BlockSize,
                                        j * BlockSize,
                                        BlockSize - 1,
                                        BlockSize - 1);
                        if (m_Solver[m_Mgr[i, j]] == BlockStatus.Mine)
                            g.DrawString(
                                         "M",
                                         m_Mgr[i, j].IsOpen ? fontO : fontC,
                                         white,
                                         i * BlockSize,
                                         j * BlockSize);
                        else if (m_Solver[m_Mgr[i, j]] == BlockStatus.Blank)
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
                        if (!m_Started)
                            if (m_Mgr[i, j].IsMine)
                                g.FillRectangle(
                                                m_Suceed ? green : red,
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
                        else if (m_Started)
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
            m_Mgr = new BlockMgr(30, 16, 99);

            var blocks = m_Mgr.TotalWidth * m_Mgr.TotalHeight;
            var helper = new BinomialHelper(blocks, m_Mgr.TotalMines);
            BigIntegerHelper.Part(helper.Binomial(blocks, m_Mgr.TotalMines), out m_Sig, out m_Exp);

            m_ToOpen = blocks - m_Mgr.TotalMines;

            m_Solver = new Solver<Block>(m_Mgr.Blocks, helper);
            m_Solver.AddRestrain(new BlockSet<Block>(m_Mgr.Blocks), m_Mgr.TotalMines);

            m_Started = true;
            m_Suceed = false;
            m_ShowProb = false;

            Width = (int)(m_Mgr.TotalWidth * BlockSize) + 22;
            Height = (int)(m_Mgr.TotalHeight * BlockSize) + 56 + progressBar1.Height;
        }

        private void Open(int x, int y)
        {
            OpenBlock(x, y);
            if (m_ToOpen == 0)
            {
                m_Started = false;
                m_Suceed = true;
            }
            Solve();
        }

        private void OpenBlock(int x, int y)
        {
            var block = m_Mgr.OpenBlock(x, y);
            if (block.IsMine)
            {
                m_Started = false;
                return;
            }

            m_ToOpen--;

            m_Solver.AddRestrain(new BlockSet<Block>(new[] { block }), 0);

            var deg = block.Degree;
            if (deg == 0)
                foreach (var b in block.Surrounding.Cast<Block>().Where(b => !b.IsOpen))
                    OpenBlock(b.X, b.Y);
            else
                m_Solver.AddRestrain(block.Surrounding, deg);
        }

        private void Form1_Paint(object sender, PaintEventArgs e) => RePaint(e.Graphics);

        private void Form1_MouseUp(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Left)
                if (m_Started)
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
                    Open(x, y);
                }
                else
                    Reset();
            else if (e.Button == MouseButtons.Right)
                if (m_Started)
                {
                    var flag = false;
                    for (var i = 0; i < m_Mgr.TotalWidth; i++)
                        for (var j = 0; j < m_Mgr.TotalHeight; j++)
                            if (!m_Mgr[i, j].IsOpen &&
                                m_Solver[m_Mgr[i, j]] == BlockStatus.Blank)
                            {
                                OpenBlock(i, j);
                                flag = true;
                            }
                    if (flag)
                    {
                        if (m_ToOpen == 0)
                        {
                            m_Started = false;
                            m_Suceed = true;
                        }
                        Solve();
                    }
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
                if (m_Started)
                {
                    var flag = true;
                    while (flag)
                    {
                        flag = false;
                        for (var i = 0; i < m_Mgr.TotalWidth; i++)
                            for (var j = 0; j < m_Mgr.TotalHeight; j++)
                                if (!m_Mgr[i, j].IsOpen &&
                                    m_Solver[m_Mgr[i, j]] == BlockStatus.Blank)
                                {
                                    OpenBlock(i, j);
                                    flag = true;
                                }
                        if (flag)
                        {
                            if (m_ToOpen == 0)
                            {
                                m_Started = false;
                                m_Suceed = true;
                            }
                            Solve();
                        }
                    }
                }
                RePaint();
            }
        }

        private void Solve()
        {
            m_Total = m_Solver.Solve();

            double sig;
            int exp;
            BigIntegerHelper.Part(m_Total, out sig, out exp);

            var cur = Math.Log(sig, 2) + exp;
            var tot = Math.Log(m_Sig, 2) + m_Exp;

            Text = $"Resume:{cur:F2}bits";

            progressBar1.Value = (int)(2147483647 * (1 - cur / tot));

            m_Bests = new List<Block>();
            for (var i = 0; i < m_Mgr.TotalWidth; i++)
                for (var j = 0; j < m_Mgr.TotalHeight; j++)
                    if (!m_Mgr[i, j].IsOpen &&
                        m_Solver[m_Mgr[i, j]] == BlockStatus.Blank)
                        m_Bests.Add(m_Mgr[i, j]);

            if (m_Bests.Count != 0)
                return;

            var prob = 1D;
            for (var i = 0; i < m_Mgr.TotalWidth; i++)
                for (var j = 0; j < m_Mgr.TotalHeight; j++)
                    if (!m_Mgr[i, j].IsOpen &&
                        m_Solver.Probability[m_Mgr[i, j]] < prob &&
                        m_Solver[m_Mgr[i, j]] == BlockStatus.Unknown)
                        prob = m_Solver.Probability[m_Mgr[i, j]];

            m_Quantities = new Dictionary<Block, double>();
            for (var i = 0; i < m_Mgr.TotalWidth; i++)
                for (var j = 0; j < m_Mgr.TotalHeight; j++)
                    if (!m_Mgr[i, j].IsOpen &&
                        m_Solver.Probability[m_Mgr[i, j]] <= prob)
                        CalcQuantity(m_Mgr[i, j]);

            var maxQ = m_Quantities.Values.Max();
            m_Bests = m_Quantities.Where(kvp => kvp.Value >= maxQ).Select(kvp => kvp.Key).ToList();
        }

        private void CalcQuantity(Block block)
        {
            var theBlock = new BlockSet<Block>(new[] { block });
            var dic = m_Solver.DistributionConditioned(block.Surrounding, theBlock, 0);
            var total = dic.Aggregate(BigInteger.Zero, (cur, kvp) => cur + kvp.Value);
            var q = dic.Sum(
                            kvp =>
                            -BigIntegerHelper.Ratio(kvp.Value, total)
                            * Math.Log(BigIntegerHelper.Ratio(kvp.Value, total), 2));
            m_Quantities[block] = q;
        }
    }
}
