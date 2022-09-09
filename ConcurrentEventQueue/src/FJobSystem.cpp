#include "FJobSystem.h"
/*
namespace t3d
{
	FJobSystem::FJobSystem()
		: ExecutionSemaphore (false)
		, b_Started          (false)
		, b_Running          (false)
		, b_Busy             (false)
	{
		SequentialThreads.push_back(std::make_unique<XWorkerThread>());
		SequentialThreads.push_back(std::make_unique<XWorkerThread>());
		SequentialThreads.push_back(std::make_unique<XWorkerThread>());
		SequentialThreads.push_back(std::make_unique<XWorkerThread>());

		ConcurrentThreads.push_back(std::make_unique<XWorkerThread>());
		ConcurrentThreads.push_back(std::make_unique<XWorkerThread>());
		ConcurrentThreads.push_back(std::make_unique<XWorkerThread>());
		ConcurrentThreads.push_back(std::make_unique<XWorkerThread>());
	}

	FJobSystem::~FJobSystem()
	{
		if (b_Running.load())
		{
			this->Shutdown();
		}
	}

	void FJobSystem::Startup()
	{
		b_Running.store(true);

		for (auto& Thread : SequentialThreads)
		{
			Thread->Launch();
		}

		for (auto& Thread : ConcurrentThreads)
		{
			Thread->Launch();
		}

		ExecutionThread = std::thread(&FJobSystem::ExecuteJobs, this);

		while (!ExecutionThread.joinable()) {}

		b_Started.store(true);

		b_Started.notify_one();
	}

	void FJobSystem::Shutdown()
	{
		b_Running.store(false);
	
		ExecutionSemaphore.release(!b_Busy.load());

		b_Started.wait(true);

		if (ExecutionThread.joinable())
		{
			ExecutionThread.join();
		}

		for (auto& Thread : SequentialThreads)
		{
			Thread->Stop();
		}

		for (auto& Thread : ConcurrentThreads)
		{
			Thread->Stop();
		}
	}

	void FJobSystem::Schedule(Job_T&& Job)
	{
		JobQueue.Push(std::move(Job));

		ExecutionSemaphore.release(!b_Busy.load());
	}

	void FJobSystem::Schedule(EThreadId Id, Job_T&& Job)
	{
		SequentialThreads.at(Id)->Schedule(std::move(Job));
	}

	bool FJobSystem::IsRunning() const
	{
		return b_Running.load();
	}

	bool FJobSystem::IsBusy(EThreadId Id) const
	{
		return SequentialThreads.at(Id)->IsBusy();
	}

	size_t FJobSystem::JobCount() const
	{
		return JobQueue.Size();
	}

	size_t FJobSystem::JobCount(EThreadId Id) const
	{
		return SequentialThreads.at(Id)->JobCount();
	}

	size_t FJobSystem::SequentialThreadsCount() const noexcept
	{
		return SequentialThreads.size();
	}

	size_t FJobSystem::ConcurrentThreadsCount() const noexcept
	{
		return ConcurrentThreads.size();
	}

	void FJobSystem::ExecuteJobs()
	{
		b_Started.wait(false);

		Job_T Job;

		size_t Index = 0;

		size_t LowestJobCount = ConcurrentThreads[Index]->JobCount();

		while (b_Running.load() || !JobQueue.IsEmpty())
		{
			ExecutionSemaphore.acquire();

			b_Busy.store(true);

			while (JobQueue.TransferFront(Job))
			{
				for (size_t i = 0u; i < ConcurrentThreads.size(); ++i)
				{
					if (ConcurrentThreads[i]->JobCount() < LowestJobCount)
					{
						Index = i;
					}
				}

				ConcurrentThreads[Index]->Schedule(std::move(Job));
			}

			b_Busy.store(false);
		}

		b_Started.store(false);

		b_Started.notify_one();
	}

}*/