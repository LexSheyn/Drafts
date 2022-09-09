#pragma once

#include <atomic>

namespace t3d
{
	class FAtomicLock
	{
	public:

	// Constructors and Destructor:

		 FAtomicLock ();
		~FAtomicLock () = default;

	// Functions:

		void Release ();
		void Acquire ();

	private:

	// Variables:

		std::atomic<bool> b_Released;
	};

//	constexpr size_t Size = sizeof(FAtomicLock);
}