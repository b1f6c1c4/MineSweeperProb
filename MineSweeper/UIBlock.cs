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

        public int X { get; set; }
        public int Y { get; set; }

        public Block TheBlock => TheMgr[X, Y];

        public GameMgrBuffered TheMgr { get; set; }

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
            Color color, fColor = Color.Black;
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
                            if (TheMgr.Started)
                            { color = Color.Black;
                                fColor = Color.White;
                            }
                            else
                                color = Color.Red;
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

                    if (TheMgr.BestsForSure.BinarySearch(TheBlock) >= 0)
                    {
                        str = "★";
                        fColor = Color.MediumSlateBlue;
                    }
                    else if (TheMgr.Mode.HasFlag(SolvingMode.Probability) && 
                            TheMgr.Bests != null &&
                             TheMgr.Bests.BinarySearch(TheBlock) >= 0)
                    {
                        str = "☆";
                        fColor = Color.SlateBlue;
                    }
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
                                 ForeColor = fColor;
                             });
        }
    }
}
