LUA_PATH = "?;?.lua;../?;../?.lua"


require "sqlite3"


db = sqlite3.open_memory()

db:exec([[ 

  CREATE TABLE test (
    id		INTEGER PRIMARY KEY,
    content	VARCHAR
  );
  
]])


insert_stmt = assert( db:prepare("INSERT INTO test VALUES (NULL, ?)") )

function insert(data)
  insert_stmt:bind(data)
  insert_stmt:exec()
end


select_stmt = assert( db:prepare("SELECT * FROM test") )

function select()
  for row in select_stmt:rows() do
    print(row.id, row.content)
  end
end


insert("Hello World")
print("First:")
select()

insert("Hello Lua")
print("Second:")
select()

insert("Hello Sqlite3")
print("Third:")
select()


