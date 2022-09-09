#pragma once

#include "FWorkerThread.h"

#include <type_traits>
#include <vector>
#include <memory>

namespace t3d
{
	enum class EThreadId
	{
		  Zero = 0
		, One
		, Two
		, Three
	};

	class FJobSystem
	{
	public:

	// Constructors and Destructor:

		 FJobSystem ();
		~FJobSystem ();

		// No copy
		// No move

	// Functions:

		void Startup  ();
		void Shutdown ();

		template<typename Functor_T, typename... Args_T>
		using Return_T = std::invoke_result_t<Functor_T, Args_T...>;

		template<typename Functor_T>
		JobHandle_T<Return_T<Functor_T>> Schedule(EThreadId ThreadId, Functor_T&& Job)
		{
			return WorkerThreads[static_cast<size_t>(ThreadId)]->Schedule(std::move(Job));
		}

	// Accessors:

		bool IsRunning () const;
		bool IsBusy    (EThreadId ThreadId) const;

	private:

	// Variables:

		std::vector<std::unique_ptr<FWorkerThread>> WorkerThreads;
		std::atomic<bool>                           b_Running;
	};

//	constexpr size_t Size = sizeof(FJobSystem);
}