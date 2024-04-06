#include "Misc/AutomationTest.h"
#include "Linq.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(LinqTest_Primary_From, "Private.LinqTest.From", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool LinqTest_Primary_From::RunTest(const FString& Parameters)
{
	TArray<int> Array{2, 4, 1, 5, 2, 3};
	int Index = 0;
	auto Result = From(Array);
	while (Result.MoveNext())
	{
		TestEqual(TEXT("From 순회 테스트"), Result.Current(), Array[Index++]);
	}
	
	Index = 0;
	Result = From(TArray<int> {2, 4, 1, 5, 2, 3});
	while (Result.MoveNext())
	{
		TestEqual(TEXT("From 순회 테스트2"), Result.Current(), Array[Index++]);
	}
	
	Index = 0;
	Result = From(2, 4, 1, 5, 2, 3);
	while (Result.MoveNext())
	{
		TestEqual(TEXT("From 순회 테스트3"), Result.Current(), Array[Index++]);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(LinqTest_Primary_From_Where, "Private.LinqTest.From_Where", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool LinqTest_Primary_From_Where::RunTest(const FString& Parameters)
{
	TArray<int> Array{2, 4, 1, 5, 2, 3};
	TArray<int> Expect{1, 5, 3};
	int Index = 0;
	auto Result = From(Array) >> Where([](int i) { return i % 2 == 1; });
	while (Result.MoveNext())
	{
		TestEqual(TEXT("Where 테스트"), Result.Current(), Expect[Index++]);
	}
	return true;
}

struct FTestStruct
{
public:
	static int Count;

	FTestStruct()
	{
		Count += 1;
	}
};

int FTestStruct::Count = 0;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(LinqTest_Struct_From_Where, "Private.LinqTest.From_Where_With_Struct", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool LinqTest_Struct_From_Where::RunTest(const FString& Parameters)
{
	int Count = FTestStruct::Count;
	TArray<FTestStruct> Array{FTestStruct(), FTestStruct()};
	auto Result = From(Array) >> Where([](const FTestStruct&) { return true; });
	while (Result.MoveNext())
	{
	}
	if (false == TestEqual(TEXT("From>>Where 불필요한 스택 생성 체크"), Count + 2, FTestStruct::Count))
	{
		return false;
	}
	
	Count = FTestStruct::Count;
	auto Result1 = From(TArray<FTestStruct> {FTestStruct(), FTestStruct()}) >> Where([](const FTestStruct&) { return true; });
	while (Result1.MoveNext())
	{
	}
	return TestEqual(TEXT("From>>Where 불필요한 스택 생성 체크"), Count + 2, FTestStruct::Count);
}

struct FTestStruct2
{
public:
	int Value = 0;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(LinqTest_From_Select, "Private.LinqTest.From_Select", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool LinqTest_From_Select::RunTest(const FString& Parameters)
{
	TArray<int> Array{2, 4, 1, 5, 2, 3};
	TArray<int> Expect = Array;

	auto Result = From(Array) >> Select([](const int X) -> int { return X; });
	int Index = 0;
	while (Result.MoveNext())
	{
		TestEqual(TEXT("Select 구현 테스트"), Result.Current(), Expect[Index++]);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(LinqTest_From_ToArray, "Private.LinqTest.From_To_Array", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool LinqTest_From_ToArray::RunTest(const FString& Parameters)
{
	TArray<int> Array{2, 4, 1, 5, 2, 3};
	TArray<int> Expect{2, 4, 1, 5, 2, 3};
	TArray<int> Result = From(Array) >> ToArray();
	for (int i = 0; i < Expect.Num(); ++i)
	{
		TestEqual(TEXT("ToArray 테스트"), Result[i], Expect[i]);
	}
	Result = From(Array) >> Select([](int X) { return X; }) >> ToArray();
	for (int i = 0; i < Expect.Num(); ++i)
	{
		TestEqual(TEXT("ToArray 테스트"), Result[i], Expect[i]);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(LinqTest_From_OrderBy_ThenBy, "Private.LinqTest.From_OrderBy_ThenBy", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool LinqTest_From_OrderBy_ThenBy::RunTest(const FString& Parameters)
{
	TArray<int> Array1{3, 2, 1, 4, 5};
	TArray<int> Expect1{1, 2, 3, 4, 5};
	TArray<int> Result1 = From(Array1) >> OrderBy([](int X) { return X; }) >> ToArray();
	for (int i = 0; i < Expect1.Num(); ++i)
	{
		TestEqual(TEXT("OrderBy 테스트"), Result1[i], Expect1[i]);
		TestEqual(TEXT("OrderBy 테스트"), Result1[i], Expect1[i]);
	}

	typedef TTuple<int, int> IntTuple;
	IntTuple a(2, 3);
	IntTuple b(2, 2);
	IntTuple c(1, 3);
	IntTuple d(1, 2);
	TArray<IntTuple> Array2{a, b, c, d};
	TArray<IntTuple> Expect2{d, c, b, a};
	TArray<IntTuple> Result2 = From(Array2) >> OrderBy([](const IntTuple& X) { return X.Key; }) >> ThenBy([](const IntTuple& X) { return X.Value; }) >> ToArray();

	for (int i = 0; i < Expect2.Num(); ++i)
	{
		TestEqual(TEXT("OrderBy >> ThenBy 테스트"), Result2[i].Key, Expect2[i].Key);
		TestEqual(TEXT("OrderBy >> ThenBy 테스트"), Result2[i].Value, Expect2[i].Value);
	}

	TArray<IntTuple> Array3;
	Array3.Reserve(1000);
	UE_LOG(LogTemp, Log, TEXT("OrdreBy Ramdom --------------------------------------"));
	for (int i = 0; i < 1000; ++i)
	{
		Array3.Emplace(IntTuple{FMath::RandHelper(1000), FMath::RandHelper(1000)});
		UE_LOG(LogTemp, Log, TEXT("OrdreBy Key: %d, Value: %d"), Array3[i].Key, Array3[i].Value);
	}
	TArray<IntTuple> Result3 = From(Array3) >> OrderBy([](const IntTuple& X) { return X.Key; }) >> ThenBy([](const IntTuple& X) { return X.Value; }) >> ToArray();
	IntTuple& Before = Result3[0];
	UE_LOG(LogTemp, Log, TEXT("OrdreBy Sorted --------------------------------------"));
	UE_LOG(LogTemp, Log, TEXT("OrdreBy Key: %d, Value: %d"), Before.Key, Before.Value);
	for (int i = 1; i < 1000; ++i)
	{
		TestTrue("OrderBy >> ThenBy 테스트", Before.Key <= Result3[i].Key);
		if (Before.Key == Result3[i].Key)
		{
			TestTrue("OrderBy >> ThenBy 테스트", Before.Value <= Result3[i].Value);
		}
		UE_LOG(LogTemp, Log, TEXT("OrdreBy Key: %d, Value: %d"), Result3[i].Key, Result3[i].Value);
		Before = Result3[i];
	}

	return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(LinqTest_From_Reserve, "Private.LinqTest.Reverse", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool LinqTest_From_Reserve::RunTest(const FString& Parameters)
{
	TArray<int> Array{2, 4, 1, 5, 2, 3};
	TArray<int> Expect{3, 2, 5, 1, 4, 2};
	TArray<int> Result = From(Array) >> Reverse() >> ToArray();
	for (int i = 0; i < Expect.Num(); ++i)
	{
		TestEqual(TEXT("Reverse 테스트 "), Expect[i], Result[i]);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(LinqTest_From_DistinctBy, "Private.LinqTest.DistinctBy", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool LinqTest_From_DistinctBy::RunTest(const FString& Parameters)
{
	TArray<int> Array{2, 2, 3, 4, 4, 1, 5, 2, 3, 2, 1, 1, 4, 5, 1, 5};
	TArray<int> Expect{1, 2, 3, 4, 5};
	TArray<int> Result = From(Array) >> DistinctBy([](int X){ return X; }) >> OrderBy([](int X){ return X; }) >> ToArray();
	if (false == TestEqual(TEXT("Distinct 테스트"), Expect.Num(), Result.Num()))
	{
		return false;
	}
	for (int i = 0; i < Expect.Num(); ++i)
	{
		TestEqual(TEXT("Distinct 테스트"), Expect[i], Result[i]);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(LinqTest_From_ExceptBy, "Private.LinqTest.ExceptBy", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool LinqTest_From_ExceptBy::RunTest(const FString& Parameters)
{
	TArray<int> Array{2, 4, 1, 5, 2, 3, 1, 2, 8, 3, 4, 5, 3, 2, 7};
	TArray<int> Expect{2, 4, 1, 5, 2, 3, 1, 2, 3, 4, 5, 3, 2 };
	
	TArray<int> Result = From(Array) >> ExceptBy(From(7, 8), [](int X){ return X; }) >> ToArray();
	if (false == TestEqual(TEXT("ExceptBy 테스트"), Expect.Num(), Result.Num()))
	{
		return false;
	}
	for (int i = 0; i < Expect.Num(); ++i)
	{
		TestEqual(TEXT("ExceptBy 테스트"), Expect[i], Result[i]);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(LinqTest_From_IntersectBy, "Private.LinqTest.IntersectBy", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool LinqTest_From_IntersectBy::RunTest(const FString& Parameters)
{
	TArray<int> Array{2, 4, 1, 5, 2, 3, 1, 2, 8, 3, 4, 5, 3, 2, 7};
	TArray<int> Intersect{7, 8, 1, 5};
	TArray<int> Expect{1, 1, 5, 5, 7, 8};
	
	// TArray<int> Result = From(Array) >> IntersectBy(From(Intersect), [](int X){ return X; }) >> OrderBy([](int X){ return X; }) >> ToArray();
	// for (int i = 0; i < Expect.Num(); ++i)
	// {
	// 	TestEqual("ExceptBy 테스트 ", Expect[i], Result[i]);
	// }

	return true;
}
