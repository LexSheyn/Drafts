#include "XArchetype.h"

#include <iostream>
#include <string>
#include <vector>

struct CTranslation
{
	int64_t X;
	int64_t Y;
	int64_t Z;
};

struct CRotation
{
	int64_t X;
	int64_t Y;
	int64_t Z;
};

struct CScale
{
	int64_t X;
	int64_t Y;
	int64_t Z;
};

int32_t main(int32_t ArgC, const char* ArgV)
{
	t3d::FComponentManager::RegisterComponent<CTranslation>();
	t3d::FComponentManager::RegisterComponent<CRotation>();
	t3d::FComponentManager::RegisterComponent<CScale>();

	t3d::XEntityWorld World;

	auto Entity0 = World.CreateEntity();
	auto Entity1 = World.CreateEntity();
	auto Entity2 = World.CreateEntity();

	World.AddComponent(Entity0, CTranslation{ 10, 10, 10 });
	World.AddComponent(Entity0, CRotation   { 11, 11, 11 });

	World.AddComponent(Entity1, CTranslation{ 20, 20, 20 });
	World.AddComponent(Entity1, CRotation   { 21, 21, 21 });

	World.AddComponent(Entity2, CTranslation{ 30, 30, 30 });
	World.AddComponent(Entity2, CRotation   { 31, 31, 31 });
//	World.AddComponent(Entity2, CScale      { 32, 32, 32 });

	struct FPrintJob : t3d::IJobForEach<CTranslation, CRotation>
	{
		void Execute(CTranslation& Translation, CRotation& Rotation) override
		{
			std::cout << "Translation: " << Translation.X << " " << Translation.Y << " " << Translation.Z << std::endl;
			std::cout << "Rotation:    " << Rotation   .X << " " << Rotation   .Y << " " << Rotation   .Z << std::endl;
		}
	};

	World.ForEachOnly<CTranslation, CRotation>(std::make_unique<FPrintJob>());

	struct A
	{
		void FunctionA()
		{}
	};

	struct B : A
	{
		void FunctionB()
		{
			this->FunctionA();
		}
	};

	B b;

	b.FunctionA();

//	std::vector<char> Vector;
//
//	Vector.push_back('Q');
//	Vector.push_back('W');
//	Vector.push_back('E');
//	Vector.push_back('R');
//	Vector.push_back('T');
//	Vector.push_back('Y');
//
//	std::cout << "Vector: ";
//
//	for (auto& Element : Vector)
//	{
//		std::cout << Element;
//	}
//
//	std::cout << std::endl;
//
//#pragma warning(push)
//#pragma warning(disable: 4996)
//
//	std::memcpy(&Vector[0], &Vector[0], 3);
//
//#pragma warning(pop)
//
//	std::cout << "Vector: ";
//
//	for (auto& Element : Vector)
//	{
//		std::cout << Element;
//	}

	return 0;
}