# Unreal Linq Plugin

This plugin is a simple implementation of Linq for Unreal Engine 4. It is designed to be used with the Unreal Engine 4's C++ API.

```c++
	TArray<int> Array{2, 2, 3, 4, 4, 1, 5, 2, 3, 2, 1, 1, 4, 5, 1, 5};
	TArray<int> Result = From(Array)
	                        >> DistinctBy([](int X){ return X; })
	                        >> OrderBy([](int X){ return X; })
	                        >> ToArray();
	for (int i = 0; i < Result.Num(); ++i)
	{
		UE_LOG(LogTemp, Log, TEXT("%d"), Result[i]);
	}
	// print 1, 2, 3, 4, 5
```

## Todo List
- [ ] Cast
- [ ] Take
- [ ] Skip
- [ ] Concat
- [x] Where
- [x] Select
- [x] Reverse
- [x] OrderBy
- [x] ThenBy
- [ ] GroupBy
- [x] Distinct
- [x] ExceptBy
- [ ] All
- [ ] Any
- [ ] Sum
- [ ] Average
- [ ] Min
- [ ] Max
- [ ] Count
- [ ] Contains
- [ ] ElementAt
- [x] ToArray
- [ ] ToMap
- [ ] ToSet