VectorMetaTable =
{
	FormattedWrite = function(file, value, alphabetical, indentLevel,
			maxIndentLevel, writeAll, writeTablePointers)
		LuaFilePrint(file, "{ ")

		for index = 1, 1000000 do
			local obj = value[index]
			if not obj then
				break
			end
			if index ~= 1 then
				LuaFilePrint(file, ", ")
			end
			LuaFilePrint(file, "%.16g", obj)
		end

		LuaFilePrint(file, " }");
	end
}

MatrixMetaTable =
{
	FormattedWrite = function(file, value, alphabetical, indentLevel,
			maxIndentLevel, writeAll, writeTablePointers)
		LuaFilePrint(file, "\n")
		LuaFileIndent(file, indentLevel)
		LuaFilePrint(file, "{\n")
		for i = 1, 4 do
			LuaFileIndent(file, indentLevel + 1)
			for j = 1, 4 do
				local obj = value[(i - 1) * 4 + j]
				LuaFilePrint(file, "%.16g, ", obj)
			end
			LuaFilePrint(file, "\n")
		end
		LuaFileIndent(file, indentLevel)
		LuaFilePrint(file, "}");
	end
}

Table =
{
	Vec = { 0, 1, 2 },
	Mat =
	{
		0, 1, 2, 3,
		4, 5, 6, 7,
		8, 9, 0, 1,
		2, 3, 4, 5,
	},
}

setmetatable(Table.Vec, VectorMetaTable)
setmetatable(Table.Mat, MatrixMetaTable)

LuaDumpFile("ScriptVectorDump.out", "Table", Table, true)
