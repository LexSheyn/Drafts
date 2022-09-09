#include "FAtomicLock.h"

namespace t3d
{
// Constructors and Destructor:

	FAtomicLock::FAtomicLock()
		: b_Released (false)
	{}


// Functions:

	void FAtomicLock::Release()
	{
		b_Released.store(true);

		b_Released.notify_one();
	}

	void FAtomicLock::Acquire()
	{
		b_Released.wait(false);

		b_Released.store(false);
	}

}