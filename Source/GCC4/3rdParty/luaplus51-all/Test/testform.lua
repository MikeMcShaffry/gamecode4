load_assembly("System.Windows.Forms")
load_assembly("System.Drawing")

Form=import_type("System.Windows.Forms.Form")
TextBox=import_type("System.Windows.Forms.TextBox")
Label=import_type("System.Windows.Forms.Label")
ListBox=import_type("System.Windows.Forms.ListBox")
Button=import_type("System.Windows.Forms.Button")
Point=import_type("System.Drawing.Point")
Size=import_type("System.Drawing.Size")
ScrollBars=import_type("System.Windows.Forms.ScrollBars")
FormBorderStyle=import_type("System.Windows.Forms.FormBorderStyle")
FormStartPosition=import_type("System.Windows.Forms.FormStartPosition")

function clear_click(sender,args)
  code:Clear()
end

function execute_click(sender,args)
  results.Items:Clear()
  result=lua:DoString(code.Text)
  if result then
    for i=0,result.Length-1 do
      results.Items:Add(result[i])
    end
  end
end

function load_click(sender,args)
  open_file:ShowDialog()
  file=StreamReader(open_file.FileName)
  code.Text=file:ReadToEnd()
  file:Close()
end

NewForm = {}

function NewForm:OnTabIndexChanged(eventArgs)
	print("Tab index changed.")
	self.base:OnTabIndexChanged(eventArgs)
end

MyTextBox = {}

function MyTextBox:set_Multiline(multi)
	print("Set Multiline")
	self.base:set_Multiline(multi)
end

--form = Form()
form = NewForm
make_object(form, Form)
--code = TextBox()
label1 = Label()
execute = Button()
clear = Button()
results = ListBox()
label2 = Label()
load = Button()

form:SuspendLayout()

code = MyTextBox
make_object(code, TextBox)

code.Multiline = true
code.Location = Point(16, 24)
code.Name = "Code"
code.Size = Size(440, 128)
code.ScrollBars = ScrollBars.Vertical
code.TabIndex = 0
code.Text = ""

label1.Location = Point(16, 8)
label1.Name = "label1"
label1.Size = Size(100, 16)
label1.TabIndex = 1
label1.Text = "Lua Code:"

execute.Location = Point(96, 160)
execute.Name = "Execute"
execute.TabIndex = 2
execute.Text = "Execute"
execute.Click:Add(execute_click)

clear.Location = Point(176, 160)
clear.Name = "Clear"
clear.TabIndex = 3
clear.Text = "Clear"
clear.Click:Add(clear_click)

results.Location = Point(16, 208)
results.Name = "Results"
results.Size = Size(440, 95)
results.TabIndex = 4

label2.Location = Point(16, 192)
label2.Name = "label2"
label2.Size = Size(100, 16)
label2.TabIndex = 5
label2.Text = "Results:"

load.Location = Point(16, 160)
load.Name = "Load"
load.TabIndex = 6
load.Text = "Load..."
load.Click:Add(load_click)

open_file.DefaultExt = "lua"
open_file.Filter = "Lua Scripts|*.lua|All Files|*.*"
open_file.Title = "Pick a File"

function MyTabIndexChanged(e)
	print("The tab index has changed.")
end

form.AutoScaleBaseSize = Size(5, 13)
form.ClientSize = Size(472, 315)
form.Controls:Add(load)
form.Controls:Add(label2)
form.Controls:Add(results)
form.Controls:Add(clear)
form.Controls:Add(execute)
form.Controls:Add(label1)
form.Controls:Add(code)
form.TabIndexChanged:Add(MyTabIndexChanged)
form.Name = "MainForm"
form.Text = "LuaNet"
form.FormBorderStyle = FormBorderStyle.Fixed3D
form.StartPosition = FormStartPosition.CenterScreen
form:ResumeLayout(false)

form.TabIndex = 2

form:ShowDialog()
