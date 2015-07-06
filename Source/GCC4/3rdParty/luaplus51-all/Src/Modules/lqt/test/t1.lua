#!/usr/bin/lua

require'qtcore'
require'qtgui'

app = QApplication.new(1 + select('#', ...), {arg[0], ...})
app.__gc = app.delete -- take ownership of object

hello = QPushButton.new(QString.new("Hello World!"))
hello:resize(100, 30)

hello:show()

app.exec()


