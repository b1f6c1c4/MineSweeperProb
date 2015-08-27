using System;
using System.ComponentModel;
using System.Drawing;
using System.Windows.Forms;

namespace MineSweeper
{
    public partial class Configuration : Form
    {
        public Configuration()
        {
            InitializeComponent();

            Scale(new SizeF(Program.ScaleFactor, Program.ScaleFactor));

            foreach (var control in Controls)
            {
                var c = control as Control;
                if (c == null)
                    continue;
                c.Font = new Font(
                    c.Font.FontFamily,
                    c.Font.Size * Program.ScaleFactor,
                    FontStyle.Regular,
                    GraphicsUnit.Point);
            }

            radLarge.Checked = true;
        }

        private void RadioButton_CheckedChanged(object sender, EventArgs e)
        {
            if (radSmall.Checked)
            {
                txtWidth.Text = 8.ToString();
                txtHeight.Text = 8.ToString();
                txtMines.Text = 10.ToString();
            }
            else if (radMiddle.Checked)
            {
                txtWidth.Text = 16.ToString();
                txtHeight.Text = 16.ToString();
                txtMines.Text = 40.ToString();
            }
            else if (radLarge.Checked)
            {
                txtWidth.Text = 30.ToString();
                txtHeight.Text = 16.ToString();
                txtMines.Text = 99.ToString();
            }
            else if (radSuper.Checked)
            {
                txtWidth.Text = 83.ToString();
                txtHeight.Text = 40.ToString();
                txtMines.Text = 720.ToString();
            }
        }

        private void TextBox_Validating(object sender, CancelEventArgs e)
        {
            int v;
            if (!int.TryParse(((TextBox)sender).Text, out v))
                e.Cancel = true;
            else if (v <= 0)
                e.Cancel = true;
            else
            {
                radSmall.Checked = false;
                radMiddle.Checked = false;
                radLarge.Checked = false;
                radSuper.Checked = false;
            }
        }

        private void btnStart_Click(object sender, EventArgs e)
        {
            var frm = new MineSweeper(
                Convert.ToInt32(txtWidth.Text),
                Convert.ToInt32(txtHeight.Text),
                Convert.ToInt32(txtMines.Text));
            frm.Show();
        }
    }
}
