using System.ComponentModel;
using System.Drawing;
using System.Globalization;
using System.Windows.Forms;

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

        public GameMgr TheMgr { get; set; }

        public void FetchState()
        {

            var str = string.Empty;
            Color color, fColor = Color.Black;
                if (TheMgr.Mode.HasFlag(SolvingMode.Half) &&
                    !TheMgr.Succeed &&
                    TheMgr.InferredStatuses != null)
                {
                    switch (TheMgr.InferredStatuses[TheBlock.Index])
                    {
                        case BlockStatus.Mine:
                            if (TheMgr.Started)
                            {
                                color = Color.Black;
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
                            //else if (TheMgr.DrainProbability != null)
                            //{
                            //    var v = (int)(TheMgr.DrainProbability[TheBlock] * 255);
                            //    color = Color.FromArgb(v, v, v);
                            //}
                            else if (TheMgr.Mode.HasFlag(SolvingMode.Probability) &&
                                     TheMgr.Probabilities != null)
                            {
                                var v = (int)((1 - TheMgr.Probabilities[TheBlock.Index]) * 255);
                                color = Color.FromArgb(v, v, v);
                            }
                            else
                                color = Color.DarkGray;
                            break;
                    }

                    if (TheMgr.BestBlocks != null &&
                        TheMgr.BestBlocks.BinarySearch(TheBlock) >= 0)
                    {
                        str = "★";
                        fColor = Color.MediumSlateBlue;
                    }
                    else if (TheMgr.Mode.HasFlag(SolvingMode.Probability) &&
                             TheMgr.PreferredBlocks != null &&
                             TheMgr.PreferredBlocks.BinarySearch(TheBlock) >= 0)
                    {
                        str = "☆";
                        fColor = Color.SlateBlue;
                    }
                }
                else if (TheBlock.IsOpen)
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
                            str = deg.ToString(CultureInfo.InvariantCulture);
                    }
                else if (!TheMgr.Started &&
                         TheBlock.IsMine)
                    color = TheMgr.Succeed ? Color.Green : Color.Red;
                else
                    color = Color.DarkGray;
            
            Text = str;
            BackColor = color;
            ForeColor = fColor;
        }
    }
}
