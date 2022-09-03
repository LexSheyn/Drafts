#pragma once

#include <string>
#include <iostream>

#if defined _MSC_VER
#pragma pointers_to_members( full_generality, multiple_inheritance )
#endif

namespace mvc
{
	class FDelegate
	{
	public:

		template<class C>
		using Callback_T = void(C::*)(std::string Data);

		FDelegate()
			: Instance (nullptr)
			, Callback (nullptr)
		{}

		template<class C>
		FDelegate(C* Instance, Callback_T<C> Callback)
			: Instance (reinterpret_cast<FClass*>(Instance))
			, Callback (reinterpret_cast<Callback_T<FClass>>(Callback))
		{}

		void operator () (std::string Data) const
		{
			(Instance->*Callback)(Data);
		}

		bool operator == (const FDelegate& Right)
		{
			return Instance == Right.Instance && Callback == Right.Callback;
		}

		template<class C>
		bool Contains(Callback_T<C> Callback)
		{
			return reinterpret_cast<Callback_T<C>>(this->Callback) == Callback;
		}

	private:

		class FClass*      Instance;
		Callback_T<FClass> Callback;
	};
}