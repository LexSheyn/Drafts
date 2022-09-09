#include <cstdint>
#include <iostream>
#include <chrono>

#include "FWorkerThread.h"

int32_t main(int32_t ArgC, const char* ArgV)
{
	t3d::FWorkerThread Thread;

	Thread.Launch();

	int32_t A = 0;
	int32_t B = 0;

	int32_t Result = 0;

	size_t i = 0;

	while (i++ < 5)
	{
		auto Handle = Thread.Schedule(
			[&]()
			{
				A++;

				std::cout << "Iteration: " << i << std::endl;
			
				std::this_thread::sleep_for(std::chrono::seconds(1));

				B = A * 10;

				return B;
			});

		Result = Handle->Await();
	}

	Thread.Stop();

	std::cout << std::endl;
	std::cout << "A:      " << A      << std::endl;
	std::cout << "B:      " << B      << std::endl;
	std::cout << "Result: " << Result << std::endl;

	return 0;
}