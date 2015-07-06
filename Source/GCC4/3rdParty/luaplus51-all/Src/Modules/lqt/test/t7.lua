#!/usr/bin/lua

require'qtcore'
require'qtgui'

local LCD_Range = function(...)
	local this = QWidget.new(...)
	--print(this:metaObject():className(), this:metaObject():methodCount())
	--print(this:metaObject():className(), this:metaObject():methodCount())

	local lcd = QLCDNumber.new()
	lcd:setSegmentStyle'Filled'

	local slider = QSlider.new'Horizontal'
	slider:setRange(0, 99)
	slider:setValue(0)

	this:__addmethod('valueChanged(int)')
	this:__addmethod('setValue(int)', function(_, val) slider:setValue(val) end)
	QObject.connect(slider, '2valueChanged(int)', lcd, '1display(int)')
	QObject.connect(slider, '2valueChanged(int)', this, '2valueChanged(int)')

	local layout = QVBoxLayout.new()
	layout:addWidget(lcd)
	layout:addWidget(slider)
	this:setLayout(layout)
	return this
end

local new_MyWidget = function(...)
	local this = QWidget.new(...)

	local quit = QPushButton.new(QString.new'Quit')
	quit:setFont(QFont.new(QString.new'Times', 18, 75))

	QObject.connect(quit, '2clicked()', this, '1close()')

	local grid = QGridLayout.new()
	local previousRange = nil
	for row = 1, 3 do
		for column = 1, 3 do
			local lcdrange = LCD_Range()
			grid:addWidget(lcdrange, row, column)
			if previousRange then
				QObject.connect(lcdrange, '2valueChanged(int)',
					previousRange, '1setValue(int)')
			end
			previousRange = lcdrange
		end
	end

	local layout = QVBoxLayout.new()
	layout:addWidget(quit)
	layout:addLayout(grid)
	this:setLayout(layout)
	return this
end

app = QApplication.new(1 + select('#', ...), {arg[0], ...})
app.__gc = app.delete -- take ownership of object

widget = new_MyWidget()
widget:show()

app.exec()


