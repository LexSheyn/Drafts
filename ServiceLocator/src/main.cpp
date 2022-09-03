#include <iostream>

#include "FServiceLocator.h"

struct FEventSystem
{
	int Event1;
	int Event2;
	int Event3;
};

int32_t main(int32_t ArgC, char* ArgV[])
{
	sl::FServiceLocator::Create<FEventSystem>();

	std::cout << "Event 1:" << sl::FServiceLocator::Get<FEventSystem>().Event1 << std::endl;
	std::cout << "Event 2:" << sl::FServiceLocator::Get<FEventSystem>().Event2 << std::endl;
	std::cout << "Event 3:" << sl::FServiceLocator::Get<FEventSystem>().Event3 << std::endl;

	sl::FServiceLocator::Get<FEventSystem>().Event1 = 1;
	sl::FServiceLocator::Get<FEventSystem>().Event2 = 2;
	sl::FServiceLocator::Get<FEventSystem>().Event3 = 3;

	std::cout << "Event 1:" << sl::FServiceLocator::Get<FEventSystem>().Event1 << std::endl;
	std::cout << "Event 2:" << sl::FServiceLocator::Get<FEventSystem>().Event2 << std::endl;
	std::cout << "Event 3:" << sl::FServiceLocator::Get<FEventSystem>().Event3 << std::endl;

	sl::FServiceLocator::Destroy<FEventSystem>();

	return 0;
}