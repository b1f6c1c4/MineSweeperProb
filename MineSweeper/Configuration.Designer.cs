namespace MineSweeper
{
    partial class Configuration
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.txtWidth = new System.Windows.Forms.TextBox();
            this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
            this.radSuper = new System.Windows.Forms.RadioButton();
            this.radLarge = new System.Windows.Forms.RadioButton();
            this.radMiddle = new System.Windows.Forms.RadioButton();
            this.radSmall = new System.Windows.Forms.RadioButton();
            this.tableLayoutPanel2 = new System.Windows.Forms.TableLayoutPanel();
            this.txtMines = new System.Windows.Forms.TextBox();
            this.label3 = new System.Windows.Forms.Label();
            this.txtHeight = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.btnStart = new System.Windows.Forms.Button();
            this.tableLayoutPanel1.SuspendLayout();
            this.tableLayoutPanel2.SuspendLayout();
            this.SuspendLayout();
            // 
            // txtWidth
            // 
            this.txtWidth.Dock = System.Windows.Forms.DockStyle.Fill;
            this.txtWidth.Font = new System.Drawing.Font("微软雅黑", 14F);
            this.txtWidth.Location = new System.Drawing.Point(53, 3);
            this.txtWidth.MaxLength = 4;
            this.txtWidth.Multiline = true;
            this.txtWidth.Name = "txtWidth";
            this.txtWidth.Size = new System.Drawing.Size(64, 28);
            this.txtWidth.TabIndex = 4;
            this.txtWidth.Validating += new System.ComponentModel.CancelEventHandler(this.TextBox_Validating);
            // 
            // tableLayoutPanel1
            // 
            this.tableLayoutPanel1.ColumnCount = 1;
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 20F));
            this.tableLayoutPanel1.Controls.Add(this.radSuper, 0, 3);
            this.tableLayoutPanel1.Controls.Add(this.radLarge, 0, 2);
            this.tableLayoutPanel1.Controls.Add(this.radMiddle, 0, 1);
            this.tableLayoutPanel1.Controls.Add(this.radSmall, 0, 0);
            this.tableLayoutPanel1.Location = new System.Drawing.Point(12, 12);
            this.tableLayoutPanel1.Name = "tableLayoutPanel1";
            this.tableLayoutPanel1.RowCount = 4;
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 25F));
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 25F));
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 25F));
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 25F));
            this.tableLayoutPanel1.Size = new System.Drawing.Size(62, 162);
            this.tableLayoutPanel1.TabIndex = 1;
            // 
            // radSuper
            // 
            this.radSuper.Appearance = System.Windows.Forms.Appearance.Button;
            this.radSuper.AutoSize = true;
            this.radSuper.Dock = System.Windows.Forms.DockStyle.Fill;
            this.radSuper.Location = new System.Drawing.Point(3, 123);
            this.radSuper.Name = "radSuper";
            this.radSuper.Size = new System.Drawing.Size(56, 36);
            this.radSuper.TabIndex = 3;
            this.radSuper.TabStop = true;
            this.radSuper.Text = "究极";
            this.radSuper.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            this.radSuper.UseVisualStyleBackColor = true;
            this.radSuper.CheckedChanged += new System.EventHandler(this.RadioButton_CheckedChanged);
            // 
            // radLarge
            // 
            this.radLarge.Appearance = System.Windows.Forms.Appearance.Button;
            this.radLarge.AutoSize = true;
            this.radLarge.Dock = System.Windows.Forms.DockStyle.Fill;
            this.radLarge.Location = new System.Drawing.Point(3, 83);
            this.radLarge.Name = "radLarge";
            this.radLarge.Size = new System.Drawing.Size(56, 34);
            this.radLarge.TabIndex = 2;
            this.radLarge.Text = "高级";
            this.radLarge.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            this.radLarge.UseVisualStyleBackColor = true;
            this.radLarge.CheckedChanged += new System.EventHandler(this.RadioButton_CheckedChanged);
            // 
            // radMiddle
            // 
            this.radMiddle.Appearance = System.Windows.Forms.Appearance.Button;
            this.radMiddle.AutoSize = true;
            this.radMiddle.Dock = System.Windows.Forms.DockStyle.Fill;
            this.radMiddle.Location = new System.Drawing.Point(3, 43);
            this.radMiddle.Name = "radMiddle";
            this.radMiddle.Size = new System.Drawing.Size(56, 34);
            this.radMiddle.TabIndex = 1;
            this.radMiddle.TabStop = true;
            this.radMiddle.Text = "中级";
            this.radMiddle.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            this.radMiddle.UseVisualStyleBackColor = true;
            this.radMiddle.CheckedChanged += new System.EventHandler(this.RadioButton_CheckedChanged);
            // 
            // radSmall
            // 
            this.radSmall.Appearance = System.Windows.Forms.Appearance.Button;
            this.radSmall.AutoSize = true;
            this.radSmall.Dock = System.Windows.Forms.DockStyle.Fill;
            this.radSmall.Location = new System.Drawing.Point(3, 3);
            this.radSmall.Name = "radSmall";
            this.radSmall.Size = new System.Drawing.Size(56, 34);
            this.radSmall.TabIndex = 0;
            this.radSmall.TabStop = true;
            this.radSmall.Text = "初级";
            this.radSmall.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            this.radSmall.UseVisualStyleBackColor = true;
            this.radSmall.CheckedChanged += new System.EventHandler(this.RadioButton_CheckedChanged);
            // 
            // tableLayoutPanel2
            // 
            this.tableLayoutPanel2.ColumnCount = 6;
            this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 50F));
            this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 33.33334F));
            this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 50F));
            this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 33.33333F));
            this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 50F));
            this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 33.33333F));
            this.tableLayoutPanel2.Controls.Add(this.txtMines, 5, 0);
            this.tableLayoutPanel2.Controls.Add(this.label3, 4, 0);
            this.tableLayoutPanel2.Controls.Add(this.txtHeight, 3, 0);
            this.tableLayoutPanel2.Controls.Add(this.label2, 2, 0);
            this.tableLayoutPanel2.Controls.Add(this.label1, 0, 0);
            this.tableLayoutPanel2.Controls.Add(this.txtWidth, 1, 0);
            this.tableLayoutPanel2.Location = new System.Drawing.Point(100, 12);
            this.tableLayoutPanel2.Name = "tableLayoutPanel2";
            this.tableLayoutPanel2.RowCount = 1;
            this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.tableLayoutPanel2.Size = new System.Drawing.Size(361, 34);
            this.tableLayoutPanel2.TabIndex = 2;
            // 
            // txtMines
            // 
            this.txtMines.Dock = System.Windows.Forms.DockStyle.Fill;
            this.txtMines.Font = new System.Drawing.Font("微软雅黑", 14F);
            this.txtMines.Location = new System.Drawing.Point(293, 3);
            this.txtMines.MaxLength = 4;
            this.txtMines.Multiline = true;
            this.txtMines.Name = "txtMines";
            this.txtMines.Size = new System.Drawing.Size(65, 28);
            this.txtMines.TabIndex = 6;
            this.txtMines.Validating += new System.ComponentModel.CancelEventHandler(this.TextBox_Validating);
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Dock = System.Windows.Forms.DockStyle.Fill;
            this.label3.Location = new System.Drawing.Point(243, 0);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(44, 34);
            this.label3.TabIndex = 3;
            this.label3.Text = "雷数：";
            this.label3.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // txtHeight
            // 
            this.txtHeight.Dock = System.Windows.Forms.DockStyle.Fill;
            this.txtHeight.Font = new System.Drawing.Font("微软雅黑", 14F);
            this.txtHeight.Location = new System.Drawing.Point(173, 3);
            this.txtHeight.MaxLength = 4;
            this.txtHeight.Multiline = true;
            this.txtHeight.Name = "txtHeight";
            this.txtHeight.Size = new System.Drawing.Size(64, 28);
            this.txtHeight.TabIndex = 5;
            this.txtHeight.Validating += new System.ComponentModel.CancelEventHandler(this.TextBox_Validating);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Dock = System.Windows.Forms.DockStyle.Fill;
            this.label2.Location = new System.Drawing.Point(123, 0);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(44, 34);
            this.label2.TabIndex = 1;
            this.label2.Text = "列数：";
            this.label2.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.label1.Location = new System.Drawing.Point(3, 0);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(44, 34);
            this.label1.TabIndex = 0;
            this.label1.Text = "行数：";
            this.label1.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // btnStart
            // 
            this.btnStart.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.btnStart.Location = new System.Drawing.Point(100, 59);
            this.btnStart.Name = "btnStart";
            this.btnStart.Size = new System.Drawing.Size(361, 115);
            this.btnStart.TabIndex = 7;
            this.btnStart.Text = "启动";
            this.btnStart.UseVisualStyleBackColor = true;
            this.btnStart.Click += new System.EventHandler(this.btnStart_Click);
            // 
            // Configuration
            // 
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.None;
            this.ClientSize = new System.Drawing.Size(473, 186);
            this.Controls.Add(this.btnStart);
            this.Controls.Add(this.tableLayoutPanel2);
            this.Controls.Add(this.tableLayoutPanel1);
            this.Font = new System.Drawing.Font("微软雅黑", 6F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
            this.KeyPreview = true;
            this.Margin = new System.Windows.Forms.Padding(2, 3, 2, 3);
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "Configuration";
            this.ShowIcon = false;
            this.Text = "MineSweeper";
            this.KeyUp += new System.Windows.Forms.KeyEventHandler(this.Configuration_KeyUp);
            this.tableLayoutPanel1.ResumeLayout(false);
            this.tableLayoutPanel1.PerformLayout();
            this.tableLayoutPanel2.ResumeLayout(false);
            this.tableLayoutPanel2.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.TextBox txtWidth;
        private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
        private System.Windows.Forms.RadioButton radSuper;
        private System.Windows.Forms.RadioButton radLarge;
        private System.Windows.Forms.RadioButton radMiddle;
        private System.Windows.Forms.RadioButton radSmall;
        private System.Windows.Forms.TableLayoutPanel tableLayoutPanel2;
        private System.Windows.Forms.TextBox txtMines;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TextBox txtHeight;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button btnStart;
    }
}