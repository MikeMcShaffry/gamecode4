namespace EditorApp
{
    partial class EditorForm
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
            // We have to call Shutdown to shutdown the DirectX device, and
            // free the dll
            Shutdown();
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            //base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(EditorForm));
            this.DisplayPanel = new System.Windows.Forms.Panel();
            this.menuStrip1 = new System.Windows.Forms.MenuStrip();
            this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.openProjectToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.closeProjectToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.buildProjectToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.projectPropertiesToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.sepToolStripMenuItem = new System.Windows.Forms.ToolStripSeparator();
            this.openLevelToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.saveLevelToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.saveLevelAsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.closeLevelToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.sepToolStripMenuItem1 = new System.Windows.Forms.ToolStripSeparator();
            this.exitToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.editToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.actorsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.createToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.componentsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.transformToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.renderToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.gridToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.teapotToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.sphereToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.skyboxToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.directionalLightToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.meshToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.physicsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.scriptToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.helpToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.splitContainer1 = new System.Windows.Forms.SplitContainer();
            this.tabControl_Properties = new System.Windows.Forms.TabControl();
            this.tabPage_AssetTree = new System.Windows.Forms.TabPage();
            this.TreeView_Assets = new System.Windows.Forms.TreeView();
            this.tabPage_Actors = new System.Windows.Forms.TabPage();
            this.TreeView_Actors = new System.Windows.Forms.TreeView();
            this.Panel_ActorComponents = new System.Windows.Forms.Panel();
            this.menuStrip1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).BeginInit();
            this.splitContainer1.Panel1.SuspendLayout();
            this.splitContainer1.Panel2.SuspendLayout();
            this.splitContainer1.SuspendLayout();
            this.tabControl_Properties.SuspendLayout();
            this.tabPage_AssetTree.SuspendLayout();
            this.tabPage_Actors.SuspendLayout();
            this.SuspendLayout();
            // 
            // DisplayPanel
            // 
            this.DisplayPanel.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.DisplayPanel.Location = new System.Drawing.Point(4, 34);
            this.DisplayPanel.Margin = new System.Windows.Forms.Padding(4);
            this.DisplayPanel.MaximumSize = new System.Drawing.Size(800, 600);
            this.DisplayPanel.MinimumSize = new System.Drawing.Size(400, 300);
            this.DisplayPanel.Name = "DisplayPanel";
            this.DisplayPanel.Padding = new System.Windows.Forms.Padding(4);
            this.DisplayPanel.Size = new System.Drawing.Size(800, 600);
            this.DisplayPanel.TabIndex = 0;
            // 
            // menuStrip1
            // 
            this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem,
            this.editToolStripMenuItem,
            this.actorsToolStripMenuItem,
            this.componentsToolStripMenuItem,
            this.helpToolStripMenuItem});
            this.menuStrip1.Location = new System.Drawing.Point(0, 0);
            this.menuStrip1.Name = "menuStrip1";
            this.menuStrip1.Padding = new System.Windows.Forms.Padding(8, 2, 0, 2);
            this.menuStrip1.Size = new System.Drawing.Size(1323, 28);
            this.menuStrip1.TabIndex = 2;
            this.menuStrip1.Text = "menuStrip1";
            // 
            // fileToolStripMenuItem
            // 
            this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.openProjectToolStripMenuItem,
            this.closeProjectToolStripMenuItem,
            this.buildProjectToolStripMenuItem,
            this.projectPropertiesToolStripMenuItem,
            this.sepToolStripMenuItem,
            this.openLevelToolStripMenuItem,
            this.saveLevelToolStripMenuItem,
            this.saveLevelAsToolStripMenuItem,
            this.closeLevelToolStripMenuItem,
            this.sepToolStripMenuItem1,
            this.exitToolStripMenuItem});
            this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            this.fileToolStripMenuItem.Size = new System.Drawing.Size(44, 24);
            this.fileToolStripMenuItem.Text = "File";
            // 
            // openProjectToolStripMenuItem
            // 
            this.openProjectToolStripMenuItem.Name = "openProjectToolStripMenuItem";
            this.openProjectToolStripMenuItem.Size = new System.Drawing.Size(195, 24);
            this.openProjectToolStripMenuItem.Text = "Open Project";
            // 
            // closeProjectToolStripMenuItem
            // 
            this.closeProjectToolStripMenuItem.Name = "closeProjectToolStripMenuItem";
            this.closeProjectToolStripMenuItem.Size = new System.Drawing.Size(195, 24);
            this.closeProjectToolStripMenuItem.Text = "Close Project";
            this.closeProjectToolStripMenuItem.Click += new System.EventHandler(this.closeProjectToolStripMenuItem_Click);
            // 
            // buildProjectToolStripMenuItem
            // 
            this.buildProjectToolStripMenuItem.Name = "buildProjectToolStripMenuItem";
            this.buildProjectToolStripMenuItem.Size = new System.Drawing.Size(195, 24);
            this.buildProjectToolStripMenuItem.Text = "Build Project";
            this.buildProjectToolStripMenuItem.Click += new System.EventHandler(this.buildProjectToolStripMenuItem_Click);
            this.buildProjectToolStripMenuItem.MouseUp += new System.Windows.Forms.MouseEventHandler(this.buildProjectToolStripMenuItem_MouseUp);
            // 
            // projectPropertiesToolStripMenuItem
            // 
            this.projectPropertiesToolStripMenuItem.Name = "projectPropertiesToolStripMenuItem";
            this.projectPropertiesToolStripMenuItem.Size = new System.Drawing.Size(195, 24);
            this.projectPropertiesToolStripMenuItem.Text = "Project Properties";
            // 
            // sepToolStripMenuItem
            // 
            this.sepToolStripMenuItem.Name = "sepToolStripMenuItem";
            this.sepToolStripMenuItem.Size = new System.Drawing.Size(192, 6);
            // 
            // openLevelToolStripMenuItem
            // 
            this.openLevelToolStripMenuItem.Name = "openLevelToolStripMenuItem";
            this.openLevelToolStripMenuItem.Size = new System.Drawing.Size(195, 24);
            this.openLevelToolStripMenuItem.Text = "Open Level";
            this.openLevelToolStripMenuItem.Click += new System.EventHandler(this.openLevelToolStripMenuItem_Click);
            // 
            // saveLevelToolStripMenuItem
            // 
            this.saveLevelToolStripMenuItem.Name = "saveLevelToolStripMenuItem";
            this.saveLevelToolStripMenuItem.Size = new System.Drawing.Size(195, 24);
            this.saveLevelToolStripMenuItem.Text = "Save Level";
            this.saveLevelToolStripMenuItem.Click += new System.EventHandler(this.saveLevelToolStripMenuItem_Click);
            // 
            // saveLevelAsToolStripMenuItem
            // 
            this.saveLevelAsToolStripMenuItem.Name = "saveLevelAsToolStripMenuItem";
            this.saveLevelAsToolStripMenuItem.Size = new System.Drawing.Size(195, 24);
            this.saveLevelAsToolStripMenuItem.Text = "Save Level As...";
            // 
            // closeLevelToolStripMenuItem
            // 
            this.closeLevelToolStripMenuItem.Name = "closeLevelToolStripMenuItem";
            this.closeLevelToolStripMenuItem.Size = new System.Drawing.Size(195, 24);
            this.closeLevelToolStripMenuItem.Text = "Close Level";
            // 
            // sepToolStripMenuItem1
            // 
            this.sepToolStripMenuItem1.Name = "sepToolStripMenuItem1";
            this.sepToolStripMenuItem1.Size = new System.Drawing.Size(192, 6);
            // 
            // exitToolStripMenuItem
            // 
            this.exitToolStripMenuItem.Name = "exitToolStripMenuItem";
            this.exitToolStripMenuItem.Size = new System.Drawing.Size(195, 24);
            this.exitToolStripMenuItem.Text = "Exit";
            // 
            // editToolStripMenuItem
            // 
            this.editToolStripMenuItem.Name = "editToolStripMenuItem";
            this.editToolStripMenuItem.Size = new System.Drawing.Size(47, 24);
            this.editToolStripMenuItem.Text = "Edit";
            // 
            // actorsToolStripMenuItem
            // 
            this.actorsToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.createToolStripMenuItem});
            this.actorsToolStripMenuItem.Name = "actorsToolStripMenuItem";
            this.actorsToolStripMenuItem.Size = new System.Drawing.Size(63, 24);
            this.actorsToolStripMenuItem.Text = "Actors";
            // 
            // createToolStripMenuItem
            // 
            this.createToolStripMenuItem.Name = "createToolStripMenuItem";
            this.createToolStripMenuItem.Size = new System.Drawing.Size(121, 24);
            this.createToolStripMenuItem.Text = "Create";
            // 
            // componentsToolStripMenuItem
            // 
            this.componentsToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.transformToolStripMenuItem,
            this.renderToolStripMenuItem,
            this.physicsToolStripMenuItem,
            this.scriptToolStripMenuItem});
            this.componentsToolStripMenuItem.Name = "componentsToolStripMenuItem";
            this.componentsToolStripMenuItem.Size = new System.Drawing.Size(105, 24);
            this.componentsToolStripMenuItem.Text = "Components";
            // 
            // transformToolStripMenuItem
            // 
            this.transformToolStripMenuItem.Name = "transformToolStripMenuItem";
            this.transformToolStripMenuItem.Size = new System.Drawing.Size(152, 24);
            this.transformToolStripMenuItem.Text = "Transform";
            // 
            // renderToolStripMenuItem
            // 
            this.renderToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.gridToolStripMenuItem,
            this.teapotToolStripMenuItem,
            this.sphereToolStripMenuItem,
            this.skyboxToolStripMenuItem,
            this.directionalLightToolStripMenuItem,
            this.meshToolStripMenuItem});
            this.renderToolStripMenuItem.Name = "renderToolStripMenuItem";
            this.renderToolStripMenuItem.Size = new System.Drawing.Size(152, 24);
            this.renderToolStripMenuItem.Text = "Render";
            // 
            // gridToolStripMenuItem
            // 
            this.gridToolStripMenuItem.Name = "gridToolStripMenuItem";
            this.gridToolStripMenuItem.Size = new System.Drawing.Size(188, 24);
            this.gridToolStripMenuItem.Text = "Grid";
            // 
            // teapotToolStripMenuItem
            // 
            this.teapotToolStripMenuItem.Name = "teapotToolStripMenuItem";
            this.teapotToolStripMenuItem.Size = new System.Drawing.Size(188, 24);
            this.teapotToolStripMenuItem.Text = "Teapot";
            // 
            // sphereToolStripMenuItem
            // 
            this.sphereToolStripMenuItem.Name = "sphereToolStripMenuItem";
            this.sphereToolStripMenuItem.Size = new System.Drawing.Size(188, 24);
            this.sphereToolStripMenuItem.Text = "Sphere";
            // 
            // skyboxToolStripMenuItem
            // 
            this.skyboxToolStripMenuItem.Name = "skyboxToolStripMenuItem";
            this.skyboxToolStripMenuItem.Size = new System.Drawing.Size(188, 24);
            this.skyboxToolStripMenuItem.Text = "Skybox";
            // 
            // directionalLightToolStripMenuItem
            // 
            this.directionalLightToolStripMenuItem.Name = "directionalLightToolStripMenuItem";
            this.directionalLightToolStripMenuItem.Size = new System.Drawing.Size(188, 24);
            this.directionalLightToolStripMenuItem.Text = "Directional Light";
            // 
            // meshToolStripMenuItem
            // 
            this.meshToolStripMenuItem.Name = "meshToolStripMenuItem";
            this.meshToolStripMenuItem.Size = new System.Drawing.Size(188, 24);
            this.meshToolStripMenuItem.Text = "Mesh";
            // 
            // physicsToolStripMenuItem
            // 
            this.physicsToolStripMenuItem.Name = "physicsToolStripMenuItem";
            this.physicsToolStripMenuItem.Size = new System.Drawing.Size(152, 24);
            this.physicsToolStripMenuItem.Text = "Physics";
            // 
            // scriptToolStripMenuItem
            // 
            this.scriptToolStripMenuItem.Name = "scriptToolStripMenuItem";
            this.scriptToolStripMenuItem.Size = new System.Drawing.Size(152, 24);
            this.scriptToolStripMenuItem.Text = "Script";
            // 
            // helpToolStripMenuItem
            // 
            this.helpToolStripMenuItem.Name = "helpToolStripMenuItem";
            this.helpToolStripMenuItem.Size = new System.Drawing.Size(53, 24);
            this.helpToolStripMenuItem.Text = "Help";
            // 
            // splitContainer1
            // 
            this.splitContainer1.Dock = System.Windows.Forms.DockStyle.Right;
            this.splitContainer1.Location = new System.Drawing.Point(808, 28);
            this.splitContainer1.Margin = new System.Windows.Forms.Padding(4);
            this.splitContainer1.Name = "splitContainer1";
            this.splitContainer1.Orientation = System.Windows.Forms.Orientation.Horizontal;
            // 
            // splitContainer1.Panel1
            // 
            this.splitContainer1.Panel1.Controls.Add(this.tabControl_Properties);
            // 
            // splitContainer1.Panel2
            // 
            this.splitContainer1.Panel2.AutoScroll = true;
            this.splitContainer1.Panel2.BackColor = System.Drawing.Color.White;
            this.splitContainer1.Panel2.Controls.Add(this.Panel_ActorComponents);
            this.splitContainer1.Size = new System.Drawing.Size(515, 609);
            this.splitContainer1.SplitterDistance = 304;
            this.splitContainer1.SplitterWidth = 5;
            this.splitContainer1.TabIndex = 4;
            // 
            // tabControl_Properties
            // 
            this.tabControl_Properties.Controls.Add(this.tabPage_AssetTree);
            this.tabControl_Properties.Controls.Add(this.tabPage_Actors);
            this.tabControl_Properties.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tabControl_Properties.Location = new System.Drawing.Point(0, 0);
            this.tabControl_Properties.Margin = new System.Windows.Forms.Padding(4);
            this.tabControl_Properties.Name = "tabControl_Properties";
            this.tabControl_Properties.SelectedIndex = 0;
            this.tabControl_Properties.Size = new System.Drawing.Size(515, 304);
            this.tabControl_Properties.TabIndex = 3;
            // 
            // tabPage_AssetTree
            // 
            this.tabPage_AssetTree.Controls.Add(this.TreeView_Assets);
            this.tabPage_AssetTree.Location = new System.Drawing.Point(4, 25);
            this.tabPage_AssetTree.Margin = new System.Windows.Forms.Padding(4);
            this.tabPage_AssetTree.Name = "tabPage_AssetTree";
            this.tabPage_AssetTree.Padding = new System.Windows.Forms.Padding(4);
            this.tabPage_AssetTree.Size = new System.Drawing.Size(507, 275);
            this.tabPage_AssetTree.TabIndex = 0;
            this.tabPage_AssetTree.Text = "Asset Tree";
            this.tabPage_AssetTree.UseVisualStyleBackColor = true;
            // 
            // TreeView_Assets
            // 
            this.TreeView_Assets.Dock = System.Windows.Forms.DockStyle.Fill;
            this.TreeView_Assets.Location = new System.Drawing.Point(4, 4);
            this.TreeView_Assets.Margin = new System.Windows.Forms.Padding(4);
            this.TreeView_Assets.Name = "TreeView_Assets";
            this.TreeView_Assets.Size = new System.Drawing.Size(499, 267);
            this.TreeView_Assets.TabIndex = 0;
            this.TreeView_Assets.MouseDoubleClick += new System.Windows.Forms.MouseEventHandler(this.TreeView_Assets_MouseDoubleClick);
            // 
            // tabPage_Actors
            // 
            this.tabPage_Actors.Controls.Add(this.TreeView_Actors);
            this.tabPage_Actors.Location = new System.Drawing.Point(4, 25);
            this.tabPage_Actors.Margin = new System.Windows.Forms.Padding(4);
            this.tabPage_Actors.Name = "tabPage_Actors";
            this.tabPage_Actors.Padding = new System.Windows.Forms.Padding(4);
            this.tabPage_Actors.Size = new System.Drawing.Size(507, 160);
            this.tabPage_Actors.TabIndex = 1;
            this.tabPage_Actors.Text = "Actors";
            this.tabPage_Actors.UseVisualStyleBackColor = true;
            // 
            // TreeView_Actors
            // 
            this.TreeView_Actors.Dock = System.Windows.Forms.DockStyle.Fill;
            this.TreeView_Actors.Location = new System.Drawing.Point(4, 4);
            this.TreeView_Actors.Margin = new System.Windows.Forms.Padding(4);
            this.TreeView_Actors.Name = "TreeView_Actors";
            this.TreeView_Actors.Size = new System.Drawing.Size(499, 152);
            this.TreeView_Actors.TabIndex = 0;
            this.TreeView_Actors.AfterSelect += new System.Windows.Forms.TreeViewEventHandler(this.TreeView_Actors_AfterSelect);
            // 
            // Panel_ActorComponents
            // 
            this.Panel_ActorComponents.AutoScroll = true;
            this.Panel_ActorComponents.AutoSize = true;
            this.Panel_ActorComponents.Dock = System.Windows.Forms.DockStyle.Fill;
            this.Panel_ActorComponents.Location = new System.Drawing.Point(0, 0);
            this.Panel_ActorComponents.Margin = new System.Windows.Forms.Padding(4);
            this.Panel_ActorComponents.Name = "Panel_ActorComponents";
            this.Panel_ActorComponents.Padding = new System.Windows.Forms.Padding(4);
            this.Panel_ActorComponents.Size = new System.Drawing.Size(515, 300);
            this.Panel_ActorComponents.TabIndex = 0;
            // 
            // EditorForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1323, 637);
            this.Controls.Add(this.splitContainer1);
            this.Controls.Add(this.DisplayPanel);
            this.Controls.Add(this.menuStrip1);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MainMenuStrip = this.menuStrip1;
            this.Margin = new System.Windows.Forms.Padding(4);
            this.Name = "EditorForm";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "GameCode4 Editor";
            this.menuStrip1.ResumeLayout(false);
            this.menuStrip1.PerformLayout();
            this.splitContainer1.Panel1.ResumeLayout(false);
            this.splitContainer1.Panel2.ResumeLayout(false);
            this.splitContainer1.Panel2.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).EndInit();
            this.splitContainer1.ResumeLayout(false);
            this.tabControl_Properties.ResumeLayout(false);
            this.tabPage_AssetTree.ResumeLayout(false);
            this.tabPage_Actors.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Panel DisplayPanel;
        private System.Windows.Forms.MenuStrip menuStrip1;
        private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem openProjectToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem closeProjectToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem buildProjectToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem openLevelToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem saveLevelToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem closeLevelToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem exitToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem componentsToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem transformToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem renderToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem gridToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem teapotToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem sphereToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem skyboxToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem directionalLightToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem meshToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem physicsToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem scriptToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem projectPropertiesToolStripMenuItem;
        private System.Windows.Forms.SplitContainer splitContainer1;
        private System.Windows.Forms.TabControl tabControl_Properties;
        private System.Windows.Forms.TabPage tabPage_AssetTree;
        private System.Windows.Forms.TreeView TreeView_Assets;
        private System.Windows.Forms.TabPage tabPage_Actors;
        private System.Windows.Forms.TreeView TreeView_Actors;
        private System.Windows.Forms.Panel Panel_ActorComponents;
        private System.Windows.Forms.ToolStripMenuItem editToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem actorsToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem createToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem helpToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator sepToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem saveLevelAsToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator sepToolStripMenuItem1;
    }
}