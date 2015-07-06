require "com"

ps = com.CreateObject("Photoshop.Application")

originalUnit = ps.preferences.rulerUnits;
ps.preferences.rulerUnits = ps.PsUnits.psInches

doc = ps.documents:add(4, 4)
artLayer = doc.artLayers:add()
artLayer.kind = ps.PsLayerKind.psTextLayer
artLayer.textItem.contents = "Hello, World!"

ps.preferences.rulerUnits = originalUnit