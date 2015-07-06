#!/usr/bin/lua

require'qtcore'
require'qtgui'

local new_MyWidget = function(...)
	local this = QWidget.new(...)

	local quit = QPushButton.new(QString.new'Quit')
	quit:setFont(QFont.new(QString.new'Times', 18, 75))

	local lcd = QLCDNumber.new()
	lcd:setSegmentStyle'Filled'

	local slider = QSlider.new'Horizontal'
	slider:setRange(0, 99)
	slider:setValue(0)

	QObject.connect(quit, '2clicked()', QCoreApplication.instance(), '1quit()')
	QObject.connect(slider, '2valueChanged(int)', lcd, '1display(int)')

	local layout = QVBoxLayout.new()
	layout:addWidget(quit)
	layout:addWidget(lcd)
	layout:addWidget(slider)
	this:setLayout(layout)
	return this
end

app = QApplication.new(1 + select('#', ...), {arg[0], ...})
app.__gc = app.delete -- take ownership of object

widget = new_MyWidget()
widget:show()

app.exec()


