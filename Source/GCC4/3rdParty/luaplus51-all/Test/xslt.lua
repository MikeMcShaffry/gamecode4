import "dotnet"

load_assembly("System.Xml")

XslTransform = dotnet["System.Xml.Xsl.XslTransform"]
XmlDocument = dotnet["System.Xml.XmlDocument"]
XmlTextWriter = dotnet["System.Xml.XmlTextWriter"]

xslt = XslTransform()
xslt:Load("u:\\Downloads\\ex\\excel2csv.xsl")

doc = XmlDocument()
doc:Load("u:\\Downloads\\ex\\book.xml")

writer = XmlTextWriter("u:\\Out.csv", nil)

xslt:Transform(doc, nil, writer, nil)
writer:Close()

