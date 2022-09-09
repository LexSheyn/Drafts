#pragma once
/*
#include "XWorkerThread.h"

#include <memory>
#include <unordered_map>
#include <vector>
#include <semaphore>
#include <atomic>

namespace t3d
{
	enum EThreadId
	{
		  T3D_THREAD_ID_ZERO = 0
		, T3D_THREAD_ID_ONE
		, T3D_THREAD_ID_TWO
		, T3D_THREAD_ID_THREE
		, T3D_THREAD_ID_MAX_ENUM
	};

	class FJobSystem
	{
	public:

		 FJobSystem ();
		~FJobSystem ();

		// No copy
		// No move

		void Startup  ();
		void Shutdown ();

		void Schedule (Job_T&& Job);               // Return JobHandle_T
		void Schedule (EThreadId Id, Job_T&& Job); // Return JobHandle_T

		bool   IsRunning              () const;
		bool   IsBusy                 (EThreadId Id) const;
		size_t JobCount               () const;
		size_t JobCount               (EThreadId Id) const;
		size_t SequentialThreadsCount () const noexcept;
		size_t ConcurrentThreadsCount () const noexcept;

	private:

		void ExecuteJobs ();

		enum { MaxThreads = 4 };

		FJobQueue                                   JobQueue;
		std::vector<std::unique_ptr<XWorkerThread>> SequentialThreads;
		std::vector<std::unique_ptr<XWorkerThread>> ConcurrentThreads;
		std::thread                                 ExecutionThread;
		std::binary_semaphore                       ExecutionSemaphore;
		std::atomic<bool>                           b_Started;
		std::atomic<bool>                           b_Running;
		std::atomic<bool>                           b_Busy;
	};

//	constexpr size_t Size = sizeof(std::thread);
//	constexpr size_t Size = sizeof(std::vector<std::unique_ptr<XWorkerThread>>);
//	constexpr size_t Size = sizeof(FJobSystem);
//	constexpr size_t Size = sizeof(std::counting_semaphore<12>);
//	constexpr size_t Size = sizeof(std::condition_variable);
//	constexpr size_t Size = sizeof(std::mutex);
}*/