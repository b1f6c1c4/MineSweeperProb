using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Runtime.Serialization.Formatters.Binary;
using System.Text;
using System.Windows.Forms;
using MineSweeperCalc;
using MineSweeperCalc.Solver;

namespace MineSweeper
{
    public partial class MineSweeper : Form
    {
        private readonly int m_Width;
        private readonly int m_Height;
        private readonly int m_Mines;

        private readonly float m_ScaleFactor;

        private GameMgrBuffered m_Mgr;
        private double m_AllLog2;
        private Block m_CurrentBlock;
        private readonly List<UIBlock> m_UIBlocks;

        private delegate void UpdateDelegate();

        public MineSweeper(int width, int height, int mines)
        {
            m_Width = width;
            m_Height = height;
            m_Mines = mines;

            var screen = Screen.FromControl(this);
            var f = Math.Min(
                             screen.Bounds.Width / (1.1F * m_Width * 25),
                             screen.Bounds.Height / (1.1F * m_Height * 25));
            m_ScaleFactor = f < Program.ScaleFactor ? f : Program.ScaleFactor;

            InitializeComponent();

            m_UIBlocks = new List<UIBlock>();
            for (var i = 0; i < m_Width; i++)
                for (var j = 0; j < m_Height; j++)
                {
                    var ub = new UIBlock
                                 {
                                     Location = new Point(i * 25, j * 25),
                                     Font = new Font("Consolas", 11F * m_ScaleFactor),
                                     X = i,
                                     Y = j
                                 };
                    ub.MouseClick += Block_Click;
                    ub.MouseEnter += Block_Enter;
                    Controls.Add(ub);
                    m_UIBlocks.Add(ub);
                }

            Scale(new SizeF(m_ScaleFactor, m_ScaleFactor));
        }

        private void MineSweeper_Load(object sender, EventArgs e) { Reset(); }

        private void UpdateAll()
        {
            UpdateText();
            foreach (var ub in m_UIBlocks)
                ub.FetchState();
        }

        private void UpdateText()
        {
            m_Mgr.EnterReadLock();
            try
            {
                var sb = new StringBuilder();
                sb.Append($"[{m_Mgr.Mode}]");
                if (m_Mgr.Mode.HasFlag(SolvingMode.Probability) &&
                    m_Mgr.Probability != null)
                {
                    var curBits = m_Mgr.TotalStates.Log2();
                    sb.Append($" {curBits:F2}/{m_AllLog2:F2}b, {1 - curBits / m_AllLog2:P0}");
                    progressBar1.Value = (int)(2147483647 * (1 - curBits / m_AllLog2));
                }
                else
                    sb.Append($" {m_AllLog2:F2}b");

                if (m_CurrentBlock != null)
                    if (m_Mgr.Mode.HasFlag(SolvingMode.Probability) &&
                        m_Mgr.Probability != null)
                    {
                        sb.Append($" M{m_Mgr.Probability[m_CurrentBlock]:P2}");

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
        }

        private void Reset(GameMgrBuffered mgr = null)
        {
            if (mgr == null)
            {
                var mode = m_Mgr?.Mode ?? SolvingMode.Probability;
                m_Mgr = new GameMgrBuffered(
                    m_Width,
                    m_Height,
                    m_Mines,
                    (int)DateTime.Now.Ticks,
                    Strategies.MinProbMaxZeroProbMaxQuantity)
                            { Mode = mode };
            }
            else
                m_Mgr = mgr;

            foreach (var ub in m_UIBlocks)
                ub.TheMgr = m_Mgr;

            var blocks = m_Width * m_Height;
            BinomialHelper.UpdateTo(blocks, m_Mgr.TotalMines);

            m_AllLog2 = BinomialHelper.Binomial(blocks, m_Mgr.TotalMines).Log2();

            ClientSize = new Size(
                (int)(m_Width * 25 * m_ScaleFactor),
                (int)(m_Height * 25 * m_ScaleFactor) + progressBar1.Height);

            Solve();
        }

        private void Block_Enter(object sender, EventArgs e)
        {
            m_CurrentBlock = ((UIBlock)sender).TheBlock;
            UpdateText();
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

            m_Mgr.OpenBlock(block.X, block.Y);
            Solve();
        }

        private void Form1_KeyUp(object sender, KeyEventArgs e)
        {
            switch (e.KeyCode)
            {
                case Keys.O:
                    if (e.Control)
                    {
                        var dialog = new OpenFileDialog
                                         {
                                             AddExtension = true,
                                             CheckPathExists = true,
                                             DefaultExt = "bin",
                                             CheckFileExists = true,
                                             Multiselect = false,
                                             ShowReadOnly = false,
                                             Filter = "扫雷文件(*.bin)|*.bin|所有文件|*"
                                         };
                        if (dialog.ShowDialog() == DialogResult.OK)
                            using (var stream = dialog.OpenFile())
                            {
                                var formatter = new BinaryFormatter();
                                Reset((GameMgrBuffered)formatter.Deserialize(stream));
                            }
                    }
                    break;
                case Keys.S:
                    if (e.Control)
                    {
                        var dialog = new SaveFileDialog
                                         {
                                             OverwritePrompt = true,
                                             AddExtension = true,
                                             CheckPathExists = true,
                                             DefaultExt = "bin",
                                             Filter = "扫雷文件(*.bin)|*.bin|所有文件|*"
                                         };
                        if (dialog.ShowDialog() == DialogResult.OK)
                            using (var stream = dialog.OpenFile())
                            {
                                var formatter = new BinaryFormatter();
                                formatter.Serialize(stream, m_Mgr);
                                stream.Flush();
                            }
                    }
                    else if (m_Mgr.Started)
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
                case Keys.D:
                    var dr = new Drainer();
                    dr.Drain(m_Mgr);
                    break;
            }
        }

        private void Solve()
        {
            m_Mgr.Solve().ContinueWith(t => Invoke(new UpdateDelegate(UpdateAll), new object[] { }));
            UpdateAll();
        }
    }
}
