TestNumber = 5
TestString = "Hello"

TestTable =
{
  Value1 = 5,
  Value2 = "Hi",
  Value3 = { [1] = 5, [100] = 50 },
  Value4 = { 1, 2, 3 },
  Value5 = { 1, 2, 3; VarA = 5, VarB = "String" },
}

NestedTableLevel1 =
{
	Level2 =
	{
		Hi = "Hi",
		Level3 =
		{
			Hi = "Hi",
			Level4 =
			{
				0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
			}
		}
	}
}

TestTable2 = CopyTable(TestTable)
TestTable2.Value5 = nil

