#include "FWorkerThread.h"

namespace t3d
{
// Constructors and Destructor:

	FWorkerThread::FWorkerThread()
		: StartSemaphore (false)
		, b_Running      (false)
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
		b_Running.store(true);

		ExecutionThread = std::thread(&FWorkerThread::ExecuteJobs, this);

		StartSemaphore.acquire();
	}

	void FWorkerThread::Stop()
	{
		this->Schedule([this]() { b_Running.store(false); });

		ExecutionThread.join();
	}


// Private Functions:

	void FWorkerThread::ExecuteJobs()
	{
		StartSemaphore.release();

		std::vector<Job_T> ReadBuffer;

		while (b_Running.load() || !this->BufferIsEmpty(WriteBuffer))
		{
			ExecutionLock.Acquire();

			{
				std::scoped_lock<std::mutex> Lock(BufferMutex);

				std::swap(ReadBuffer, WriteBuffer);
			}

			for (auto& Job : ReadBuffer)
			{
				Job->Execute();
			}

			ReadBuffer.clear();
		}
	}


// Private Accessors:

	bool FWorkerThread::BufferIsEmpty(std::vector<Job_T>& Buffer) const
	{
		std::scoped_lock<std::mutex> Lock(BufferMutex);

		return Buffer.empty();
	}

}