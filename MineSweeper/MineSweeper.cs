using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Linq;
using System.Runtime.Serialization.Formatters.Binary;
using System.Text;
using System.Windows.Forms;

namespace MineSweeper
{
    public partial class MineSweeper : Form
    {
        private readonly int m_Width;
        private readonly int m_Height;
        private readonly int m_Mines;

        private readonly float m_ScaleFactor;

        private GameMgr m_Mgr;
        private Block m_CurrentBlock;
        private readonly List<UIBlock> m_UIBlocks;

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

            SuspendLayout();
            m_UIBlocks = new List<UIBlock>();
            var font = new Font("Consolas", 11F * m_ScaleFactor);
            for (var i = 0; i < m_Width; i++)
                for (var j = 0; j < m_Height; j++)
                {
                    var ub = new UIBlock
                                 {
                                     Location = new Point(i * 25, j * 25),
                                     Font = font,
                                     X = i,
                                     Y = j
                                 };
                    ub.MouseClick += Block_Click;
                    ub.MouseEnter += Block_Enter;
                    Controls.Add(ub);
                    m_UIBlocks.Add(ub);
                }

            Scale(new SizeF(m_ScaleFactor, m_ScaleFactor));
            ResumeLayout();
        }

        private void MineSweeper_Load(object sender, EventArgs e) { Reset(); }

        private static void InvokeIfRequired(ISynchronizeInvoke control, MethodInvoker action)
        {
            if (control.InvokeRequired)
                control.Invoke(action, new object[0]);
            else
                action();
        }

        private void UpdateAll()
        {
            UpdateText();
            m_Mgr.EnterReadLock();
            try
            {
                foreach (var ub in m_UIBlocks)
                    ub.FetchState();
            }
            finally
            {
                m_Mgr.ExitReadLock();
            }
        }

        private void UpdateText()
        {
            m_Mgr.EnterReadLock();
            try
            {
                var sb = new StringBuilder();
                sb.Append($"[{m_Mgr.Mode}]");
                if (m_Mgr.Mode.HasFlag(SolvingMode.Probability) &&
                    m_Mgr.Probabilities != null &&
                    m_Mgr.Bits >= 0)
                {
                    sb.Append($" {m_Mgr.Bits:F2}/{m_Mgr.AllBits:F2}b, {1 - m_Mgr.Bits / m_Mgr.AllBits:P0}");
                    progressBar1.Value = (int)(2147483647 * (1 - m_Mgr.Bits / m_Mgr.AllBits));
                }
                else
                    sb.Append($" {m_Mgr.AllBits:F2}b");

                if (m_CurrentBlock != null)
                {
                    if (m_Mgr.Mode.HasFlag(SolvingMode.Probability) &&
                        m_Mgr.Probabilities != null)
                        sb.Append($" M{m_Mgr.Probabilities[m_CurrentBlock.Index]:P2}");
                    //if (m_Mgr.Mode.HasFlag(SolvingMode.ZeroProb) &&
                    //    (m_Mgr.DegreeDist != null &&
                    //     m_Mgr.Quantity != null))
                    //{
                    //    sb.Append($"Q{m_Mgr.Quantity[m_CurrentBlock]:F2}b: ");

                    //    var dist = m_Mgr.DegreeDist[m_CurrentBlock];
                    //    if (dist != null)
                    //    {
                    //        foreach (var kvp in dist.OrderBy(kvp => kvp.Key))
                    //            sb.Append($"{kvp.Value:P1}[{kvp.Key}],");
                    //        if (dist.Count > 0)
                    //            sb.Remove(sb.Length - 1, 1);
                    //    }
                    //}
                    if (m_Mgr.Mode.HasFlag(SolvingMode.Drained) &&
                        m_Mgr.DrainProbabilities != null)
                    {
                        sb.Append($" D{m_Mgr.DrainProbabilities[m_CurrentBlock.Index]:P2}");
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

        private void Reset(GameMgr mgr = null)
        {
            if (mgr == null)
            {
                var mode = (m_Mgr?.Mode ?? SolvingMode.ZeroProb) & SolvingMode.ZeroProb;
                m_Mgr = new GameMgr(m_Width, m_Height, m_Mines) { Mode = mode };
            }
            else
                m_Mgr = mgr;

            foreach (var ub in m_UIBlocks)
                ub.TheMgr = m_Mgr;

            ClientSize = new Size(
                (int)(m_Width * 25 * m_ScaleFactor),
                (int)(m_Height * 25 * m_ScaleFactor) + progressBar1.Height);

            m_Mgr.StatusUpdated += () => InvokeIfRequired(this, UpdateAll);
            m_Mgr.FetchStatus();

            m_Mgr.Solve();
            UpdateText();
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
            UpdateText();
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
                                Reset((GameMgr)formatter.Deserialize(stream));
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
                        m_Mgr.SemiAutomaticStep();
                    break;
                case Keys.X:
                    if (m_Mgr.Started)
                        m_Mgr.SemiAutomatic();
                    break;
                case Keys.A:
                    if (m_Mgr.Started &&
                        (m_Mgr.Mode.HasFlag(SolvingMode.ZeroProb) ||
                         m_Mgr.Mode.HasFlag(SolvingMode.Drained)))
                        if (m_Mgr.BestBlocks.Any())
                            m_Mgr.SemiAutomaticStep();
                        else
                            m_Mgr.AutomaticStep();
                    break;
                case Keys.Z:
                    if (m_Mgr.Started &&
                        (m_Mgr.Mode.HasFlag(SolvingMode.ZeroProb) ||
                         m_Mgr.Mode.HasFlag(SolvingMode.Drained)))
                        if (m_Mgr.BestBlocks.Any())
                            m_Mgr.SemiAutomatic();
                        else
                            m_Mgr.AutomaticStep();
                    break;
                case Keys.R:
                    Reset();
                    break;
                case Keys.D0:
                    m_Mgr.Mode = SolvingMode.None;
                    m_Mgr.Solve();
                    break;
                case Keys.D1:
                    m_Mgr.Mode = SolvingMode.Reduce;
                    m_Mgr.Solve();
                    break;
                case Keys.D2:
                    m_Mgr.Mode = SolvingMode.Overlap;
                    m_Mgr.Solve();
                    break;
                case Keys.D3:
                    m_Mgr.Mode = SolvingMode.Probability;
                    m_Mgr.Solve();
                    break;
                case Keys.D4:
                    m_Mgr.Mode = SolvingMode.ZeroProb;
                    m_Mgr.Solve();
                    break;
                case Keys.D5:
                    m_Mgr.Mode = SolvingMode.Drained;
                    break;
                case Keys.C:
                    m_Mgr.Cancel();
                    break;
            }
            UpdateText();
        }
    }
}
