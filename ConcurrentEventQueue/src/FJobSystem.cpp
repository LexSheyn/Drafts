#include "FJobSystem.h"

namespace t3d
{
// Constructors and Destructor:

	FJobSystem::FJobSystem()
		: b_Running (false)
	{
		WorkerThreads.reserve(4);

		WorkerThreads.push_back(std::make_unique<FWorkerThread>());
		WorkerThreads.push_back(std::make_unique<FWorkerThread>());
		WorkerThreads.push_back(std::make_unique<FWorkerThread>());
		WorkerThreads.push_back(std::make_unique<FWorkerThread>());
	}

	FJobSystem::~FJobSystem()
	{
		if (b_Running.load())
		{
			this->Shutdown();
		}
	}


// Functions:

	void FJobSystem::Startup()
	{
		b_Running.store(true);

		for (auto& Thread : WorkerThreads)
		{
			Thread->Launch();
		}
	}

	void FJobSystem::Shutdown()
	{
		b_Running.store(false);
	
		for (auto& Thread : WorkerThreads)
		{
			Thread->Stop();
		}
	}


// Accessors:

	bool FJobSystem::IsRunning() const
	{
		return b_Running.load();
	}

	bool FJobSystem::IsBusy(EThreadId ThreadId) const
	{
		return WorkerThreads[static_cast<size_t>(ThreadId)]->IsBusy();
	}

}