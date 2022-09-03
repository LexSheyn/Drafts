#pragma once

#include <memory>

namespace cmd
{
	class FEvent
	{};

	using EventPointer_T = std::shared_ptr<FEvent>;

	class FEventQueue
	{
	public:

		void Create()
		{
			Instance = new FEventQueue();
		}

		void Destroy()
		{
			delete Instance;

			Instance = nullptr;
		}

		static FEventQueue* GetInstance()
		{
			return Instance;
		}

		void PushEvent(const EventPointer_T& Event)
		{
			//
		}

	private:

		static FEventQueue* Instance;
	};
}