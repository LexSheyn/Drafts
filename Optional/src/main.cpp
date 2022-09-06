#include <cstdint>
#include <iostream>

#include <optional>
#include "TOptional.h"

int32_t main()
{
	t3d::TOptional<int32_t> Int;

	std::cout << Int.HasValue() << std::endl;

	Int = 322;

	std::cout << Int.Value() << std::endl;

	Int.Reset();

	std::cout << Int.HasValue() << std::endl;

	Int.Emplace(100500);

	std::cout << Int.Value() << std::endl;

	Int = t3d::NullOptional;

	std::cout << Int.HasValue() << std::endl;

	Int = 1337;

	std::cout << Int.Value() << std::endl;

	Int = {};

	std::cout << Int.HasValue() << std::endl;

	return 0;
}