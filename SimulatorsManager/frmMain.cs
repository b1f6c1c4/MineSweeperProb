using System.Windows.Forms;

namespace SimulatorsManager
{
    public partial class frmMain : Form
    {
        private readonly SimulatorsManager m_SimulatorsManager;

        public frmMain()
        {
            InitializeComponent();

            m_SimulatorsManager = new SimulatorsManager();

            dataGridView1.AutoGenerateColumns = false;
            dataGridView1.DataSource = m_SimulatorsManager.Binding;
            dataGridView1.Columns[4].DefaultCellStyle.WrapMode = DataGridViewTriState.True;
        }

        protected override bool ProcessDialogKey(Keys keyData)
        {
            if (textBox1.Focused)
                if (keyData == Keys.Enter)
                    return true;
            return false;
        }

        private void textBox1_KeyUp(object sender, KeyEventArgs e)
        {
            if (e.KeyCode != Keys.Enter)
                return;

            var cmd = textBox1.Text;
            if (m_SimulatorsManager.ProcessCommand(cmd))
                textBox1.Clear();
        }
    }
}
