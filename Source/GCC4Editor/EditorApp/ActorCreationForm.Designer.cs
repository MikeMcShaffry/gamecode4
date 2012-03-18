namespace EditorApp
{
    partial class ActorCreationForm
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
            this.m_actorTypeComboBox = new System.Windows.Forms.ComboBox();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.m_radiusLabel = new System.Windows.Forms.Label();
            this.m_lengthLabel = new System.Windows.Forms.Label();
            this.m_RadiusTextbox = new System.Windows.Forms.TextBox();
            this.m_colorLabel = new System.Windows.Forms.Label();
            this.m_colorDisplayPanel = new System.Windows.Forms.Panel();
            this.m_LengthTextbox = new System.Windows.Forms.TextBox();
            this.m_createActorBtn = new System.Windows.Forms.Button();
            this.m_cancelBtn = new System.Windows.Forms.Button();
            this.m_colorDialog = new System.Windows.Forms.ColorDialog();
            this.m_effectFileLabel = new System.Windows.Forms.Label();
            this.m_meshFileLabel = new System.Windows.Forms.Label();
            this.m_effectFileTextbox = new System.Windows.Forms.TextBox();
            this.m_meshFileTextbox = new System.Windows.Forms.TextBox();
            this.m_browseEffectFileBtn = new System.Windows.Forms.Button();
            this.m_browseMeshFileBtn = new System.Windows.Forms.Button();
            this.m_openFileDialog = new System.Windows.Forms.OpenFileDialog();
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // m_actorTypeComboBox
            // 
            this.m_actorTypeComboBox.FormattingEnabled = true;
            this.m_actorTypeComboBox.Location = new System.Drawing.Point(13, 13);
            this.m_actorTypeComboBox.Name = "m_actorTypeComboBox";
            this.m_actorTypeComboBox.Size = new System.Drawing.Size(121, 21);
            this.m_actorTypeComboBox.TabIndex = 0;
            this.m_actorTypeComboBox.SelectedIndexChanged += new System.EventHandler(this.ActorTypeComboBox_SelectedIndexChanged);
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.m_browseMeshFileBtn);
            this.groupBox1.Controls.Add(this.m_browseEffectFileBtn);
            this.groupBox1.Controls.Add(this.m_meshFileTextbox);
            this.groupBox1.Controls.Add(this.m_effectFileTextbox);
            this.groupBox1.Controls.Add(this.m_meshFileLabel);
            this.groupBox1.Controls.Add(this.m_effectFileLabel);
            this.groupBox1.Controls.Add(this.m_radiusLabel);
            this.groupBox1.Controls.Add(this.m_lengthLabel);
            this.groupBox1.Controls.Add(this.m_RadiusTextbox);
            this.groupBox1.Controls.Add(this.m_colorLabel);
            this.groupBox1.Controls.Add(this.m_colorDisplayPanel);
            this.groupBox1.Controls.Add(this.m_LengthTextbox);
            this.groupBox1.Location = new System.Drawing.Point(13, 41);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(319, 188);
            this.groupBox1.TabIndex = 1;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Actor Properties";
            // 
            // m_radiusLabel
            // 
            this.m_radiusLabel.AutoSize = true;
            this.m_radiusLabel.Location = new System.Drawing.Point(35, 80);
            this.m_radiusLabel.Name = "m_radiusLabel";
            this.m_radiusLabel.Size = new System.Drawing.Size(40, 13);
            this.m_radiusLabel.TabIndex = 5;
            this.m_radiusLabel.Text = "Radius";
            // 
            // m_lengthLabel
            // 
            this.m_lengthLabel.AutoSize = true;
            this.m_lengthLabel.Location = new System.Drawing.Point(35, 54);
            this.m_lengthLabel.Name = "m_lengthLabel";
            this.m_lengthLabel.Size = new System.Drawing.Size(40, 13);
            this.m_lengthLabel.TabIndex = 4;
            this.m_lengthLabel.Text = "Length";
            // 
            // m_RadiusTextbox
            // 
            this.m_RadiusTextbox.Location = new System.Drawing.Point(106, 80);
            this.m_RadiusTextbox.Name = "m_RadiusTextbox";
            this.m_RadiusTextbox.Size = new System.Drawing.Size(100, 20);
            this.m_RadiusTextbox.TabIndex = 3;
            // 
            // m_colorLabel
            // 
            this.m_colorLabel.AutoSize = true;
            this.m_colorLabel.Location = new System.Drawing.Point(44, 26);
            this.m_colorLabel.Name = "m_colorLabel";
            this.m_colorLabel.Size = new System.Drawing.Size(31, 13);
            this.m_colorLabel.TabIndex = 1;
            this.m_colorLabel.Text = "Color";
            // 
            // m_colorDisplayPanel
            // 
            this.m_colorDisplayPanel.BackColor = System.Drawing.Color.White;
            this.m_colorDisplayPanel.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.m_colorDisplayPanel.Location = new System.Drawing.Point(106, 26);
            this.m_colorDisplayPanel.Name = "m_colorDisplayPanel";
            this.m_colorDisplayPanel.Size = new System.Drawing.Size(75, 22);
            this.m_colorDisplayPanel.TabIndex = 0;
            this.m_colorDisplayPanel.MouseDown += new System.Windows.Forms.MouseEventHandler(this.ColorDisplayPanel_MouseDown);
            // 
            // m_LengthTextbox
            // 
            this.m_LengthTextbox.Location = new System.Drawing.Point(106, 54);
            this.m_LengthTextbox.Name = "m_LengthTextbox";
            this.m_LengthTextbox.Size = new System.Drawing.Size(100, 20);
            this.m_LengthTextbox.TabIndex = 2;
            // 
            // m_createActorBtn
            // 
            this.m_createActorBtn.Location = new System.Drawing.Point(12, 252);
            this.m_createActorBtn.Name = "m_createActorBtn";
            this.m_createActorBtn.Size = new System.Drawing.Size(75, 23);
            this.m_createActorBtn.TabIndex = 2;
            this.m_createActorBtn.Text = "Create Actor";
            this.m_createActorBtn.UseVisualStyleBackColor = true;
            this.m_createActorBtn.Click += new System.EventHandler(this.CreateActorBtn_Click);
            // 
            // m_cancelBtn
            // 
            this.m_cancelBtn.Location = new System.Drawing.Point(93, 252);
            this.m_cancelBtn.Name = "m_cancelBtn";
            this.m_cancelBtn.Size = new System.Drawing.Size(75, 23);
            this.m_cancelBtn.TabIndex = 3;
            this.m_cancelBtn.Text = "Cancel";
            this.m_cancelBtn.UseVisualStyleBackColor = true;
            this.m_cancelBtn.Click += new System.EventHandler(this.CancelBtn_Click);
            // 
            // m_effectFileLabel
            // 
            this.m_effectFileLabel.AutoSize = true;
            this.m_effectFileLabel.Location = new System.Drawing.Point(35, 115);
            this.m_effectFileLabel.Name = "m_effectFileLabel";
            this.m_effectFileLabel.Size = new System.Drawing.Size(54, 13);
            this.m_effectFileLabel.TabIndex = 6;
            this.m_effectFileLabel.Text = "Effect File";
            // 
            // m_meshFileLabel
            // 
            this.m_meshFileLabel.AutoSize = true;
            this.m_meshFileLabel.Location = new System.Drawing.Point(35, 144);
            this.m_meshFileLabel.Name = "m_meshFileLabel";
            this.m_meshFileLabel.Size = new System.Drawing.Size(52, 13);
            this.m_meshFileLabel.TabIndex = 7;
            this.m_meshFileLabel.Text = "Mesh File";
            // 
            // m_effectFileTextbox
            // 
            this.m_effectFileTextbox.Location = new System.Drawing.Point(106, 106);
            this.m_effectFileTextbox.Name = "m_effectFileTextbox";
            this.m_effectFileTextbox.Size = new System.Drawing.Size(100, 20);
            this.m_effectFileTextbox.TabIndex = 8;
            // 
            // m_meshFileTextbox
            // 
            this.m_meshFileTextbox.Location = new System.Drawing.Point(106, 132);
            this.m_meshFileTextbox.Name = "m_meshFileTextbox";
            this.m_meshFileTextbox.Size = new System.Drawing.Size(100, 20);
            this.m_meshFileTextbox.TabIndex = 9;
            // 
            // m_browseEffectFileBtn
            // 
            this.m_browseEffectFileBtn.Location = new System.Drawing.Point(212, 103);
            this.m_browseEffectFileBtn.Name = "m_browseEffectFileBtn";
            this.m_browseEffectFileBtn.Size = new System.Drawing.Size(75, 23);
            this.m_browseEffectFileBtn.TabIndex = 4;
            this.m_browseEffectFileBtn.Text = "Browse";
            this.m_browseEffectFileBtn.UseVisualStyleBackColor = true;
            this.m_browseEffectFileBtn.Click += new System.EventHandler(this.BrowseEffectFileBtn_Click);
            // 
            // m_browseMeshFileBtn
            // 
            this.m_browseMeshFileBtn.Location = new System.Drawing.Point(212, 132);
            this.m_browseMeshFileBtn.Name = "m_browseMeshFileBtn";
            this.m_browseMeshFileBtn.Size = new System.Drawing.Size(75, 23);
            this.m_browseMeshFileBtn.TabIndex = 10;
            this.m_browseMeshFileBtn.Text = "Browse";
            this.m_browseMeshFileBtn.UseVisualStyleBackColor = true;
            this.m_browseMeshFileBtn.Click += new System.EventHandler(this.BrowseMeshFileBtn_Click);
            // 
            // ActorCreationForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(344, 287);
            this.Controls.Add(this.m_cancelBtn);
            this.Controls.Add(this.m_createActorBtn);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.m_actorTypeComboBox);
            this.Name = "ActorCreationForm";
            this.Text = "Actor Creation";
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.ComboBox m_actorTypeComboBox;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Panel m_colorDisplayPanel;
        private System.Windows.Forms.Button m_createActorBtn;
        private System.Windows.Forms.Button m_cancelBtn;
        private System.Windows.Forms.Label m_colorLabel;
        private System.Windows.Forms.TextBox m_LengthTextbox;
        private System.Windows.Forms.Label m_radiusLabel;
        private System.Windows.Forms.Label m_lengthLabel;
        private System.Windows.Forms.TextBox m_RadiusTextbox;
        private System.Windows.Forms.ColorDialog m_colorDialog;
        private System.Windows.Forms.Button m_browseMeshFileBtn;
        private System.Windows.Forms.Button m_browseEffectFileBtn;
        private System.Windows.Forms.TextBox m_meshFileTextbox;
        private System.Windows.Forms.TextBox m_effectFileTextbox;
        private System.Windows.Forms.Label m_meshFileLabel;
        private System.Windows.Forms.Label m_effectFileLabel;
        private System.Windows.Forms.OpenFileDialog m_openFileDialog;
    }
}