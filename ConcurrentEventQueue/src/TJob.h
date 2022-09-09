#pragma once

#include "IJob.h"
#include "TJobHandle.h"

#include <memory>
#include <functional>

namespace t3d
{
	template<typename Return_T>
	class TJob : public IJob
	{
	public:

	// Constructors and Destructor:

		TJob(auto&& Functor)
			: Functor (std::move(Functor))
			, Handle  (std::make_shared<TJobHandle<Return_T>>())
		{}

	// Functions:

		void Execute() override
		{
			Handle->Submit(Functor());
			Handle->Signal();
		}

	// Accessors:

		JobHandle_T<Return_T> GetHandle() const
		{
			return Handle;
		}

	private:

		std::function<Return_T()> Functor;
		JobHandle_T<Return_T>     Handle;
	};

	template<>
	class TJob<void> : public IJob
	{
	public:

	// Constructors and Destructor:

		TJob(auto&& Functor)
			: Functor (std::move(Functor))
			, Handle  (std::make_shared<TJobHandle<void>>())
		{}

	// Functions:

		void Execute() override
		{
			Functor();

			Handle->Signal();
		}

	// Accessors:

		JobHandle_T<void> GetHandle() const
		{
			return Handle;
		}

	private:

		std::function<void()> Functor;
		JobHandle_T<void>     Handle;
	};
}