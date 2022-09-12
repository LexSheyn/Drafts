#include "FJobQueue.h"

namespace t3d
{
	void FJobQueue::Submit(JobPointer_T&& Job)
	{
		std::scoped_lock<std::mutex> Lock(AccessMutex);

		Jobs.push_back(std::move(Job));
	}

	bool FJobQueue::TransferFront(JobPointer_T& Job)
	{
		std::scoped_lock<std::mutex> Lock(AccessMutex);

		if (Jobs.empty())
		{
			return false;
		}

		Job = std::move(Jobs.front());

		Jobs.pop_front();

		return true;
	}

	bool FJobQueue::IsEmpty() const
	{
		std::scoped_lock<std::mutex> Lock(AccessMutex);

		return Jobs.empty();
	}

	size_t FJobQueue::Size() const
	{
		std::scoped_lock<std::mutex> Lock(AccessMutex);

		return Jobs.size();
	}

}