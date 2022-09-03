#include "Templates/TFunction.h"

#include <iostream>

int32_t main(int32_t ArgC, const char* ArgV)
{
	t3d::TFunction<t3d::int32(t3d::float32)> Function = [](t3d::float32) ->t3d::int32 { return 0; };
	t3d::TFunction<t3d::int32(t3d::float32)> Function1 = nullptr;

	Function1 = nullptr;

	auto Lambda = [](t3d::float32 Float) ->t3d::int32 { std::cout << "t3d::TFunction: " << Float << std::endl; return 0; };

	Function = Lambda;

	t3d::float32 Float = 0.0f;

	Function(Float);

	class A {};
	class B : public A {};

	t3d::TSharedPointer<A> a;
	t3d::TSharedPointer<B> b;

	auto Result = (a == b);
	Result = (a == nullptr);
	Result = (nullptr == b);

	if (Function.ContainsFunctorOfType<decltype([](t3d::float32) ->t3d::int32 { return 0; }) > ())
	{
		std::cout << "t3d::TFunction contains lambda." << std::endl;
	}
	else
	{
		std::cout << "t3d::TFunction does not contain lambda." << std::endl;
	}

	if (Function.ContainsFunctorOfType<decltype(Lambda)>())
	{
		std::cout << "t3d::TFunction contains lambda." << std::endl;
	}
	else
	{
		std::cout << "t3d::TFunction does not contain lambda." << std::endl;
	}

	if (Function.HasSameInstance(Function1))
	{
		std::cout << "Same instance." << std::endl;
	}
	else
	{
		std::cout << "Not the same instances." << std::endl;
	}

	if (Function.HasSameMemberFunction(Function1))
	{
		std::cout << "Same member function." << std::endl;
	}
	else
	{
		std::cout << "Not the same member function." << std::endl;
	}

	if (Function.HasSameStaticFunction(Function1))
	{
		std::cout << "Same static function." << std::endl;
	}
	else
	{
		std::cout << "Not the same static function." << std::endl;
	}

	if (Function.IsEmpty())
	{
		std::cout << "Function is empty." << std::endl;
	}
	else
	{
		std::cout << "Function is not empty." << std::endl;
	}

	if (Function1.IsEmpty())
	{
		std::cout << "Function1 is empty." << std::endl;
	}
	else
	{
		std::cout << "Function1 is not empty." << std::endl;
	}

	t3d::TFunction<void()> EmptyFunction;

	try
	{
		EmptyFunction();
	}
	catch (const std::exception& Exception)
	{
		std::cout << "Exception thrown: " << Exception.what() << std::endl;
	}

	return 0;
}