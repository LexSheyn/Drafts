#pragma once

#include "TJob.h"

#include <mutex>
#include <thread>
#include <semaphore>
#include <atomic>
#include <vector>
#include <functional>
#include <type_traits>

namespace t3d
{
	class FWorkerThread
	{
	public:

	// Constructors and Destructor:

		 FWorkerThread ();		
		~FWorkerThread ();

		// No copy
		// No move

	// Functions:

		void Launch ();
		void Stop   ();
		
		template<typename Functor_T, typename... Args_T>
		using Return_T = std::invoke_result_t<Functor_T, Args_T...>;

		template<typename Functor_T>
		JobHandle_T<Return_T<Functor_T>> Schedule(Functor_T&& Job)
		{
			TJob<Return_T<Functor_T>> InternalJob(std::move(Job));

			JobHandle_T<Return_T<Functor_T>> Handle = InternalJob.GetHandle();

			{
				std::scoped_lock<std::mutex> Lock(BufferMutex);

				WriteBuffer.push_back(std::make_unique<TJob<Return_T<Functor_T>>>(std::move(InternalJob)));
			}

			ExecutionLock.Release();

			return Handle;
		}

	private:

	// Private Functions:

		void ExecuteJobs ();

	// Private Accessors:

		bool BufferIsEmpty (std::vector<Job_T>& Buffer) const;

	// Variables:

		mutable std::mutex    BufferMutex;
		std::vector<Job_T>    WriteBuffer;
		std::thread           ExecutionThread;
		FAtomicLock           ExecutionLock;
		std::binary_semaphore StartSemaphore;
		std::atomic<bool>     b_Running;
	};

//	constexpr size_t Size = sizeof(FWorkerThread);
}