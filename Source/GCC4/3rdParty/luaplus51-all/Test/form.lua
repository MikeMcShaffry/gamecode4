import "dotnet"

load_assembly("System.Windows.Forms")
load_assembly("System")
--[[
--Convert = import_type("System.Convert")
Convert = dotnet["System.Convert"]
--AnchorStyles = import_type("System.Windows.Forms.AnchorStyles")
AnchorStyles = dotnet["System.Windows.Forms.AnchorStyles"]
print(Convert:ToInt32(AnchorStyles.Bottom))
--print(Convert)
--Int32 = import_type("System.Int32")
--ToInt32 = get_method_bysig(Convert, "ToInt32", Int32)
--print(ToInt32(5))

ToInt32 = get_method_bysig(Convert, "ToInt32", AnchorStyles)
print(ToInt32)
print(ToInt32(AnchorStyles.Bottom))
]]


load_assembly("System.Drawing")

Form=import_type("System.Windows.Forms.Form")
Button=import_type("System.Windows.Forms.Button")
Point=import_type("System.Drawing.Point")

form1=Form()
button1=Button()
button2=Button()

function handleClick(sender,data)
  if sender.Text=="OK" then
    sender.Text="Clicked"
  else
    sender.Text="OK"
  end
  button1.MouseUp:Remove(handler)
  print(sender:ToString())
end

button1.Text = "OK"
button1.Location=Point(10,10)
button2.Text = "Cancel"
button2.Location=Point(button1.Left, button1.Height + button1.Top + 10)
handler=button1.MouseUp:Add(handleClick)
form1.Text = "My Dialog Box"
form1.HelpButton = true
form1.MaximizeBox=false
form1.MinimizeBox=false
form1.AcceptButton = button1
form1.CancelButton = button2
form1.Controls:Add(button1)
form1.Controls:Add(button2)
form1:ShowDialog()

