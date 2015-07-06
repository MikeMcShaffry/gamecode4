import "com"

ps = com.CreateObject("Photoshop.Application")

originalUnit = ps.preferences.rulerUnits;
ps.preferences.rulerUnits = ps.PsUnits.psInches

ps.documents.add(4, 4, 72, "My New Document")
doc = ps.activeDocument
layer1 = doc.artLayers.add()
layer2 = doc.artLayers.add()
layer1.name = "This layer was first"

ps.preferences.rulerUnits = originalUnit