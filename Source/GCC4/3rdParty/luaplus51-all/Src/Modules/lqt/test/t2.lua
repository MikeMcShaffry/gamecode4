#!/usr/bin/lua

require'qtcore'
require'qtgui'

app = QApplication.new(1 + select('#', ...), {arg[0], ...})
app.__gc = app.delete -- take ownership of object

quit = QPushButton.new(QString.new("Quit"))
quit:resize(75, 30)
quit:setFont(QFont.new(QString.new'Times', 18, 75))

QObject.connect(quit, '2clicked()', app, '1quit()')

quit:show()

app.exec()


