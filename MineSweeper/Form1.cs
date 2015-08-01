using System.Drawing;
using System.Linq;
using System.Runtime.InteropServices;
using System.Windows.Forms;
using MineSweeperCalc;

namespace MineSweeper
{
    public partial class Form1 : Form
    {
        private bool m_Started;
        private bool m_Solved;
        private BlockMgr m_Mgr;
        private Solver<Block> m_Solver;
        private bool m_ShowProb;
        private const float BlockSize = 38F;

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
                                         new Font("Consolas", m_Mgr[i, j].IsOpen ? 24 : 12),
                                         new SolidBrush(Color.White),
                                         i * BlockSize,
                                         j * BlockSize);
                        else if (m_Solver[m_Mgr[i, j]] == BlockStatus.Blank)
                            g.DrawString(
                                         "B",
                                         new Font("Consolas", m_Mgr[i, j].IsOpen ? 24 : 12),
                                         new SolidBrush(Color.Black),
                                         i * BlockSize,
                                         j * BlockSize);
                    }
            else
                for (var i = 0; i < m_Mgr.TotalWidth; i++)
                    for (var j = 0; j < m_Mgr.TotalHeight; j++)
                    {
                        if (!m_Started)
                            if (m_Mgr[i, j].IsMine)
                                g.FillRectangle(
                                                new SolidBrush(Color.Red),
                                                i * BlockSize,
                                                j * BlockSize,
                                                BlockSize - 1,
                                                BlockSize - 1);
                        if (m_Mgr[i, j].IsOpen)
                        {
                            if (!m_Mgr[i, j].IsMine)
                            {
                                g.FillRectangle(
                                                new SolidBrush(Color.White),
                                                i * BlockSize,
                                                j * BlockSize,
                                                BlockSize - 1,
                                                BlockSize - 1);
                                g.DrawString(
                                             m_Mgr[i, j].Degree.ToString(),
                                             new Font("Consolas", 24),
                                             new SolidBrush(Color.Black),
                                             i * BlockSize,
                                             j * BlockSize);
                            }
                        }
                        else if (m_Started)
                            g.FillRectangle(
                                            new SolidBrush(Color.LightGray),
                                            i * BlockSize,
                                            j * BlockSize,
                                            BlockSize - 1,
                                            BlockSize - 1);
                    }
        }

        private void Reset()
        {
            m_Mgr = new BlockMgr(30, 16, 99);
            m_Solver = new Solver<Block>(
                m_Mgr.Blocks,
                new BinomialHelper(m_Mgr.TotalWidth * m_Mgr.TotalHeight, m_Mgr.TotalMines));
            m_Solver.AddRestrain(new Restrain<Block>(m_Mgr.TotalMines, new BlockSet<Block>(m_Mgr.Blocks)));
            m_Started = true;
            m_ShowProb = false;
            Width = (int)(m_Mgr.TotalWidth * BlockSize) + 22;
            Height = (int)(m_Mgr.TotalHeight * BlockSize) + 56;
        }

        private void Open(int x, int y)
        {
            var block = m_Mgr.OpenBlock(x, y);
            if (block.IsMine)
            {
                m_Started = false;
                return;
            }

            m_Solver.AddRestrain(new Restrain<Block>(0, new BlockSet<Block>(new[] { block })));

            var deg = block.Degree;
            if (deg == 0)
                foreach (var b in block.Surrounding.Cast<Block>().Where(b => !b.IsOpen))
                    Open(b.X, b.Y);
            else
                m_Solver.AddRestrain(new Restrain<Block>(deg, block.Surrounding));

            m_Solved = false;
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
            RePaint();
        }

        private void Form1_KeyUp(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Space)
            {
                m_ShowProb ^= true;
                if (m_ShowProb && !m_Solved)
                {
                    m_Solver.Solve(0);
                    m_Solved = true;
                }
                RePaint();
            }
        }
    }
}
