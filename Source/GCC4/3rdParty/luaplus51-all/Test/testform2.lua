require "luanet"

luanet.load_assembly("System.Windows.Forms")
luanet.load_assembly("System.Drawing")

Form = luanet.import_type("System.Windows.Forms.Form")
ListView = luanet.import_type("System.Windows.Forms.ListView")
ColumnHeader = luanet.import_type("System.Windows.Forms.ColumnHeader")
ColumnHeaderStyle = luanet.import_type("System.Windows.Forms.ColumnHeaderStyle")
Point = luanet.import_type("System.Drawing.Point")
Size = luanet.import_type("System.Drawing.Size")
View = luanet.import_type("System.Windows.Forms.View")

form = Form()
listView = ListView()
form:SuspendLayout()

columnHeader = ColumnHeader()
columnHeader.Text = "Message";
columnHeader.Width = 800;

listView.Columns:Add(columnHeader)
listView.Location = Point(8, 8)
listView.Name = "listView1";
listView.Size = Size(280, 248)
listView.TabIndex = 0
listView.View = View.Details
listView.FullRowSelect = true
listView.HeaderStyle = ColumnHeaderStyle.None
listView.MultiSelect = false

form.AutoScaleBaseSize = Size(5, 13)
form.ClientSize = Size(292, 266)
form.Controls:Add(listView)
form.Name = "Form1"
form.Text = "Form1"

form:ResumeLayout(false)
form:ShowDialog()

