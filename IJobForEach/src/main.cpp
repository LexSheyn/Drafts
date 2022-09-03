#include "TJobForEach.h"

#include <vector>
#include <algorithm>
#include <iostream>
#include <string>

int32_t main(int32_t ArgC, char* ArgV[])
{
	std::vector<test::CHealth> Healths;

	Healths.push_back(test::CHealth{1, 0});
	Healths.push_back(test::CHealth{2, 0});
	Healths.push_back(test::CHealth{3, 0});
	Healths.push_back(test::CHealth{4, 0});

	std::cout << "Health 0: " << std::to_string(Healths[0].Max) << " / " << std::to_string(Healths[0].Value) << std::endl;
	std::cout << "Health 1: " << std::to_string(Healths[1].Max) << " / " << std::to_string(Healths[1].Value) << std::endl;
	std::cout << "Health 2: " << std::to_string(Healths[2].Max) << " / " << std::to_string(Healths[2].Value) << std::endl;
	std::cout << "Health 3: " << std::to_string(Healths[3].Max) << " / " << std::to_string(Healths[3].Value) << std::endl;

	struct FJob : public test::TJobForEach<test::CHealth>
	{
		void Execute(test::CHealth& Health) override
		{
			++Health.Max;
			Health.Value = Health.Max;
		}
	};

//	std::for_each(Healths.begin(), Healths.end(), FJob());
//
//	std::cout << "Health 0: " << std::to_string(Healths[0].Max) << " / " << std::to_string(Healths[0].Value) << std::endl;
//	std::cout << "Health 1: " << std::to_string(Healths[1].Max) << " / " << std::to_string(Healths[1].Value) << std::endl;
//	std::cout << "Health 2: " << std::to_string(Healths[2].Max) << " / " << std::to_string(Healths[2].Value) << std::endl;
//	std::cout << "Health 3: " << std::to_string(Healths[3].Max) << " / " << std::to_string(Healths[3].Value) << std::endl;

	auto A = test::ForEach(Healths.begin(), Healths.end(), FJob());

	std::cout << "Health 0: " << std::to_string(Healths[0].Max) << " / " << std::to_string(Healths[0].Value) << std::endl;
	std::cout << "Health 1: " << std::to_string(Healths[1].Max) << " / " << std::to_string(Healths[1].Value) << std::endl;
	std::cout << "Health 2: " << std::to_string(Healths[2].Max) << " / " << std::to_string(Healths[2].Value) << std::endl;
	std::cout << "Health 3: " << std::to_string(Healths[3].Max) << " / " << std::to_string(Healths[3].Value) << std::endl;

	return 0;
}