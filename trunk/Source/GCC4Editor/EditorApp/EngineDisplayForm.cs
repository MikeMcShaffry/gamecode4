//========================================================================
// EngineDisplayForm.cs - the user interface for the C# Editor
//
// Part of the GameCode4 Application
//
// GameCode4 is the sample application that encapsulates much of the source code
// discussed in "Game Coding Complete - 4th Edition" by Mike McShaffry and David
// "Rez" Graham, published by Charles River Media. 
// ISBN-10: 1133776574 | ISBN-13: 978-1133776574
//
// If this source code has found it's way to you, and you think it has helped you
// in any way, do the authors a favor and buy a new copy of the book - there are 
// detailed explanations in it that compliment this code well. Buy a copy at Amazon.com
// by clicking here: 
//    http://www.amazon.com/gp/product/1133776574/ref=olp_product_details?ie=UTF8&me=&seller=
//
// There's a companion web site at http://www.mcshaffry.com/GameCode/
// 
// The source code is managed and maintained through Google Code: 
//    http://code.google.com/p/gamecode4/
//
// (c) Copyright 2012 Michael L. McShaffry and David Graham
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser GPL v3
// as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See 
// http://www.gnu.org/licenses/lgpl-3.0.txt for more details.
//
// You should have received a copy of the GNU Lesser GPL v3
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
//========================================================================

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Collections;
using System.Windows.Forms;
using System.Runtime.InteropServices;
using System.Xml;

namespace EditorApp
{
    //========================================================================
    //
    // class EditorForm                      - Chapter 22, page 772
    //
    // The EditorForm handles the rendering for the editor, and as such,
    // is the main class in the EditorApp that interacts with the unmanaged dll.
    //
    //========================================================================

    public partial class EditorForm : Form
    {
        private const int INVALID_ACTOR_ID = -1;

        private string m_ProjectDirectory;
        private string m_AssetsDirectory;
        private string m_CurrentLevelFile;

        private List<XmlNode> m_ActorsXmlNodes = new List<XmlNode>();
        private int m_SelectedActorId = 0;

        private MessageHandler m_messageFilter;
        private ActorComponentEditor m_ActorComponentEditor;

        public EditorForm()
        {

            var currentDirectory = Directory.GetCurrentDirectory();
            var parentDirectory = Directory.GetParent(currentDirectory);
            m_ProjectDirectory = parentDirectory.FullName;
            m_AssetsDirectory = m_ProjectDirectory + "\\Assets";

            InitializeComponent();
            try
            {
                // This is how we get the instance handle for our C# app.
                System.IntPtr hInstance = Marshal.GetHINSTANCE(this.GetType().Module);
                
                // This is how we get the window handle for the panel we'll be rendering into.
                IntPtr hwnd = this.DisplayPanel.Handle;

                // Call into our Dll main function, which will set up an instance of the EditorApp project.
                NativeMethods.EditorMain(hInstance, IntPtr.Zero, hwnd, 1, this.DisplayPanel.Width, this.DisplayPanel.Height);

                InitializeAssetTree();

                m_messageFilter = new MessageHandler(this.Handle, this.DisplayPanel.Handle, this);
                m_ActorComponentEditor = new ActorComponentEditor(Panel_ActorComponents, m_ProjectDirectory);
            }
            catch (Exception e)
            {
                MessageBox.Show("Error: " + e.ToString());
                // If something goes wrong with the initialization, free the library
                // TODO QT: Why is this commented out???? Maybe Quoc knows...
                // NativeMethods.FreeLibrary(m_pDll);
            }
        }


        public EditorApp.MessageHandler GetMessageHandler()
        {
            return m_messageFilter;
        }

        private void Shutdown()
        {
            NativeMethods.Shutdown();
        }

        //
        // SetEditorReadyForUse             - Chapter 20, page 760
        //
        /***
        public void SetEditorReadyForUse()
        {
            InitializeAssetTree();
            InitializeActors();
        }
         * ***/

        // Asset Tree Methods
        private void InitializeAssetTree()
        {
            TreeView_Assets.Nodes.Clear();

            var stack = new Stack<TreeNode>();
            var rootDirectory = new DirectoryInfo(m_AssetsDirectory);
            var node = new TreeNode(rootDirectory.Name) { Tag = rootDirectory };
            stack.Push(node);

            while (stack.Count > 0)
            {
                var currentNode = stack.Pop();
                var directoryInfo = (DirectoryInfo)currentNode.Tag;
                foreach (var directory in directoryInfo.GetDirectories())
                {
                    FileAttributes attributes = File.GetAttributes(directory.FullName);
                    if ((attributes & FileAttributes.Hidden) == 0 )
                    {
                        var childDirectoryNode = new TreeNode(directory.Name);
                        childDirectoryNode.Tag = directory;
                        currentNode.Nodes.Add(childDirectoryNode);
                        stack.Push(childDirectoryNode);
                    }
                }
                foreach (var file in directoryInfo.GetFiles())
                {
                    FileAttributes attributes = File.GetAttributes(file.FullName);
                    if ((attributes & FileAttributes.Hidden) ==  0 )
                    {
                        var childNode = new TreeNode(file.Name);
                        childNode.Tag = file.FullName;
                        currentNode.Nodes.Add(childNode);
                    }
                }
            }

            TreeView_Assets.Nodes.Add(node);
        }


        private void TreeView_Assets_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            TreeNode node = TreeView_Assets.SelectedNode;
            if (node != null && node.Nodes.Count == 0)
            {
                string file = node.Tag.ToString();
                if (File.Exists(file))
                {
                    Process.Start(file);
                }
            }
        }




        private void buildProjectToolStripMenuItem_MouseUp(object sender, MouseEventArgs e)
        {
            ZipFileUtility.Create(m_AssetsDirectory, m_ProjectDirectory + "\\Assets.zip");
        }

        private XmlElement GetActorXml(uint actorId)
        {
            int xmlSize = NativeMethods.GetActorXmlSize(actorId);
            if (xmlSize == 0)
                return null;

            IntPtr tempArray = Marshal.AllocCoTaskMem((xmlSize + 1) * sizeof(char));
            NativeMethods.GetActorXml(tempArray, actorId);
            string actorXml = Marshal.PtrToStringAnsi(tempArray);
            Console.WriteLine(actorXml);
            Marshal.FreeCoTaskMem(tempArray);

            XmlDocument actorDoc = new XmlDocument();
            actorDoc.Load(new StringReader(actorXml));
            return actorDoc.DocumentElement;
        }

        // Actor List Methods
        private void InitializeActors()
        {
            TreeView_Actors.Nodes.Clear();

            int[] actorList = GetActorList();

            m_ActorsXmlNodes.Add(null);             // Game starts actors at Id=1, so we'll make a space for a null actor here.      

            // Add each actor as its own node in the treeview.
            for (int i = 0; i < actorList.GetLength(0); i++)
            {
                uint actorId = Convert.ToUInt32(actorList[i]);

                TreeNode node = new TreeNode();

                XmlElement actorXml = GetActorXml(actorId);
                if (actorXml != null)
                {
                    node.Name = actorList[i].ToString();
                    m_ActorsXmlNodes.Add(actorXml);
                    node.Text = actorXml.GetAttribute("type");
                }
                else
                {
                    node.Text = "<undefined actor - no xml>";
                }
                TreeView_Actors.Nodes.Add(node);
            }
        }

        //
        // GetActorList                         - Chapter 20, page 761
        //
        private int[] GetActorList()
        {
            // We need to know how many actors there are,
            // in order to find out how much memory to allocate
            int numActors = NativeMethods.GetNumActors();

            IntPtr tempArray = Marshal.AllocCoTaskMem(numActors * sizeof(uint));
            NativeMethods.GetActorList(tempArray, numActors);

            // Copy the memory into an array of ints and dispose of our
            // our memory.
            int[] actorList = new int[numActors];
            Marshal.Copy(tempArray, actorList, 0, numActors);
            Marshal.FreeCoTaskMem(tempArray);

            return actorList;
        }

        //
        // SelectActor                              - Chapter 20, page 761
        //
        public void PickActor()
        {
            IntPtr hWnd = this.DisplayPanel.Handle;
            int actorId = NativeMethods.PickActor(hWnd);
            if (actorId != INVALID_ACTOR_ID)
            {
                // TODO Mike - renable Actor Picking...
            //    m_actorPropertiesForm.SelectTreeNode(actorId);
            }
        }

        private void TreeView_Actors_AfterSelect(object sender, TreeViewEventArgs e)
        {
            TreeNode node = TreeView_Actors.SelectedNode;
            if (node != null)
            {
                m_SelectedActorId = node.Index + 1;                 // Game starts Actor Ids at 1
                m_ActorComponentEditor.ShowActorComponents(m_SelectedActorId, m_ActorsXmlNodes[m_SelectedActorId]);
            }
        }

        // Main Menu Methods

        private void openLevelToolStripMenuItem_Click(object sender, EventArgs e)
        {
            OpenFileDialog dialog = new OpenFileDialog();

            dialog.InitialDirectory = m_AssetsDirectory + "\\World";
            dialog.Filter = "XML files (*.xml)|*.xml";
            dialog.FilterIndex = 1;
            dialog.RestoreDirectory = true;

            if (dialog.ShowDialog() == DialogResult.OK)
            {
                m_CurrentLevelFile = dialog.FileName;
                NativeMethods.OpenLevel(m_CurrentLevelFile);
                InitializeActors();
            }
        }

        private void closeProjectToolStripMenuItem_Click(object sender, EventArgs e)
        {

        }

        private void saveLevelToolStripMenuItem_Click(object sender, EventArgs e)
        {
            XmlDocument levelXml = new XmlDocument();
            XmlElement world = levelXml.CreateElement("World");
            levelXml.AppendChild(world);
            XmlElement staticActors = levelXml.CreateElement("StaticActors");
            world.AppendChild(staticActors);

            int[] actorList = GetActorList();

            for (int i = 0; i < actorList.GetLength(0); i++)
            {
                uint actorId = Convert.ToUInt32(actorList[i]);
                XmlElement actorXml = GetActorXml(actorId);
                if (actorXml != null)
                {
                    staticActors.AppendChild(staticActors.OwnerDocument.ImportNode(actorXml, true));
                }
            }
            
            // Save the document to a file and auto-indent the output.

            XmlTextWriter writer = new XmlTextWriter(m_AssetsDirectory + "\\World\\New.xml", null);
            // TODO: There are some things currently not actors, which the editor doesn't know how to save!
            // When those are actor-ified I can fee safe to uncomment this and overwrite good level files.
            // XmlTextWriter writer = new XmlTextWriter(m_CurrentLevelFile, null);
            writer.Formatting = Formatting.Indented;
            levelXml.Save(writer);
        }

        private void buildProjectToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ZipFileUtility.Create(m_AssetsDirectory, m_ProjectDirectory + "\\Assets.zip");
        }
   }
}