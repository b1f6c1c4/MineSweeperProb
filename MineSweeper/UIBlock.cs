using System.ComponentModel;
using System.Drawing;
using System.Windows.Forms;
using MineSweeperCalc;
using MineSweeperCalc.Solver;

namespace MineSweeper
{
    public class UIBlock : Label
    {
        public UIBlock()
        {
            Margin = new Padding(0);
            Name = "UIBlock";
            Size = new Size(24, 24);
        }

        public Block TheBlock { get; set; }

        public GameMgrBuffered TheMgr { get; set; }

        private bool m_Best;
        private readonly SolidBrush m_SolidBrush = new SolidBrush(Color.FromArgb(160, Color.Yellow));

        public static void InvokeIfRequired(ISynchronizeInvoke control, MethodInvoker action)
        {
            if (control.InvokeRequired)
                control.Invoke(action, new object[0]);
            else
                action();
        }

        public void FetchState()
        {
            TheMgr.EnterReadLock();

            string str = null;
            Color color;
            bool best;
            try
            {
                if (TheMgr.Mode.HasFlag(SolvingMode.Half) &&
                    !TheMgr.Succeed &&
                    TheMgr.InferredStatuses != null)
                {
                    switch (TheMgr.InferredStatuses[TheBlock])
                    {
                        case BlockStatus.Mine:
                            color = TheMgr.Started ? Color.Black : Color.Red;
                            str = "M";
                            break;
                        case BlockStatus.Blank:
                            color = Color.White;
                            if (TheBlock.IsOpen)
                            {
                                var deg = TheBlock.Degree;
                                if (deg != 0)
                                    str = deg.ToString();
                            }
                            else
                                str = "B";
                            break;
                        default:
                            if (!TheMgr.Started)
                                if (TheBlock.IsMine)
                                {
                                    color = Color.Red;
                                    if (TheBlock.IsOpen)
                                        str = "X";
                                }
                                else
                                    color = Color.DarkGray;
                            else if (TheMgr.Mode.HasFlag(SolvingMode.Probability) &&
                                     TheMgr.Probability != null)
                            {
                                var v = (int)((1 - TheMgr.Probability[TheBlock]) * 255);
                                color = Color.FromArgb(v, v, v);
                            }
                            else
                                color = Color.DarkGray;
                            break;
                    }

                    best = TheMgr.Bests.BinarySearch(TheBlock) >= 0;
                }
                else
                {
                    if (TheBlock.IsOpen)
                        if (TheBlock.IsMine)
                        {
                            color = Color.Red;
                            str = "X";
                        }
                        else
                        {
                            color = Color.White;
                            var deg = TheBlock.Degree;
                            if (deg != 0)
                                str = deg.ToString();
                        }
                    else if (!TheMgr.Started &&
                             TheBlock.IsMine)
                        color = TheMgr.Succeed ? Color.Green : Color.Red;
                    else
                        color = Color.DarkGray;

                    best = false;
                }
            }
            finally
            {
                TheMgr.ExitReadLock();
            }

            InvokeIfRequired(
                             this,
                             () =>
                             {
                                 Text = str;
                                 BackColor = color;
                                 if (m_Best != best)
                                 {
                                     m_Best = best;
                                     Refresh();
                                 }
                             });
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            base.OnPaint(e);
            if (m_Best)
                e.Graphics.FillEllipse(
                                       m_SolidBrush,
                                       ClientSize.Width / 4,
                                       ClientSize.Height / 4,
                                       ClientSize.Width / 2,
                                       ClientSize.Height / 2);
        }
    }
}
