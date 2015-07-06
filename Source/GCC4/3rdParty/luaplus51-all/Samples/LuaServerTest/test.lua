str = "Hello"

table =
{
	yo = 5,
	anotherTable =
	{
		test = 10,
	}
}

function CalledFunc()
	local var = 5
	print("Called here.")
end

function Func()
	local var = 5
	local str = "Hello"
	local wstr = L"Wi\xfb33de string"
	local myTable =
	{
		val = 5
	}
	print('Hello')
	CalledFunc()
end

function Func2()
	print('Hello2')
	print('Hello3')
end


Func()
Func2()


