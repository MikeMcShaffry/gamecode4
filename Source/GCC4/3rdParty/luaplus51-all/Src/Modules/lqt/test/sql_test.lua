require 'qtcore'
require 'qtsql'

function Q(s) return QString.new_local(s) end

local db = QSqlDatabase.addDatabase(Q"QSQLITE", Q"conn1")
db:setDatabaseName(Q"numbers.db")
if not db:open() then
	local err = db:lastError()
	print('!!!', err:text():toLocal8Bit())
end
print('ok', db)

db:exec(Q"CREATE TABLE IF NOT EXISTS tab (n INT)")

local q = QSqlQuery.new_local(db)
q:prepare(Q"INSERT INTO tab VALUES (:n)")

for i=1,10 do
	q:bindValue(Q"n", QVariant.new_local(i))
	q:exec()
end

local q2 = QSqlQuery.new_local(db)
q2:exec(Q"SELECT * FROM tab")
while q2:next() do
	local v = q2:value(0)
	print(v:toInt())
end
