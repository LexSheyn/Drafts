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

void Function(int32_t Int, float_t Float)
{
	std::cout << "Int:   " << Int   << std::endl;
	std::cout << "Float: " << Float << std::endl;
}

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
	World.AddComponent(Entity2, CScale      { 32, 32, 32 });

	World.RemoveComponent<CScale>(Entity2);

	auto Entity3 = World.CreateEntity();

	World.AddComponent(Entity3, CTranslation{ 40, 40, 40 });
	World.AddComponent(Entity3, CRotation   { 41, 41, 41 });

	//World.RemoveEntity(Entity0);
	//World.RemoveEntity(Entity1);
	//World.RemoveEntity(Entity2);
	//World.RemoveEntity(Entity3);

	struct FPrintJob : t3d::IJobForEach<CTranslation, CRotation>
	{
		void Execute(CTranslation& Translation, CRotation& Rotation) override
		{
			std::cout << "Translation: " << Translation.X << " " << Translation.Y << " " << Translation.Z << std::endl;
			std::cout << "Rotation:    " << Rotation   .X << " " << Rotation   .Y << " " << Rotation   .Z << std::endl;
		}
	};

	struct FModifyJob : t3d::IJobForEach<CTranslation, CRotation>
	{
		void Execute(CTranslation& Translation, CRotation& Rotation) override
		{
			Translation.X *= 1488;
			Rotation.Z *= 322;
		}
	};

	struct FPrintEntityJob : t3d::IJobForEachWithEntity<CTranslation, CRotation>
	{
		void Execute(t3d::EntityId_T Entity, CTranslation& Translation, CRotation& Rotation) override
		{
			std::cout << "Entity:      " << Entity << std::endl;
			std::cout << "Translation: " << Translation.X << " " << Translation.Y << " " << Translation.Z << std::endl;
			std::cout << "Rotation:    " << Rotation   .X << " " << Rotation   .Y << " " << Rotation   .Z << std::endl;
		}
	};

//	World.ForEachOnly<CTranslation, CRotation>(FPrintJob());
	World.ForEachWith<CTranslation, CRotation>(FPrintJob());

	World.ForEachOnly<CTranslation, CRotation>(FModifyJob());

//	World.ForEachWithEntityOnly<CTranslation, CRotation>(FPrintEntityJob());
	World.ForEachWithEntityWith<CTranslation, CRotation>(FPrintEntityJob());

	std::tuple<int32_t, float_t> Tuple = { 1488, 322.0f };

	auto Result = t3d::CreateTupleOfReferences(Tuple);

	t3d::CallWithArgumentsFromTuple(Function, Result);

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