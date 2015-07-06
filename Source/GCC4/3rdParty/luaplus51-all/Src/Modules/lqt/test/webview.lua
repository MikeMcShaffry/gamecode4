#!/usr/bin/lua

require'qtcore'
require'qtgui'
require'qtwebkit'

app = QApplication.new_local(1 + select('#', ...), {arg[0], ...})

local address = tostring(arg[1])

if address == 'nil' then
	address = 'www.lua.org'
end

url = QUrl.new(QString.new('http://'..address))

print('Loading site  '..address..' ...')

webView = QWebView.new_local()
webView:setUrl(url)
webView:show()

app.exec()



