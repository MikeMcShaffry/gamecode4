import "com"

ps = com.CreateObject("Photoshop.Application")
--pdfOpenOptions = com.NewObject(ps, "PDFOpenOptions")
pdfOpenOptions = com.CreateObject("Photoshop.PDFOpenOptions")
pdfOpenOptions.AntiAlias = true
--pdfOpenOptions.Height = 100
--pdfOpenOptions.Width = 200
pdfOpenOptions.mode = ps.PsOpenDocumentMode.psOpenRGB
pdfOpenOptions.Resolution = 72
pdfOpenOptions.ConstrainProportions = false
pdfOpenOptions.Page = 5

ps.open("f:\\Downloads\\luacom.pdf", pdfOpenOptions)

