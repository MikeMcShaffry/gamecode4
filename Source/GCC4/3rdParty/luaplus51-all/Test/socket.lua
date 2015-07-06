import "dotnet"

load_assembly("System")

WebClient=import_type("System.Net.WebClient")
StreamReader=import_type("System.IO.StreamReader")

myWebClient = WebClient()
myStream = myWebClient:OpenRead(arg[1])
sr = StreamReader(myStream)
line=sr:ReadLine()
repeat
  print(line)
  line=sr:ReadLine()
until not line
myStream:Close()
