#include "FWorkerThread.h"

#include <cassert>

namespace t3d
{
// Constructors and Destructor:

	FWorkerThread::FWorkerThread()
		: LaunchSemaphore (false)
		, StopSemaphore   (false)
		, b_Running       (false)
	{}

	FWorkerThread::~FWorkerThread()
	{
		if (b_Running.load())
		{
			this->Stop();
		}
	}


// Functions:

	void FWorkerThread::Launch()
	{
		assert(b_Running.load() == false && "Thread is already launched!");

		b_Running.store(true);

		ExecutionThread = std::thread(&FWorkerThread::ExecuteJobs, this);

		ExecutionThread.detach();

		LaunchSemaphore.acquire();
	}

	void FWorkerThread::Stop()
	{
		assert(b_Running.load() && "Thread is not running!");

		this->Schedule([this]() { b_Running.store(false); });

		StopSemaphore.acquire();
	}


// Accessors:

	bool FWorkerThread::IsRunning() const
	{
		return b_Running.load();
	}

	bool FWorkerThread::IsBusy() const
	{
		return b_Busy.load();
	}


// Private Functions:

	void FWorkerThread::ExecuteJobs()
	{
		LaunchSemaphore.release();

		std::vector<Job_T> ReadBuffer;

		while (b_Running.load() || !this->BufferIsEmpty(WriteBuffer))
		{
			ExecutionLock.Acquire();

			b_Busy.store(true);

			{
				std::scoped_lock<std::mutex> Lock(BufferMutex);

				std::swap(ReadBuffer, WriteBuffer);
			}

			for (auto& Job : ReadBuffer)
			{
				Job->Execute();
			}

			ReadBuffer.clear();

			b_Busy.store(false);
		}

		StopSemaphore.release();
	}


// Private Accessors:

	bool FWorkerThread::BufferIsEmpty(std::vector<Job_T>& Buffer) const
	{
		std::scoped_lock<std::mutex> Lock(BufferMutex);

		return Buffer.empty();
	}

}