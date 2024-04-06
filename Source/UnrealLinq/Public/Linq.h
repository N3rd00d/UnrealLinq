#pragma once
#include "Algo/StableSort.h"

namespace Linq
{
	template <typename TElementType>
	struct TEnumeratorArray
	{
	public:
		typedef TElementType ElementType;
		TArray<ElementType> Container;
		ElementType* Data = nullptr;
		int Size = 0;
		int Index = -1;

		TEnumeratorArray(ElementType* Data, int Size) : Data(Data), Size(Size)
		{
		}

		TEnumeratorArray(TArray<ElementType>&& Array) : Container(MoveTemp(Array)), Data(Container.GetData()), Size(Container.Num())
		{
		}

		bool MoveNext()
		{
			if (Index < Size - 1)
			{
				Index += 1;
				return true;
			}
			return false;
		}

		ElementType& Current()
		{
			return Data[Index];
		}

		template <typename TGenerator>
		auto operator>>(TGenerator Generator)
		{
			return Generator.Gen(*this);
		}
	};

	template <typename TEnumerator, typename TPredicate>
	struct TEnumeratorWhere
	{
	public:
		typedef typename TEnumerator::ElementType ElementType;
		TEnumerator Enumerator;
		TPredicate&& Pred;

		TEnumeratorWhere(TEnumerator&& Enumerator, TPredicate&& Pred) : Enumerator(MoveTemp(Enumerator)), Pred(MoveTemp(Pred))
		{
		}

		bool MoveNext()
		{
			while (Enumerator.MoveNext())
			{
				if (Pred(Enumerator.Current()))
				{
					return true;
				}
			}
			return false;
		}

		ElementType& Current()
		{
			return Enumerator.Current();
		}

		template <typename TGenerator>
		auto operator>>(TGenerator Generator)
		{
			return Generator.Gen(*this);
		}
	};

	template <typename TPredicate>
	struct TGeneratorWhere
	{
	public:
		TPredicate&& Pred;

		TGeneratorWhere(TPredicate&& Pred) : Pred(MoveTemp(Pred))
		{
		}

		template <typename TEnumerator>
		auto Gen(TEnumerator& Enumerator)
		{
			return TEnumeratorWhere<TEnumerator, TPredicate>(MoveTemp(Enumerator), MoveTemp(Pred));
		}
	};

	template <typename TPredicate>
	struct Lambda : public Lambda<decltype(&TPredicate::operator())>
	{
	};

	template <typename TClass, typename TReturn, typename... Args>
	struct Lambda<TReturn(TClass::*)(Args...) const>
	{
		typedef TReturn ReturnType;
	};

	template <typename TEnumerator, typename TPredicate>
	struct TEnumeratorSelect
	{
		TEnumerator Enumerator;
		TPredicate&& Pred;
		typedef typename Lambda<TPredicate>::ReturnType ElementType;
		ElementType CurrentValue;

		TEnumeratorSelect(TEnumerator&& Enumerator, TPredicate&& Pred) : Enumerator(MoveTemp(Enumerator)), Pred(MoveTemp(Pred))
		{
		}

		bool MoveNext()
		{
			while (Enumerator.MoveNext())
			{
				CurrentValue = Pred(Enumerator.Current());
				return true;
			}
			return false;
		}

		ElementType& Current()
		{
			return CurrentValue;
		}

		template <typename TGenerator>
		auto operator>>(TGenerator Generator)
		{
			return Generator.Gen(*this);
		}
	};

	template <typename TPredicate>
	struct TGeneratorSelect
	{
	public:
		TPredicate&& Pred;

		TGeneratorSelect(TPredicate&& Pred) : Pred(MoveTemp(Pred))
		{
		}

		template <typename TEnumerator>
		auto Gen(TEnumerator& Enumerator)
		{
			return TEnumeratorSelect<TEnumerator, TPredicate>(MoveTemp(Enumerator), MoveTemp(Pred));
		}
	};

	struct TGeneratorToArray
	{
	public:
		TGeneratorToArray()
		{
		}

		template <typename TEnumerator>
		auto Gen(TEnumerator& Enumerator)
		{
			TArray<typename TEnumerator::ElementType> Array;
			while (Enumerator.MoveNext())
			{
				Array.Emplace(Enumerator.Current());
			}
			return Array;
		}
	};

	struct IEnumeratorSort
	{
	};

	template <typename TEnumerator, typename TPredicate, bool IsAscending>
	struct TEnumeratorOrderBy : public IEnumeratorSort
	{
	public:
		typedef typename TEnumerator::ElementType ElementType;
		TEnumerator Enumerator;
		TPredicate&& Pred;
		TArray<ElementType*> Sorted;
		int64 Index = -1;

		TEnumeratorOrderBy(TEnumerator&& Enumerator, TPredicate&& Pred) : Enumerator(MoveTemp(Enumerator)), Pred(MoveTemp(Pred))
		{
			if (std::is_convertible<TEnumerator, IEnumeratorSort>::value)
			{
				verify(true);
			}
		}

		bool Compare(const ElementType& L, const ElementType& R)
		{
			if (IsAscending)
			{
				return Pred(L) < Pred(R);
			}
			else
			{
				return Pred(R) < Pred(L);
			}
		}

		bool ForwardMoveNext()
		{
			return Enumerator.MoveNext();
		}

		ElementType& ForwardCurrent()
		{
			return Enumerator.Current();
		}

		bool MoveNext()
		{
			if (Index == -1)
			{
				while (Enumerator.MoveNext())
				{
					Sorted.Emplace(&Enumerator.Current());
				}
				Algo::StableSort(Sorted, [this](ElementType* L, ElementType* R)
				{
					return this->Compare(*L, *R);
				});
			}

			if (Index < Sorted.Num() - 1)
			{
				Index += 1;
				return true;
			}
			return false;
		}

		ElementType& Current()
		{
			return *Sorted[Index];
		}

		template <typename TGenerator>
		auto operator>>(TGenerator Generator)
		{
			return Generator.Gen(*this);
		}
	};

	template <typename TEnumerator, typename TPredicate, bool IsAscending>
	struct TEnumeratorThenBy : public IEnumeratorSort
	{
	public:
		typedef typename TEnumerator::ElementType ElementType;
		TEnumerator Enumerator;
		TPredicate&& Pred;
		TArray<ElementType*> Sorted;
		int64 Index = -1;

		TEnumeratorThenBy(TEnumerator&& Enumerator, TPredicate&& Pred) : Enumerator(MoveTemp(Enumerator)), Pred(MoveTemp(Pred))
		{
			if (!std::is_convertible<TEnumerator, IEnumeratorSort>::value)
			{
				verify(true);
			}
		}

		bool Compare(const ElementType& L, const ElementType& R)
		{
			if (Enumerator.Compare(L, R))
			{
				return true;
			}
			if (Enumerator.Compare(R, L))
			{
				return false;
			}
			if (IsAscending)
			{
				return Pred(L) < Pred(R);
			}
			else
			{
				return Pred(R) < Pred(L);
			}
		}

		bool ForwardMoveNext()
		{
			return Enumerator.MoveNext();
		}

		ElementType& ForwardCurrent()
		{
			return Enumerator.Current();
		}

		bool MoveNext()
		{
			if (Index == -1)
			{
				while (Enumerator.ForwardMoveNext())
				{
					Sorted.Emplace(&Enumerator.ForwardCurrent());
				}
				Algo::StableSort(Sorted, [this](ElementType* L, ElementType* R)
				{
					return this->Compare(*L, *R);
				});
			}

			if (Index < Sorted.Num() - 1)
			{
				Index += 1;
				return true;
			}
			return false;
		}

		ElementType& Current()
		{
			return *Sorted[Index];
		}

		template <typename TGenerator>
		auto operator>>(TGenerator Generator)
		{
			return Generator.Gen(*this);
		}
	};

	template <typename TPredicate>
	struct TGeneratorOrderBy
	{
	public:
		TPredicate&& Pred;

		TGeneratorOrderBy(TPredicate&& Pred) : Pred(MoveTemp(Pred))
		{
		}

		template <typename TEnumerator>
		auto Gen(TEnumerator& Enumerator)
		{
			return TEnumeratorOrderBy<TEnumerator, TPredicate, true>(MoveTemp(Enumerator), MoveTemp(Pred));
		}
	};

	template <typename TPredicate>
	struct TGeneratorThenBy
	{
	public:
		TPredicate&& Pred;

		TGeneratorThenBy(TPredicate&& Pred) : Pred(MoveTemp(Pred))
		{
		}

		template <typename TEnumerator>
		auto Gen(TEnumerator& Enumerator)
		{
			return TEnumeratorThenBy<TEnumerator, TPredicate, true>(MoveTemp(Enumerator), MoveTemp(Pred));
		}
	};

	template <typename TEnumerator>
	struct TEnumeratorReverse
	{
		TEnumerator Enumerator;
		typedef typename TEnumerator::ElementType ElementType;
		TArray<ElementType*> Reversed;
		int Index = -1;

		TEnumeratorReverse(TEnumerator&& Enumerator) : Enumerator(Enumerator)
		{
		}

		bool MoveNext()
		{
			if (Index == -1)
			{
				while (Enumerator.MoveNext())
				{
					Reversed.Emplace(&Enumerator.Current());
				}
				Index = Reversed.Num() - 1;
			}

			if (Index > -1)
			{
				Index -= 1;
				return true;
			}
			return false;
		}

		ElementType& Current()
		{
			return *Reversed[Index];
		}

		template <typename TGenerator>
		auto operator>>(TGenerator Generator)
		{
			return Generator.Gen(*this);
		}
	};

	struct TGeneratorReverse
	{
	public:
		TGeneratorReverse()
		{
		}

		template <typename TEnumerator>
		auto Gen(TEnumerator& Enumerator)
		{
			return TEnumeratorReverse<TEnumerator>(MoveTemp(Enumerator));
		}
	};

	template <typename TEnumerator, typename TPredicate>
	struct TEnumeratorDistinctBy
	{
	public:
		typedef typename TEnumerator::ElementType ElementType;
		typedef typename Lambda<TPredicate>::ReturnType KeySelectorType;
		TEnumerator Enumerator;
		TPredicate&& Pred;
		TArray<ElementType*> Distinct;
		int Index = -1;

		TEnumeratorDistinctBy(TEnumerator&& Enumerator, TPredicate&& Pred) : Enumerator(MoveTemp(Enumerator)), Pred(MoveTemp(Pred))
		{
		}

		bool MoveNext()
		{
			if (Index == -1)
			{
				TSet<KeySelectorType> Keys;
				bool IsAlreadyInSet;
				while (Enumerator.MoveNext())
				{
					auto& Current = Enumerator.Current();
					Keys.Emplace(Pred(Current), &IsAlreadyInSet);
					if (!IsAlreadyInSet)
					{
						Distinct.Emplace(&Current);
					}
				}
			}

			if (Index < Distinct.Num() - 1)
			{
				Index += 1;
				return true;
			}
			return false;
		}

		ElementType& Current()
		{
			return *Distinct[Index];
		}

		template <typename TGenerator>
		auto operator>>(TGenerator Generator)
		{
			return Generator.Gen(*this);
		}
	};

	template <typename TPredicate>
	struct TGeneratorDistinctBy
	{
	public:
		TPredicate&& Pred;

		TGeneratorDistinctBy(TPredicate&& Pred) : Pred(MoveTemp(Pred))
		{
		}

		template <typename TEnumerator>
		auto Gen(TEnumerator& Enumerator)
		{
			return TEnumeratorDistinctBy<TEnumerator, TPredicate>(MoveTemp(Enumerator), MoveTemp(Pred));
		}
	};

	template <typename TEnumerator, typename TEnumeratorKey, typename TPredicate>
	struct TEnumeratorExceptBy
	{
	public:
		typedef typename TEnumerator::ElementType ElementType;
		typedef typename TEnumeratorKey::ElementType ElementKeyType;
		TEnumerator Enumerator;
		TEnumeratorKey Key;
		TPredicate&& Pred;
		TArray<ElementKeyType*> ExceptValues;
		bool MoveStart = false;

		TEnumeratorExceptBy(TEnumerator&& Enumerator, TEnumeratorKey&& Key, TPredicate&& Pred) : Enumerator(MoveTemp(Enumerator)), Key(MoveTemp(Key)), Pred(MoveTemp(Pred))
		{
		}

		bool MoveNext()
		{
			if (false == MoveStart)
			{
				while (Key.MoveNext())
				{
					ExceptValues.Add(&Key.Current());
				}
				MoveStart = true;
			}

			while (Enumerator.MoveNext())
			{
				for (ElementKeyType* Value : ExceptValues)
				{
					if (*Value == Pred(Enumerator.Current()))
					{
						MoveNext();
					}
				}
				return true;
			}
			return false;
		}

		ElementType& Current()
		{
			return Enumerator.Current();
		}

		template <typename TGenerator>
		auto operator>>(TGenerator Generator)
		{
			return Generator.Gen(*this);
		}
	};


	template <typename TEnumeratorKey, typename TPredicate>
	struct TGeneratorExceptBy
	{
	public:
		TEnumeratorKey Key;
		TPredicate&& Pred;

		TGeneratorExceptBy(TEnumeratorKey&& Key, TPredicate&& Pred) : Key(MoveTemp(Key)), Pred(MoveTemp(Pred))
		{
		}

		template <typename TEnumerator>
		auto Gen(TEnumerator& Enumerator)
		{
			return TEnumeratorExceptBy<TEnumerator, TEnumeratorKey, TPredicate>(MoveTemp(Enumerator), MoveTemp(Key), MoveTemp(Pred));
		}
	};
}

template <typename T>
auto From(TArray<T>& Array)
{
	return Linq::TEnumeratorArray<T>(Array.GetData(), Array.Num());
}

template <typename T>
auto From(TArray<T>&& Array)
{
	return Linq::TEnumeratorArray<T>(MoveTemp(Array));
}

template <typename... T>
auto From(T&&... Args)
{
	TArray<std::common_type_t<T...>> Array;
	(Array.Emplace(MoveTemp(Args)), ...);
	return From(MoveTemp(Array));
}

template <typename TPredicate>
auto Where(TPredicate&& Pred)
{
	return Linq::TGeneratorWhere<TPredicate>(MoveTemp(Pred));
}

template <typename TPredicate>
auto Select(TPredicate&& Pred)
{
	return Linq::TGeneratorSelect<TPredicate>(MoveTemp(Pred));
}

inline auto ToArray()
{
	return Linq::TGeneratorToArray();
}

template <typename TPredicate>
auto OrderBy(TPredicate&& Pred)
{
	return Linq::TGeneratorOrderBy<TPredicate>(MoveTemp(Pred));
}

template <typename TPredicate>
auto ThenBy(TPredicate&& Pred)
{
	return Linq::TGeneratorThenBy<TPredicate>(MoveTemp(Pred));
}

inline auto Reverse()
{
	return Linq::TGeneratorReverse();
}

template <typename TPredicate>
auto DistinctBy(TPredicate&& Pred)
{
	return Linq::TGeneratorDistinctBy<TPredicate>(MoveTemp(Pred));
}

template <typename TEnumeratorKey, typename TPredicate>
auto ExceptBy(TEnumeratorKey&& Key, TPredicate&& Pred)
{
	return Linq::TGeneratorExceptBy<TEnumeratorKey, TPredicate>(MoveTemp(Key), MoveTemp(Pred));
}
