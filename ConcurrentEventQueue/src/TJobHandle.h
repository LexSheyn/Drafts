#pragma once

#include "FAtomicLock.h"

#include <memory>

namespace t3d
{
	template<typename Return_T>
	class TJobHandle
	{
	public:

	// Constructors and Destructors:

		TJobHandle()
			: Result (nullptr)
		{}

		~TJobHandle()
		{
			this->TryDelete();
		}

	// Functions:

		void Submit(Return_T&& Value)
		{
			this->TryDelete();

			Result = new Return_T(std::forward<Return_T>(Value));
		}

		void Signal()
		{
			AwaitLock.Release();
		}

		Return_T Await()
		{
			AwaitLock.Acquire();

			return *Result;
		}

	private:

	// Private Functions:

		void TryDelete()
		{
			if (Result)
			{
				delete Result;

				Result = nullptr;
			}
		}

	// Variables:

		FAtomicLock AwaitLock;
		Return_T*           Result;
	};

	template<>
	class TJobHandle<void>
	{
	public:

	// Constructors and Destructors:

		 TJobHandle () = default;
		~TJobHandle () = default;

	// Functions:

		void Signal()
		{
			AwaitLock.Release();
		}

		void Await()
		{
			AwaitLock.Acquire();
		}

	private:

	// Variables:

		FAtomicLock AwaitLock;
	};

	template<typename Return_T>
	using JobHandle_T = std::shared_ptr<TJobHandle<Return_T>>;
}