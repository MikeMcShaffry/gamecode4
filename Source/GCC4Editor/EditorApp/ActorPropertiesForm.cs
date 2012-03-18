//========================================================================
// EditorForm.cs
//
// Part of the Editor Application
//
// GameCode3 is the sample application that encapsulates much of the source code
// discussed in "Game Coding Complete - 3rd Edition" by Mike McShaffry, published by
// Charles River Media. ISBN-10: 1-58450-680-6   ISBN-13: 978-1-58450-680-5
//
// If this source code has found it's way to you, and you think it has helped you
// in any way, do the author a favor and buy a new copy of the book - there are 
// detailed explanations in it that compliment this code well. Buy a copy at Amazon.com
// by clicking here: 
//    http://www.amazon.com/gp/product/1584506806?ie=UTF8&tag=gamecodecompl-20&linkCode=as2&camp=1789&creative=390957&creativeASIN=1584506806
//
// There's a companion web site at http://www.mcshaffry.com/GameCode/
// 
// The source code is managed and maintained through Google Code: http://gamecode3.googlecode.com/svn/trunk/
//
// (c) Copyright 2009 Michael L. McShaffry
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License v2
// as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
//========================================================================

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Collections;
using System.Windows.Forms;
using System.IO;

namespace EditorApp
{
    //========================================================================
    // class ActorPropertiesForm                    - Chapter 20, page 775
    //
    // The EditorForm displays information on the current scene
    //
    //========================================================================

    public partial class ActorPropertiesForm : Form
    {
        EngineDisplayForm m_parent;
        ArrayList m_types;
        TreeNode m_mainNode;
        ActorCreationForm m_createForm;

        public ActorPropertiesForm( EngineDisplayForm parent )
        {
            InitializeComponent();
            m_parent = parent;
            m_types = new ArrayList();
            m_types.Add("Unknown");
            m_types.Add("Sphere");
            m_types.Add("Teapot");
            m_types.Add("AiTeapot");
            m_types.Add("Test Object");
            m_types.Add("Grid");
            m_types.Add("Generic Mesh Object");

            m_createForm = new ActorCreationForm();

            m_mainNode = new TreeNode();
            m_mainNode.Name = "mainNode";
            m_mainNode.Text = "Scene";
            this.m_treeView.Nodes.Add(m_mainNode);

            // Make sure nothing that can change the scene
            // is active until the Editor itself has been
            // initialized
            m_fileToolStripMenuItem.Enabled = false;
            m_addActorBtn.Enabled = false;
            m_removeActorBtn.Enabled = false;
            m_updateActorPosBtn.Enabled = false;
            m_restoreActorPosBtn.Enabled = false;
            m_rotateActorYAxisBtn.Enabled = false;
            m_rotateActorXAxisBtn.Enabled = false;
        }

        public void InitScene()
        {
            // This gets called when we are ready to make
            // adjustments to the scene.

            m_fileToolStripMenuItem.Enabled = true;
            m_addActorBtn.Enabled = true;
            m_removeActorBtn.Enabled = true;
            m_updateActorPosBtn.Enabled = true;
            m_restoreActorPosBtn.Enabled = true;
            m_rotateActorXAxisBtn.Enabled = true;
            m_rotateActorYAxisBtn.Enabled = true;

            m_mainNode.Nodes.Clear();
            int[] actorList = m_parent.GetActorList();
            // Get a list of actors in the scene, and add
            // the appropriate information to our treeview
            PopulateTreeView(actorList);
        }

        public void SelectTreeNode(int actorId)
        {
            // There are two methods of populating actor information. We can
            // select a node in the treeview, or we can click on the actor
            // in the world view. If we click on the actor in the world view,
            // we use this function to update the actor information.
            TreeNode[] node = m_mainNode.Nodes.Find(actorId.ToString(), true);
            if (node.GetLength(0) > 0)
            {
                m_treeView.SelectedNode = node[0];
                EditorActorParams actorParams = m_parent.GetActorInformation((uint)actorId);
                PopulateActorInformation(actorParams);
            }
        }


        private void PopulateActorInformation(EditorActorParams actorParams)
        {
            m_actorIdTextbox.Text = actorParams.m_actorId.ToString();
            m_actorTypeTextbox.Text = (String)m_types[(int)actorParams.m_actorType];
            m_actorXTextbox.Text = actorParams.m_pos[0].ToString();
            m_actorYTextbox.Text = actorParams.m_pos[1].ToString();
            m_actorZTextbox.Text = actorParams.m_pos[2].ToString();
            m_colorPreviewPanel.BackColor = actorParams.m_color;
        }


        private void TreeView_NodeMouseClick(object sender, TreeNodeMouseClickEventArgs e)
        {
            TreeNode node = e.Node;
            if (node != m_mainNode)
            {
                // If we click on an actor node, have the EngineDisplayForm retrieve information
                // on that actor from the unmanaged dll.
                EditorActorParams actorParams = m_parent.GetActorInformation(UInt32.Parse(node.Name));
                PopulateActorInformation(actorParams);
            }
        }

        private void PopulateTreeView(int[] actorList)
        {
            // Add each actor as its own node in the treeview.
            for (int i = 0; i < actorList.GetLength(0); i++)
            {
                TreeNode node = new TreeNode();
                node.Name = actorList[i].ToString();
                node.Text = "Actor " + actorList[i];
                m_mainNode.Nodes.Add(node);
            }
        }

        //
        // UpdateActorPosBtn_Click                  - Chapter 20, page 786
        //
        private void UpdateActorPosBtn_Click(object sender, EventArgs e)
        {
            //m_parent.SetActorInformation(UInt32.Parse(this.m_actorIdTextbox.Text), float.Parse(this.m_actorXTextbox.Text), float.Parse(this.m_actorYTextbox.Text), float.Parse(this.m_actorZTextbox.Text)); 
            NativeMethods.SetActorPos(UInt32.Parse(this.m_actorIdTextbox.Text), float.Parse(this.m_actorXTextbox.Text), float.Parse(this.m_actorYTextbox.Text), float.Parse(this.m_actorZTextbox.Text)); 
        }

        //
        // RestoreActorPosBtn_Click                  - Chapter 20, page 786
        //
        private void RestoreActorPosBtn_Click(object sender, EventArgs e)
        {
            // The position has been changed in the EditorForm, but the user wants to get the 
            // old position information on this actor before updating the position.
            // The EngineDisplayForm should retrieve information on this actor, which the EditorForm
            // will use to repopulate actor information.
            EditorActorParams actorParams = m_parent.GetActorInformation(UInt32.Parse(this.m_actorIdTextbox.Text));
            PopulateActorInformation(actorParams);
        }

        //
        // RotateActorXAxisBtn_Click                  - Chapter 20, page 786
        //
        private void RotateActorXAxisBtn_Click(object sender, EventArgs e)
        {
            if (m_treeView.SelectedNode != m_mainNode)
            {
                float degrees = float.Parse(m_rotateActorXAxisDegreesTextbox.Text);
                //m_parent.RotateObjectX(UInt32.Parse(m_treeView.SelectedNode.Name), (float)(Math.PI * degrees / 180.0));
                NativeMethods.RotateActorXAxis(UInt32.Parse(m_treeView.SelectedNode.Name), (float)(Math.PI * degrees / 180.0));
            }
        }

        private void RotateActorYAxisBtn_Click(object sender, EventArgs e)
        {
            if (m_treeView.SelectedNode != m_mainNode)
            {
                float degrees = float.Parse(m_rotateActorYAxisDegreesTextbox.Text);
                //m_parent.RotateObjectY(UInt32.Parse(m_treeView.SelectedNode.Name), (float)(Math.PI * degrees / 180.0));
                NativeMethods.RotateActorYAxis(UInt32.Parse(m_treeView.SelectedNode.Name), (float)(Math.PI * degrees / 180.0));
            }
        }

        private void RemoveActorBtn_Click(object sender, EventArgs e)
        {
            if (m_treeView.SelectedNode != m_mainNode)
            {
                UInt32 id = UInt32.Parse(m_treeView.SelectedNode.Name);
                m_mainNode.Nodes.Remove(m_treeView.SelectedNode);
                //m_parent.RemoveActor(id);
                NativeMethods.RemoveActor(id);
            }
        }

        private void AddActorBtn_Click(object sender, EventArgs e)
        {
            DialogResult result = m_createForm.ShowDialog();
            if (result == DialogResult.OK)
            {
                try
                {
                    NativeMethods.CreateActor((int)m_createForm.GetParams().m_actorType, (UInt32)m_createForm.GetParams().m_color.ToArgb(), m_createForm.GetParams().m_length, m_createForm.GetParams().m_radius, m_createForm.GetParams().m_effectFile, m_createForm.GetParams().m_meshFile);
                    InitScene();
                }
                catch { }
            }
            m_createForm.Hide();
        }

        private void SaveLevelToolStripMenuItem_Click(object sender, EventArgs e)
        {
            DialogResult res = SaveFileDialog.ShowDialog();
            if (res != DialogResult.OK)
            {
                return;
            }
            String fileName = SaveFileDialog.FileName;
            NativeMethods.SaveLevel(fileName);
        }

        private void OpenLevelToolStripMenuItem_Click(object sender, EventArgs e)
        {
            DialogResult res = OpenFileDialog.ShowDialog();
            if (res != DialogResult.OK)
            {
                return;
            }
            String fileName = OpenFileDialog.FileName;
            NativeMethods.OpenLevel(fileName);
            InitScene();
        }

        private void m_treeView_AfterSelect(object sender, TreeViewEventArgs e)
        {

        }
    }
}