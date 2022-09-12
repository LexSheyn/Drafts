#pragma once

#include <deque>
#include <functional>
#include <memory>
#include <mutex>

namespace t3d
{
	using Functor_T    = std::function<void()>;
	using JobPointer_T = std::unique_ptr<Functor_T>;

	class FJobQueue
	{
	public:

		 FJobQueue () = default;
		~FJobQueue () = default;

		// No copy
		// No move

		void Submit        (JobPointer_T&& Job);
		bool TransferFront (JobPointer_T& Job);

		bool   IsEmpty () const;
		size_t Size    () const;

	private:

		mutable std::mutex        AccessMutex;
		std::deque<JobPointer_T>  Jobs;
	};

//	constexpr size_t Size = sizeof(FJobQueue);
}