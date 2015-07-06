#!/usr/bin/lua
--[[

Copyright (c) 2007-2008 Mauro Iazzi

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

--]]


while false do
  local i = QLabel.__index
  QLabel.__index = function (...) print('QLabel required', ...) return i(...) end
end

if false then
local i = QPushButton.__index
QPushButton.__index = function (...) print(...) local ret = { i(...) } print(unpack(ret)) return unpack(ret) end
end

qt = {
  slot = function(s) return '1'..tostring(s) end,
  signal = function(s) return '2'..tostring(s) end,
}


--app = QApplication.new()

--print'====='
--print(app, app())

--QApplication.new = app


--print( app )
--print( pcall( app ) )

--[[
QApplication.new = app
--]]

--print( QApplication.new )
--print( pcall( QApplication.new ) )


app = QApplication.new()
--print(app)


mainwin = QWidget.new()
--print(mainwin)
mainwin:show()

layout = QVBoxLayout.new()

mainwin:setLayout(layout)
mainwin:setWindowTitle'lqt test window'

te = QTextEdit.new()
layout:addWidget(te)
te:setToolTip'I\'m the Code Edit Box: I hold the code while you edit'

--[==[
te:setPlainText[[
but = QPushButton.new()
but:show()
but:connect(qt.signal'pressed()', but, qt.slot'close()')
but:setText'Close Me!'
but:resize(150, 50)
but:setFont(QFont.new('Times', 18, 75))
]]
--]==]

te:setPlainText[[
buttons = buttons or {}
i = #buttons + 1

but = QPushButton.new()

but:show()
but:connect(qt.signal'pressed()', but, qt.slot'close()')
but:setText'Close Me!'
but:setWindowTitle('Button '..i)
but:resize(180, 50)
but:setFont(QFont.new('Times', 18, 75))

buttons[i] = but

return 'Created button '..i

]]
--te:show()

--print'----'

print'-----'
pb = QPushButton.new()
print'-----'
layout:addWidget(pb)
print'-----'
pb:setText'Exec'
print'-----'
pb:setToolTip'I\'m the Exec button: push me to execute your code'
print'-----'
--pb:show()

quit = QPushButton.new()
layout:addWidget(quit)
quit:setText'Quit'
quit:setToolTip'I\'m the Quit button: push me to close all windows'


--pb:connect('2pressed()', SLOT(function() print 'lqt RULES!' end))
--pb:connect('2pressed()', SLOT(function() print 'lqt ROCKS!' end))

results = QLabel.new()
results:setToolTip'I\'m the result bar: I will show you your results and your errors'
layout:addWidget(results)

pb:connect('2pressed()', SLOT(function()
  local f, err = loadstring(te:toPlainText())
  if f==nil then
    results:setText(err)
  else
    local res = { pcall( f ) }
    if res[1] then
      local string = ''
      for i, r in ipairs(res) do
        if i~=1 then string = string .. tostring(r) .. ', ' end
      end
      results:setText(string)
    else
      results:setText(res[2])
    end
  end
end))
quit:connect('2pressed()', app, '1closeAllWindows()')

app:exec()


