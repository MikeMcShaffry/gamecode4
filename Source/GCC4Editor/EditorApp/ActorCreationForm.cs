//========================================================================
// ActorCreationForm.cs
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
using System.Windows.Forms;

namespace EditorApp
{
    //========================================================================
    // class ActorCreationForm                      - Chapter 20, page 782
    //
    // This form is used to gather information about new actors that will be
    // added to the scene.
    //
    //========================================================================

    public partial class ActorCreationForm : Form
    {
        EditorActorParams m_actorParams;

        public ActorCreationForm()
        {
            InitializeComponent();

            m_actorParams.m_color = Color.White;
            m_actorParams.m_length = 0;
            m_actorParams.m_radius = 0;
            m_actorParams.m_actorType = ActorType.AT_Unknown;

            m_actorTypeComboBox.Items.Add("Sphere");
            m_actorTypeComboBox.Items.Add("Grid");
            m_actorTypeComboBox.Items.Add("Teapot");
            m_actorTypeComboBox.Items.Add("Test Object");
            m_actorTypeComboBox.Items.Add("Generic Mesh Object");

            // Display the controls for modifying actor properties
            // only after the user has selected what type of actor
            // to create
            m_createActorBtn.Visible = false;
            m_cancelBtn.Visible = false;
            m_colorDisplayPanel.Visible = false;
            m_colorLabel.Visible = false;
            m_LengthTextbox.Visible = false;
            m_lengthLabel.Visible = false;
            m_RadiusTextbox.Visible = false;
            m_radiusLabel.Visible = false;
            m_effectFileLabel.Visible = false;
            m_meshFileLabel.Visible = false;
            m_effectFileTextbox.Visible = false;
            m_meshFileTextbox.Visible = false;
            m_browseEffectFileBtn.Visible = false;
            m_browseMeshFileBtn.Visible = false;
        }

        public EditorActorParams GetParams()
        {
            return m_actorParams;
        }

        private void ActorTypeComboBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            m_createActorBtn.Visible = false;
            m_cancelBtn.Visible = false;
            m_colorDisplayPanel.Visible = false;
            m_colorLabel.Visible = false;
            m_LengthTextbox.Visible = false;
            m_lengthLabel.Visible = false;
            m_RadiusTextbox.Visible = false;
            m_radiusLabel.Visible = false;
            m_effectFileLabel.Visible = false;
            m_meshFileLabel.Visible = false;
            m_effectFileTextbox.Visible = false;
            m_meshFileTextbox.Visible = false;
            m_browseEffectFileBtn.Visible = false;
            m_browseMeshFileBtn.Visible = false;

            // Display the appropriate controls for each actor type
            switch (m_actorTypeComboBox.Text)
            {
                case "Sphere":
                    m_createActorBtn.Visible = true;
                    m_cancelBtn.Visible = true;
                    m_colorDisplayPanel.Visible = true;
                    m_colorLabel.Visible = true;
                    m_RadiusTextbox.Visible = true;
                    m_radiusLabel.Visible = true;
                    break;
                case "Teapot":
                case "Test Object":
                    m_createActorBtn.Visible = true;
                    m_cancelBtn.Visible = true;
                    m_colorDisplayPanel.Visible = true;
                    m_colorLabel.Visible = true;
                    break;
                case "Grid":
                    m_createActorBtn.Visible = true;
                    m_cancelBtn.Visible = true;
                    m_colorDisplayPanel.Visible = true;
                    m_colorLabel.Visible = true;
                    m_LengthTextbox.Visible = true;
                    m_lengthLabel.Visible = true;
                    break;
                case "Generic Mesh Object":
                    m_createActorBtn.Visible = true;
                    m_cancelBtn.Visible = true;
                    m_colorDisplayPanel.Visible = true;
                    m_colorLabel.Visible = true;
                    m_effectFileLabel.Visible = true;
                    m_meshFileLabel.Visible = true;
                    m_effectFileTextbox.Visible = true;
                    m_meshFileTextbox.Visible = true;
                    m_browseEffectFileBtn.Visible = true;
                    m_browseMeshFileBtn.Visible = true;
                    break;
            }
        }


        //
        // CreateActorBtn_Click                 - Chapter 20, page 784
        //
        private void CreateActorBtn_Click(object sender, EventArgs e)
        {
            switch (m_actorTypeComboBox.Text)
            {
                case "Sphere":
                    m_actorParams.m_actorType = ActorType.AT_Sphere;
                    m_actorParams.m_radius = float.Parse(m_RadiusTextbox.Text);
                    m_actorParams.m_color = m_colorDisplayPanel.BackColor;
                    this.DialogResult = DialogResult.OK;
                    this.Close();
                    break;
                case "Teapot":
                    m_actorParams.m_actorType = ActorType.AT_Teapot;
                    m_actorParams.m_color = m_colorDisplayPanel.BackColor;
                    this.DialogResult = DialogResult.OK;
                    this.Close();
                    break;
                case "Test Object":
                    m_actorParams.m_actorType = ActorType.AT_TestObject;
                    m_actorParams.m_color = m_colorDisplayPanel.BackColor;
                    this.DialogResult = DialogResult.OK;
                    this.Close();
                    break;
                case "Grid":
                    m_actorParams.m_actorType = ActorType.AT_Grid;
                    m_actorParams.m_color = m_colorDisplayPanel.BackColor;
                    m_actorParams.m_length = float.Parse(m_LengthTextbox.Text);
                    this.DialogResult = DialogResult.OK;
                    this.Close();
                    break;
                case "Generic Mesh Object":
                    m_actorParams.m_actorType = ActorType.AT_GenericMeshObject;
                    m_actorParams.m_color = m_colorDisplayPanel.BackColor;
                    m_actorParams.m_effectFile = m_effectFileTextbox.Text;
                    m_actorParams.m_meshFile = m_meshFileTextbox.Text;
                    this.DialogResult = DialogResult.OK;
                    this.Close();
                    break;
                default:
                    MessageBox.Show("You must select an actor type!");
                    break;
            }
        }

        //
        // ColorDisplayPanel_MouseDown                  - Chapter 20, page 785
        //
        private void ColorDisplayPanel_MouseDown(object sender, MouseEventArgs e)
        {
            m_colorDialog.ShowDialog();
            m_colorDisplayPanel.BackColor = m_colorDialog.Color;
        }

        private void CancelBtn_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void BrowseEffectFileBtn_Click(object sender, EventArgs e)
        {
            DialogResult res = m_openFileDialog.ShowDialog();
            if (res != DialogResult.OK)
            {
                return;
            }
            m_effectFileTextbox.Text = m_openFileDialog.FileName;            
        }

        private void BrowseMeshFileBtn_Click(object sender, EventArgs e)
        {
            DialogResult res = m_openFileDialog.ShowDialog();
            if (res != DialogResult.OK)
            {
                return;
            }
            m_meshFileTextbox.Text = m_openFileDialog.FileName;            
        }
    }
}