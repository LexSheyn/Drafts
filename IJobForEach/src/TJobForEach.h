#pragma once

#include <utility>

namespace test
{
	struct CHealth
	{
		int32_t Max;
		int32_t Value;
	};

	struct CVelocity
	{
		float X;
		float Y;
	};

	template<typename... Args_T>
	struct TJobForEach
	{
		inline virtual void Execute (Args_T&... Args) = 0;

		void operator () (Args_T&... Args)
		{
			this->Execute(Args...);
		}
	};

	template<typename Iterator_T, typename Function_T>
	inline Function_T ForEach(Iterator_T Begin, Iterator_T End, Function_T Function)
	{
		for (; Begin != End; ++Begin)
		{
			Function(*Begin);
		}

		return Function;
	}
}