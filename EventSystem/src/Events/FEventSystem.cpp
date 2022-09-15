#include "PCH/t3dpch.h"
#include "Events/FEventSystem.h"

namespace t3d
{
// Functions:

	void FEventSystem::PollEvents()
	{
		ReadQueue.swap(WriteQueue);

		for (auto& Event : ReadQueue)
		{
			for (auto& Callback : Callbacks.at(Event.Type))
			{
				if (Callback(Event.Data) == EPropagation::Stop)
				{
					break;
				}
			}
		}

		ReadQueue.clear();
	}

	void FEventSystem::PushEvent(EEvent Event, FEventData EventData)
	{
		WriteQueue.push_back(FEvent{ Event, EventData });
	}



} // namespace t3d