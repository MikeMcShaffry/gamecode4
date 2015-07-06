require "dotnet"

dotnet.LoadAssembly("System.Windows.Forms")

Form = dotnet.Type("System.Windows.Forms.Form")
