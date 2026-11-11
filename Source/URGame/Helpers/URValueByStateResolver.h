// Written by EXPx2

#pragma once

#include "Containers/Map.h"
#include "Misc/Optional.h"

#include <type_traits>

/**
 *
 */
template <typename TSource, typename TValue>
class TURValueByStateResolver
{
public:
	void Set(TSource Source, const TValue& Value)
	{
		Values.Add(Source, Value);
		
		if (!CurrentSource.IsSet() || Source > CurrentSource.GetValue())
		{
			CurrentSource = Source;
		}
	}
	
	void CancelSource(TSource Source)
	{
		if (!Values.Remove(Source))
		{
			return;
		}
		
		if (CurrentSource.IsSet() && 
			CurrentSource.GetValue() == Source)
		{
			// 현재 소스일 경우
			CurrentSource.Reset();
			for (const TPair<TSource, TValue>& Entry : Values)
			{
				if (!CurrentSource.IsSet() || 
					Entry.Key > CurrentSource.GetValue())
				{
					CurrentSource = Entry.Key;
				}
			}
		}
	}
	
	void Clear()
	{
		Values.Reset();
		CurrentSource.Reset();
	}
	
	const TValue* GetCurrentValue() const
	{
		return CurrentSource.IsSet() ? Values.Find(CurrentSource.GetValue()) : nullptr;
	}

	TSource GetCurrentSource() const
	{
		return CurrentSource.Get(TSource{});
	}

private:
	TMap<TSource, TValue> Values;
	TOptional<TSource> CurrentSource;
};
