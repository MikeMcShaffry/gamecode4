//========================================================================
// ActorComponentEditor.cs - implements the Actor Component Window in the C# Editor
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

    public partial class EditorForm : Form
    {
        //----------------------------------------------------------------
        // ActorComponentEditor                 - Chapter 22, page 784
        //----------------------------------------------------------------

        class ActorComponentEditor
        {
            Dictionary<string, XmlNode> m_ComponentsByName;
            XmlDocument m_SelectedActorComponents;
            int m_SelectedActorId;
            XmlNode m_ActorXml;
            string m_AssetsDirectory;

            const int g_LabelColumnWidth = 160;
            int m_LineSpacing;
            Panel m_Panel;

            public ActorComponentEditor(Panel panel, string projectDirectory)
            {
                m_ComponentsByName = new Dictionary<string, XmlNode>();

                m_Panel = panel;
                m_LineSpacing = m_Panel.Font.Height * 2;

                m_AssetsDirectory = projectDirectory + "\\Assets";

                XmlDocument componentsXML = new XmlDocument();
                componentsXML.Load(m_AssetsDirectory + "\\Editor\\components.xml");

                XmlElement root = componentsXML.DocumentElement;
                XmlNodeList components = root.SelectNodes("child::*");
                foreach (XmlNode component in components)
                {
                    m_ComponentsByName[component.Attributes["name"].Value] = component;
                }
            }

            //----------------------------------------------------------------
            // ShowActorComponents                 - Chapter 22, page 789
            //----------------------------------------------------------------
            public unsafe void ShowActorComponents(int selectedActorId, XmlNode actorXml)
            {
                m_SelectedActorId = selectedActorId;
                m_ActorXml = actorXml;

                m_SelectedActorComponents = new XmlDocument();
                XmlNode editorComponents = m_SelectedActorComponents.CreateElement("Actor");
                m_SelectedActorComponents.AppendChild(editorComponents);

                m_Panel.Controls.Clear();

                XmlNodeList actorValueComponents = m_ActorXml.SelectNodes("child::*");
                int lineNum = 0;
                foreach (XmlNode actorValueComponent in actorValueComponents)
                {
                    // [mrmike] - if you crash right here you have a component that you've never defined in Editor\components.xml
                    XmlNode sourceEditorComponent = m_ComponentsByName[actorValueComponent.Name];
                    XmlDocument ownerDoc = editorComponents.OwnerDocument;
                    XmlNode editorComponent = ownerDoc.ImportNode(sourceEditorComponent, true);
                    editorComponents.AppendChild(editorComponent);
                    lineNum = AddComponentUI(actorValueComponent, editorComponent, lineNum);
                }
            }

            //----------------------------------------------------------------
            // FindEditorElementFromXPath               - Chapter 22, page 793
            //    Finds the definition of the element as stored in Editor\components.xml
            //----------------------------------------------------------------
            private XmlNode FindEditorElementFromXPath(string xpath)
            {
                XmlNode root = m_SelectedActorComponents.FirstChild;
                XmlNodeList nodeList = root.SelectNodes(xpath);
                return nodeList[0];
            }

            //----------------------------------------------------------------
            // FindActorElementFromXPath                - Chapter 22, page 794
            //     Finds the XML of the actor element
            //----------------------------------------------------------------
            private XmlNode FindActorElementFromXPath(string xpath)
            {
                XmlNodeList nodeList = m_ActorXml.SelectNodes(xpath);
                return nodeList[0];
            }


            //----------------------------------------------------------------
            // AddComponentUI                 - Chapter 22, page 790
            //----------------------------------------------------------------

            public int AddComponentUI(XmlNode actorComponentValues, XmlNode editorComponentValues, int lineNum)
            {
                string componentName = actorComponentValues.Name.ToString();
                string componentXpath = XPathUtility.GetXPathToNode(actorComponentValues);
                try
                {
                    AddElementLabel(componentName, lineNum);
                    ++lineNum;
                    int elementNum = 0;

                    foreach (XmlNode inputField in editorComponentValues)
                    {
                        // FUTURE WORK - We should check for Comment style nodes here and ignore them. 
                        //               Putting comments in XML breaks this code!

                        string xpath = XPathUtility.GetXPathToNode(inputField);
                        string elementName = inputField.Attributes["name"].Value;
                        string elementType = inputField.Attributes["type"].Value;

                        XmlNode actorValues = actorComponentValues.ChildNodes[elementNum];

                        AddElementLabel("   " + elementName, lineNum);

                        switch (elementType)
                        {
                            case "int":
                            case "float":
                                string format = (elementType == "int") ? "0" : "0.000";
                                AddNum(actorValues, xpath, format, lineNum);
                                ++lineNum;
                                break;

                            case "Vec3":
                                AddVec3(actorValues, xpath, lineNum);
                                ++lineNum;
                                break;

                            case "RGBA":
                                AddRGBA(actorValues, xpath, lineNum);
                                ++lineNum;
                                break;

                            case "File":
                                AddFileElement(actorValues, xpath, lineNum);
                                ++lineNum;
                                break;

                            case "PhysicsShape":
                                List<string> physicsShapes = new List<string> { "Box", "Sphere" };
                                AddCombobox(xpath, ref physicsShapes, lineNum);
                                ++lineNum;
                                break;

                            case "PhysicsDensity":
                            case "PhysicsMaterial":
                            case "boolean":
                                AddElementLabel("   " + elementName + ": " + elementType, lineNum);
                                ++lineNum;
                                break;

                            default:                  
                                AddElementLabel("   " + elementName + ": " + elementType + " (unknown!)", lineNum);
                                ++lineNum;
                                break;
                        }

                        ++elementNum;
                    }
                }
                catch (Exception e)
                {
                    MessageBox.Show("Error in ComponentName " + componentName + "\n" + e.ToString());
                }

                return lineNum;
            }



            //----------------------------------------------------------------
            // AddFileElement                 - Chapter 22, page 791
            //----------------------------------------------------------------

            public void AddFileElement(XmlNode actorValues, string elementName, int lineNum)
            {
                const int boxWidth = 160;
                const int horizSpacing = 20;

                System.Windows.Forms.TextBox textBox = new System.Windows.Forms.TextBox();
                System.Drawing.Point location = new System.Drawing.Point(g_LabelColumnWidth, lineNum * m_LineSpacing);
                textBox.Name = elementName;
                textBox.Location = location;
                textBox.Text = actorValues.FirstChild.Value;
                textBox.TextChanged += new EventHandler(FileElementChanged);
                textBox.ReadOnly = true;
                m_Panel.Controls.Add(textBox);

                System.Windows.Forms.Button button = new System.Windows.Forms.Button();
                location = new System.Drawing.Point(g_LabelColumnWidth + boxWidth + horizSpacing, lineNum * m_LineSpacing);
                button.Name = elementName + "Button";
                button.Text = "Browse...";
                button.Location = location;
                button.MouseClick += new MouseEventHandler(SelectFile);
                m_Panel.Controls.Add(button);
            }

            //----------------------------------------------------------------
            // SelectFile                           - Chapter 22, page 793
            //----------------------------------------------------------------
            private void FileElementChanged(object sender, EventArgs e)
            {
                TextBox textBox = (TextBox)sender;
                string xPath = textBox.Name;
                string newValue = textBox.Text;

                XmlDocument xmlDoc = new XmlDocument();
                XmlElement xmlActor = xmlDoc.CreateElement("Actor");
                xmlDoc.AppendChild(xmlActor);

                XmlAttribute xmlActorId = xmlDoc.CreateAttribute("id");
                xmlActorId.InnerText = m_SelectedActorId.ToString();
                xmlActor.Attributes.Append(xmlActorId);

                XmlNode elementNode = FindActorElementFromXPath(xPath);
                XmlNode componentNode = elementNode.ParentNode;

                string componentName = componentNode.Name;
                string elementName = elementNode.Name;

                XmlElement xmlComponent = xmlDoc.CreateElement(componentName);
                xmlActor.AppendChild(xmlComponent);

                XmlElement xmlElementName = xmlDoc.CreateElement(elementName);
                xmlComponent.AppendChild(xmlElementName);

                xmlElementName.InnerText = newValue;

                NativeMethods.ModifyActor(xmlDoc.InnerXml);
            }

            //----------------------------------------------------------------
            // SelectFile                           - Chapter 22, page 792
            //----------------------------------------------------------------
            private void SelectFile(object sender, MouseEventArgs e)
            {
                OpenFileDialog openFile = new OpenFileDialog();
                Button button = (Button)sender;
                string buttonDesc = "Button";
                string textBoxElementName = button.Name.Substring(0, button.Name.Length - buttonDesc.Length);

                XmlNode fileElement = FindEditorElementFromXPath(textBoxElementName);

                openFile.Filter = fileElement.Attributes["extensions"].Value;
                openFile.ShowDialog();
                if (openFile.FileNames.Length > 0)
                {
                    try
                    {
                        string fileName = openFile.FileNames[0];
                        if (fileName.StartsWith(m_AssetsDirectory))
                        {
                            TextBox textBox = (TextBox)m_Panel.Controls[textBoxElementName];
                            textBox.Text = fileName.Substring(m_AssetsDirectory.Length + 1);
                        }
                        else
                        {
                            MessageBox.Show("Error - This file isn't a part of this project (it must be in " + m_AssetsDirectory + ").");
                        }
                    }
                    catch
                    {
                        MessageBox.Show("ElementName is incorrect in SelectFile");
                    }
                }
            }


            //----------------------------------------------------------------
            // The rest of the code in this file follows a similar structure to 
            // that of AddFileElement. Look at it first and get a handle on how it
            // works, then take a look at how other types of data can be presented, hooked
            // up to actor data, and changed with the editor interface

            public void AddElementLabel(string labelText, int lineNum)
            {
                System.Windows.Forms.Label label = new System.Windows.Forms.Label();
                System.Drawing.Point location = new System.Drawing.Point(0, lineNum * m_LineSpacing);
                label.Location = location;
                //                label.Height = m_Panel.Font.Height;
                label.Width = g_LabelColumnWidth;
                label.Text = labelText;
                m_Panel.Controls.Add(label);
            }


            protected void AddNum(XmlNode actorValues, string xpath, string format, int lineNum)
            {
                const int boxWidth = 60;

                System.Windows.Forms.TextBox textBox = new System.Windows.Forms.TextBox();
                System.Drawing.Point location = new System.Drawing.Point(g_LabelColumnWidth, lineNum * m_LineSpacing);
                textBox.Name = xpath;

                string actorValue = actorValues.FirstChild.Value;
                textBox.Text = actorValue;
                textBox.Location = location;
                textBox.TextAlign = HorizontalAlignment.Right;
                textBox.Leave += new EventHandler(NumElementChanged);

                textBox.Width = boxWidth;
                m_Panel.Controls.Add(textBox);
            }

            private void NumElementChanged(object sender, EventArgs e)
            {
                try 
                {
                    TextBox textBox = (TextBox)sender;
                    string xPath = textBox.Name;
                    string newValue = textBox.Text;

                    XmlDocument xmlDoc = new XmlDocument();
                    XmlElement xmlActor = xmlDoc.CreateElement("Actor");
                    xmlDoc.AppendChild(xmlActor);

                    XmlAttribute xmlActorId = xmlDoc.CreateAttribute("id");
                    xmlActorId.InnerText = m_SelectedActorId.ToString();
                    xmlActor.Attributes.Append(xmlActorId);

                    XmlNode elementNode;
                    XmlNode node = FindActorElementFromXPath(xPath);
                    if (node.ParentNode == null)
                    {
                        XmlAttribute attribute = (XmlAttribute)node;
                        elementNode = attribute.OwnerElement;

                        attribute.Value = newValue;
                    }
                    else
                    {
                        elementNode = node;
                        elementNode.InnerText = newValue;
                    }

                    XmlNode componentNode = elementNode.ParentNode;

                    string componentName = componentNode.Name;
                    string elementName = elementNode.Name;

                    XmlElement xmlComponent = xmlDoc.CreateElement(componentName);
                    xmlActor.AppendChild(xmlComponent);

                    XmlNode newElementNode = xmlDoc.ImportNode(elementNode, true);

                    xmlComponent.AppendChild(newElementNode);

                    NativeMethods.ModifyActor(xmlDoc.InnerXml);

                }
                catch (Exception ex)
                {
                    Debug.WriteLine(ex);
                }
            }

            public void AddVec3(XmlNode actorValues, string xpath, int lineNum)
            {
                const int horizSpacing = 10;
                const int boxWidth = 60;

                XmlNode fieldsElement = FindEditorElementFromXPath(xpath);
                string fieldNames = fieldsElement.Attributes["fieldNames"].Value;
                string[] fields = fieldNames.Split(',');

                for (int i = 0; i < 3; ++i)
                {
                    TextBox textBox = new TextBox();
                    System.Drawing.Point location = new System.Drawing.Point(g_LabelColumnWidth + (i * boxWidth + horizSpacing), lineNum * m_LineSpacing);
                    textBox.Name = xpath + "/@" + fields[i];

                    float actorValue = Convert.ToSingle(actorValues.Attributes[fields[i]].Value);
                    textBox.Text = String.Format("{0:0.###}", actorValue);
                    textBox.Location = location;
                    textBox.TextAlign = HorizontalAlignment.Right;
                    textBox.Leave += new EventHandler(NumElementChanged);

                    textBox.Width = boxWidth;

                    m_Panel.Controls.Add(textBox);
                }
            }

            public void AddCombobox(string elementName, ref List<string> values, int lineNum)
            {
                //                const int comboBoxWidth = 160;
                System.Windows.Forms.ComboBox comboBox = new System.Windows.Forms.ComboBox();
                System.Drawing.Point location = new System.Drawing.Point(g_LabelColumnWidth, lineNum * m_LineSpacing);
                comboBox.Name = elementName;
                comboBox.Location = location;
                //                comboBox.Width = comboBoxWidth;
                //                comboBox.Height = m_Panel.Font.Height;
                foreach (string s in values)
                {
                    comboBox.Items.Add(s);
                }
                m_Panel.Controls.Add(comboBox);
            }

            public void AddRGBA(XmlNode actorValues, string elementName, int lineNum)
            {
                const int boxWidth = 40;
                const int horizSpacing = 20;

                System.Windows.Forms.TextBox textBox = new System.Windows.Forms.TextBox();
                System.Drawing.Point location = new System.Drawing.Point(g_LabelColumnWidth, lineNum * m_LineSpacing);
                textBox.Name = elementName;
                textBox.Location = location;
                textBox.Width = boxWidth;
                textBox.Text = " ";

                float r = Convert.ToSingle(actorValues.Attributes["r"].Value);
                float g = Convert.ToSingle(actorValues.Attributes["g"].Value);
                float b = Convert.ToSingle(actorValues.Attributes["b"].Value);
                // FUTURE WORK - We need a text box to enter Alpha values 
                float a = Convert.ToSingle(actorValues.Attributes["a"].Value);

                textBox.BackColor = Color.FromArgb((int)(a * 255.0), (int)(r * 255.0), (int)(g * 255.0), (int)(b * 255.0));
                textBox.TextChanged += new EventHandler(RGBAElementTextChanged);
                textBox.BackColorChanged += new EventHandler(RGBAElementChanged);
                m_Panel.Controls.Add(textBox);

                System.Windows.Forms.Button button = new System.Windows.Forms.Button();
                location = new System.Drawing.Point(g_LabelColumnWidth + boxWidth + horizSpacing, lineNum * m_LineSpacing);
                button.Name = elementName + "Button";
                button.Text = "Choose...";
                button.Location = location;
                button.MouseClick += new MouseEventHandler(SelectRGBA);
                m_Panel.Controls.Add(button);
            }


            private void RGBAElementTextChanged(object sender, EventArgs e)
            {
                TextBox textBox = (TextBox)sender;
                textBox.Text = "";
            }

            private void RGBAElementChanged(object sender, EventArgs e)
            {
                TextBox textBox = (TextBox)sender;
                string xPath = textBox.Name;
                string newValue = textBox.Text;

                XmlDocument xmlDoc = new XmlDocument();
                XmlElement xmlActor = xmlDoc.CreateElement("Actor");
                xmlDoc.AppendChild(xmlActor);

                XmlAttribute xmlActorId = xmlDoc.CreateAttribute("id");
                xmlActorId.InnerText = m_SelectedActorId.ToString();
                xmlActor.Attributes.Append(xmlActorId);

                XmlNode elementNode = FindActorElementFromXPath(xPath);
                XmlNode componentNode = elementNode.ParentNode;

                string componentName = componentNode.Name;
                string elementName = elementNode.Name;

                XmlElement xmlComponent = xmlDoc.CreateElement(componentName);
                xmlActor.AppendChild(xmlComponent);

                XmlElement xmlElementName = xmlDoc.CreateElement(elementName);
                xmlComponent.AppendChild(xmlElementName);

                string format = "{0:0.00}";
                XmlAttribute rAttribute = xmlDoc.CreateAttribute("r");
                float r = Convert.ToSingle(textBox.BackColor.R) / 255.0f;
                rAttribute.Value = String.Format(format, r.ToString());
                xmlElementName.Attributes.Append(rAttribute);

                XmlAttribute gAttribute = xmlDoc.CreateAttribute("g");
                float g = Convert.ToSingle(textBox.BackColor.G) / 255.0f;
                gAttribute.Value = String.Format(format, g.ToString());
                xmlElementName.Attributes.Append(gAttribute);

                XmlAttribute bAttribute = xmlDoc.CreateAttribute("b");
                float b = Convert.ToSingle(textBox.BackColor.B) / 255.0f;
                bAttribute.Value = String.Format(format, b.ToString());
                xmlElementName.Attributes.Append(bAttribute);

                XmlAttribute aAttribute = xmlDoc.CreateAttribute("a");
                float a = Convert.ToSingle(textBox.BackColor.A) / 255.0f;
                aAttribute.Value = String.Format(format, a.ToString());
                xmlElementName.Attributes.Append(aAttribute);

                NativeMethods.ModifyActor(xmlDoc.InnerXml);
            }

            private void SelectRGBA(object sender, MouseEventArgs e)
            {
                Button button = (Button)sender;
                string buttonDesc = "Button";
                string textBoxElementName = button.Name.Substring(0, button.Name.Length - buttonDesc.Length);
                TextBox textBox = (TextBox)m_Panel.Controls[textBoxElementName];

                ColorDialog MyDialog = new ColorDialog();
                // Keeps the user from selecting a custom color.
                MyDialog.AllowFullOpen = true;
                // Allows the user to get help. (The default is false.)
                MyDialog.ShowHelp = true;
                // Sets the initial color select to the current text color.
                MyDialog.Color = textBox.BackColor;

                // Update the text box color if the user clicks OK 
                if (MyDialog.ShowDialog() == DialogResult.OK)
                {
                    textBox.BackColor = MyDialog.Color;
                }
            }
        };
    }
}