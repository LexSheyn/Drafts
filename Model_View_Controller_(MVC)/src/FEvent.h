#pragma once

#include "FDelegate.h"
#include <vector>

namespace mvc
{
	class FEvent
	{
	public:

		void Invoke(std::string Data) const
		{
			for (auto& Delegate : Delegates)
			{
				Delegate(Data);
			}
		}

		template<class C>
		void Subscribe(C* Instance, FDelegate::Callback_T<C> Callback)
		{
			Delegates.push_back(FDelegate(Instance, Callback));
		}

		template<class C>
		void Unsubscribe(FDelegate::Callback_T<C> Callback)
		{
			for (size_t i = 0u; i < Delegates.size(); ++i)
			{
				if (Delegates[i].Contains(Callback))
				{
					Delegates.erase(Delegates.begin() + i);

					return;
				}
			}
		}

	private:

		std::vector<FDelegate> Delegates;
	};
}