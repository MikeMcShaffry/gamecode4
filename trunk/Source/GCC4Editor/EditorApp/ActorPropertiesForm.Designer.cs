namespace EditorApp
{
    partial class ActorPropertiesForm
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
            this.m_actorIdTextbox = new System.Windows.Forms.TextBox();
            this.m_actorTypeTextbox = new System.Windows.Forms.TextBox();
            this.m_actorXTextbox = new System.Windows.Forms.TextBox();
            this.m_actorYTextbox = new System.Windows.Forms.TextBox();
            this.m_actorZTextbox = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.m_updateActorPosBtn = new System.Windows.Forms.Button();
            this.m_restoreActorPosBtn = new System.Windows.Forms.Button();
            this.m_treeView = new System.Windows.Forms.TreeView();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.label6 = new System.Windows.Forms.Label();
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.m_rotateActorYAxisDegreesTextbox = new System.Windows.Forms.TextBox();
            this.m_rotateActorXAxisDegreesTextbox = new System.Windows.Forms.TextBox();
            this.m_rotateActorXAxisBtn = new System.Windows.Forms.Button();
            this.m_rotateActorYAxisBtn = new System.Windows.Forms.Button();
            this.m_colorPreviewPanel = new System.Windows.Forms.Panel();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.m_removeActorBtn = new System.Windows.Forms.Button();
            this.m_addActorBtn = new System.Windows.Forms.Button();
            this.MenuStrip = new System.Windows.Forms.MenuStrip();
            this.m_fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.saveLevelToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.openLevelToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.SaveFileDialog = new System.Windows.Forms.SaveFileDialog();
            this.OpenFileDialog = new System.Windows.Forms.OpenFileDialog();
            this.groupBox1.SuspendLayout();
            this.groupBox3.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.MenuStrip.SuspendLayout();
            this.SuspendLayout();
            // 
            // m_actorIdTextbox
            // 
            this.m_actorIdTextbox.Enabled = false;
            this.m_actorIdTextbox.Location = new System.Drawing.Point(84, 28);
            this.m_actorIdTextbox.Name = "m_actorIdTextbox";
            this.m_actorIdTextbox.Size = new System.Drawing.Size(100, 20);
            this.m_actorIdTextbox.TabIndex = 0;
            // 
            // m_actorTypeTextbox
            // 
            this.m_actorTypeTextbox.Enabled = false;
            this.m_actorTypeTextbox.Location = new System.Drawing.Point(84, 54);
            this.m_actorTypeTextbox.Name = "m_actorTypeTextbox";
            this.m_actorTypeTextbox.Size = new System.Drawing.Size(100, 20);
            this.m_actorTypeTextbox.TabIndex = 1;
            // 
            // m_actorXTextbox
            // 
            this.m_actorXTextbox.Location = new System.Drawing.Point(37, 113);
            this.m_actorXTextbox.Name = "m_actorXTextbox";
            this.m_actorXTextbox.Size = new System.Drawing.Size(64, 20);
            this.m_actorXTextbox.TabIndex = 2;
            // 
            // m_actorYTextbox
            // 
            this.m_actorYTextbox.Location = new System.Drawing.Point(37, 139);
            this.m_actorYTextbox.Name = "m_actorYTextbox";
            this.m_actorYTextbox.Size = new System.Drawing.Size(64, 20);
            this.m_actorYTextbox.TabIndex = 3;
            // 
            // m_actorZTextbox
            // 
            this.m_actorZTextbox.Location = new System.Drawing.Point(37, 165);
            this.m_actorZTextbox.Name = "m_actorZTextbox";
            this.m_actorZTextbox.Size = new System.Drawing.Size(64, 20);
            this.m_actorZTextbox.TabIndex = 4;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(34, 28);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(44, 13);
            this.label1.TabIndex = 5;
            this.label1.Text = "Actor Id";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(19, 54);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(59, 13);
            this.label2.TabIndex = 6;
            this.label2.Text = "Actor Type";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(17, 113);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(14, 13);
            this.label3.TabIndex = 7;
            this.label3.Text = "X";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(17, 139);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(14, 13);
            this.label4.TabIndex = 8;
            this.label4.Text = "Y";
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(17, 165);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(14, 13);
            this.label5.TabIndex = 9;
            this.label5.Text = "Z";
            // 
            // m_updateActorPosBtn
            // 
            this.m_updateActorPosBtn.Location = new System.Drawing.Point(6, 269);
            this.m_updateActorPosBtn.Name = "m_updateActorPosBtn";
            this.m_updateActorPosBtn.Size = new System.Drawing.Size(152, 23);
            this.m_updateActorPosBtn.TabIndex = 10;
            this.m_updateActorPosBtn.Text = "Update Properties";
            this.m_updateActorPosBtn.UseVisualStyleBackColor = true;
            this.m_updateActorPosBtn.Click += new System.EventHandler(this.UpdateActorPosBtn_Click);
            // 
            // m_restoreActorPosBtn
            // 
            this.m_restoreActorPosBtn.Location = new System.Drawing.Point(6, 297);
            this.m_restoreActorPosBtn.Name = "m_restoreActorPosBtn";
            this.m_restoreActorPosBtn.Size = new System.Drawing.Size(152, 23);
            this.m_restoreActorPosBtn.TabIndex = 11;
            this.m_restoreActorPosBtn.Text = "Restore Properties";
            this.m_restoreActorPosBtn.UseVisualStyleBackColor = true;
            this.m_restoreActorPosBtn.Click += new System.EventHandler(this.RestoreActorPosBtn_Click);
            // 
            // m_treeView
            // 
            this.m_treeView.Location = new System.Drawing.Point(12, 34);
            this.m_treeView.Name = "m_treeView";
            this.m_treeView.Size = new System.Drawing.Size(291, 419);
            this.m_treeView.TabIndex = 12;
            this.m_treeView.AfterSelect += new System.Windows.Forms.TreeViewEventHandler(this.m_treeView_AfterSelect);
            this.m_treeView.NodeMouseClick += new System.Windows.Forms.TreeNodeMouseClickEventHandler(this.TreeView_NodeMouseClick);
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.label6);
            this.groupBox1.Controls.Add(this.groupBox3);
            this.groupBox1.Controls.Add(this.m_colorPreviewPanel);
            this.groupBox1.Controls.Add(this.label1);
            this.groupBox1.Controls.Add(this.m_actorIdTextbox);
            this.groupBox1.Controls.Add(this.m_restoreActorPosBtn);
            this.groupBox1.Controls.Add(this.m_actorTypeTextbox);
            this.groupBox1.Controls.Add(this.m_updateActorPosBtn);
            this.groupBox1.Controls.Add(this.label2);
            this.groupBox1.Controls.Add(this.label5);
            this.groupBox1.Controls.Add(this.m_actorXTextbox);
            this.groupBox1.Controls.Add(this.label4);
            this.groupBox1.Controls.Add(this.m_actorYTextbox);
            this.groupBox1.Controls.Add(this.label3);
            this.groupBox1.Controls.Add(this.m_actorZTextbox);
            this.groupBox1.Location = new System.Drawing.Point(310, 29);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(254, 326);
            this.groupBox1.TabIndex = 13;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Actor Properties";
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(34, 197);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(31, 13);
            this.label6.TabIndex = 18;
            this.label6.Text = "Color";
            // 
            // groupBox3
            // 
            this.groupBox3.Controls.Add(this.m_rotateActorYAxisDegreesTextbox);
            this.groupBox3.Controls.Add(this.m_rotateActorXAxisDegreesTextbox);
            this.groupBox3.Controls.Add(this.m_rotateActorXAxisBtn);
            this.groupBox3.Controls.Add(this.m_rotateActorYAxisBtn);
            this.groupBox3.Location = new System.Drawing.Point(120, 103);
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.Size = new System.Drawing.Size(128, 107);
            this.groupBox3.TabIndex = 17;
            this.groupBox3.TabStop = false;
            this.groupBox3.Text = "Rotate Selected Object";
            // 
            // m_rotateActorYAxisDegreesTextbox
            // 
            this.m_rotateActorYAxisDegreesTextbox.Location = new System.Drawing.Point(85, 65);
            this.m_rotateActorYAxisDegreesTextbox.Name = "m_rotateActorYAxisDegreesTextbox";
            this.m_rotateActorYAxisDegreesTextbox.Size = new System.Drawing.Size(37, 20);
            this.m_rotateActorYAxisDegreesTextbox.TabIndex = 20;
            this.m_rotateActorYAxisDegreesTextbox.Text = "0";
            // 
            // m_rotateActorXAxisDegreesTextbox
            // 
            this.m_rotateActorXAxisDegreesTextbox.Location = new System.Drawing.Point(85, 36);
            this.m_rotateActorXAxisDegreesTextbox.Name = "m_rotateActorXAxisDegreesTextbox";
            this.m_rotateActorXAxisDegreesTextbox.Size = new System.Drawing.Size(37, 20);
            this.m_rotateActorXAxisDegreesTextbox.TabIndex = 19;
            this.m_rotateActorXAxisDegreesTextbox.Text = "0";
            // 
            // m_rotateActorXAxisBtn
            // 
            this.m_rotateActorXAxisBtn.Location = new System.Drawing.Point(6, 36);
            this.m_rotateActorXAxisBtn.Name = "m_rotateActorXAxisBtn";
            this.m_rotateActorXAxisBtn.Size = new System.Drawing.Size(58, 23);
            this.m_rotateActorXAxisBtn.TabIndex = 16;
            this.m_rotateActorXAxisBtn.Text = "X Axis";
            this.m_rotateActorXAxisBtn.UseVisualStyleBackColor = true;
            this.m_rotateActorXAxisBtn.Click += new System.EventHandler(this.RotateActorXAxisBtn_Click);
            // 
            // m_rotateActorYAxisBtn
            // 
            this.m_rotateActorYAxisBtn.Location = new System.Drawing.Point(6, 65);
            this.m_rotateActorYAxisBtn.Name = "m_rotateActorYAxisBtn";
            this.m_rotateActorYAxisBtn.Size = new System.Drawing.Size(58, 23);
            this.m_rotateActorYAxisBtn.TabIndex = 13;
            this.m_rotateActorYAxisBtn.Text = "Y Axis";
            this.m_rotateActorYAxisBtn.UseVisualStyleBackColor = true;
            this.m_rotateActorYAxisBtn.Click += new System.EventHandler(this.RotateActorYAxisBtn_Click);
            // 
            // m_colorPreviewPanel
            // 
            this.m_colorPreviewPanel.BackColor = System.Drawing.Color.White;
            this.m_colorPreviewPanel.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.m_colorPreviewPanel.Location = new System.Drawing.Point(37, 228);
            this.m_colorPreviewPanel.Name = "m_colorPreviewPanel";
            this.m_colorPreviewPanel.Size = new System.Drawing.Size(64, 20);
            this.m_colorPreviewPanel.TabIndex = 14;
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.m_removeActorBtn);
            this.groupBox2.Controls.Add(this.m_addActorBtn);
            this.groupBox2.Location = new System.Drawing.Point(310, 360);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(254, 87);
            this.groupBox2.TabIndex = 14;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Actor Operations";
            // 
            // m_removeActorBtn
            // 
            this.m_removeActorBtn.Location = new System.Drawing.Point(6, 48);
            this.m_removeActorBtn.Name = "m_removeActorBtn";
            this.m_removeActorBtn.Size = new System.Drawing.Size(152, 23);
            this.m_removeActorBtn.TabIndex = 1;
            this.m_removeActorBtn.Text = "Remove Selected Actor";
            this.m_removeActorBtn.UseVisualStyleBackColor = true;
            this.m_removeActorBtn.Click += new System.EventHandler(this.RemoveActorBtn_Click);
            // 
            // m_addActorBtn
            // 
            this.m_addActorBtn.Location = new System.Drawing.Point(6, 19);
            this.m_addActorBtn.Name = "m_addActorBtn";
            this.m_addActorBtn.Size = new System.Drawing.Size(152, 23);
            this.m_addActorBtn.TabIndex = 0;
            this.m_addActorBtn.Text = "Add New Actor";
            this.m_addActorBtn.UseVisualStyleBackColor = true;
            this.m_addActorBtn.Click += new System.EventHandler(this.AddActorBtn_Click);
            // 
            // MenuStrip
            // 
            this.MenuStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.m_fileToolStripMenuItem});
            this.MenuStrip.Location = new System.Drawing.Point(0, 0);
            this.MenuStrip.Name = "MenuStrip";
            this.MenuStrip.Size = new System.Drawing.Size(573, 24);
            this.MenuStrip.TabIndex = 15;
            // 
            // m_fileToolStripMenuItem
            // 
            this.m_fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.saveLevelToolStripMenuItem,
            this.openLevelToolStripMenuItem});
            this.m_fileToolStripMenuItem.Name = "m_fileToolStripMenuItem";
            this.m_fileToolStripMenuItem.Size = new System.Drawing.Size(37, 20);
            this.m_fileToolStripMenuItem.Text = "File";
            // 
            // saveLevelToolStripMenuItem
            // 
            this.saveLevelToolStripMenuItem.Name = "saveLevelToolStripMenuItem";
            this.saveLevelToolStripMenuItem.Size = new System.Drawing.Size(133, 22);
            this.saveLevelToolStripMenuItem.Text = "Save Level";
            this.saveLevelToolStripMenuItem.Click += new System.EventHandler(this.SaveLevelToolStripMenuItem_Click);
            // 
            // openLevelToolStripMenuItem
            // 
            this.openLevelToolStripMenuItem.Name = "openLevelToolStripMenuItem";
            this.openLevelToolStripMenuItem.Size = new System.Drawing.Size(133, 22);
            this.openLevelToolStripMenuItem.Text = "Open Level";
            this.openLevelToolStripMenuItem.Click += new System.EventHandler(this.OpenLevelToolStripMenuItem_Click);
            // 
            // ActorPropertiesForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(573, 501);
            this.ControlBox = false;
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.m_treeView);
            this.Controls.Add(this.MenuStrip);
            this.MainMenuStrip = this.MenuStrip;
            this.Name = "ActorPropertiesForm";
            this.Text = "Actor Properties";
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.groupBox3.ResumeLayout(false);
            this.groupBox3.PerformLayout();
            this.groupBox2.ResumeLayout(false);
            this.MenuStrip.ResumeLayout(false);
            this.MenuStrip.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox m_actorIdTextbox;
        private System.Windows.Forms.TextBox m_actorTypeTextbox;
        private System.Windows.Forms.TextBox m_actorXTextbox;
        private System.Windows.Forms.TextBox m_actorYTextbox;
        private System.Windows.Forms.TextBox m_actorZTextbox;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Button m_updateActorPosBtn;
        private System.Windows.Forms.Button m_restoreActorPosBtn;
        private System.Windows.Forms.TreeView m_treeView;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.Button m_removeActorBtn;
        private System.Windows.Forms.Button m_addActorBtn;
        private System.Windows.Forms.Button m_rotateActorYAxisBtn;
        private System.Windows.Forms.Button m_rotateActorXAxisBtn;
        private System.Windows.Forms.Panel m_colorPreviewPanel;
        private System.Windows.Forms.GroupBox groupBox3;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.MenuStrip MenuStrip;
        private System.Windows.Forms.ToolStripMenuItem m_fileToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem saveLevelToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem openLevelToolStripMenuItem;
        private System.Windows.Forms.SaveFileDialog SaveFileDialog;
        private System.Windows.Forms.OpenFileDialog OpenFileDialog;
        private System.Windows.Forms.TextBox m_rotateActorXAxisDegreesTextbox;
        private System.Windows.Forms.TextBox m_rotateActorYAxisDegreesTextbox;



    }
}