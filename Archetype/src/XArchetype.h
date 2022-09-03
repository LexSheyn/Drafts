#pragma once

#include "FComponentManager.h"
#include "FDataVector.h"
#include "Templates/THash.h"

#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <type_traits>
#include <memory>

namespace t3d
{
	template<>
	struct THash<ComponentSignature_T>
	{
		size_t operator () (const ComponentSignature_T& Signature) const
		{
			std::string SignatureString;

			SignatureString.reserve(Signature.size() * 2);
		
			for (auto& ComponentId : Signature)
			{
				SignatureString += std::to_string(ComponentId);
			}

			return std::hash<std::string>()(SignatureString);
		}
	};

	class XArchetype
	{
	public:

		XArchetype(ComponentSignature_T Signature)
			: ComponentSignature (Signature)
		{
			if (ComponentSignature.empty())
			{
				return;
			}

			size_t LargestId = ComponentSignature[0];

			for (size_t i = 1u; i < ComponentSignature.size(); ++i)
			{
				if (LargestId < ComponentSignature[i])
				{
					LargestId = ComponentSignature[i];
				}
			}

			ComponentVectors.resize(LargestId + 1u);

			for (auto& ComponentId : ComponentSignature)
			{
				ComponentVectors[ComponentId].SetElementSize(FComponentManager::GetComponentSize(ComponentId));
			}
		}

		void AddEntity(EntityId_T Entity)
		{
			Entities.push_back(Entity);

			for (auto& ComponentId : ComponentSignature)
			{
				ComponentVectors[ComponentId].Resize(Entities.size());
			}
		}

		void RemoveEntity(EntityId_T Entity)
		{
			size_t Index = SIZE_MAX;

			for (size_t i = 0u; i < Entities.size(); ++i)
			{
				if (Entities[i] == Entity)
				{
					Index = i;

					break;
				}
			}

			Entities[Index] = Entities.back();

			Entities.pop_back();

			for (auto& ComponentId : ComponentSignature)
			{
				ComponentVectors[ComponentId].CopyValue(Index, Entities.size());

				ComponentVectors[ComponentId].PopBack();
			}
		}

		void TransferEntityFromArchetype(XArchetype& Right, EntityId_T Entity)
		{
			this->AddEntity(Entity);

			size_t ThisEntityIndex  = this->GetEntityIndex(Entity);
			size_t RightEntityIndex = Right.GetEntityIndex(Entity);

			const ComponentSignature_T& ThisComponentSignature  = this->GetComponentSignature();
			const ComponentSignature_T& RightComponentSignature = Right.GetComponentSignature();

			size_t ThisComponentSignatureSize  = ThisComponentSignature .size();
			size_t RightComponentSignatureSize = RightComponentSignature.size();

			std::vector<size_t> SameComponentIds;

			for (size_t i = 0u; i < ThisComponentSignatureSize; ++i)
			{
				for (size_t j = 0u; j < RightComponentSignatureSize; ++j)
				{
					if (ThisComponentSignature[i] == RightComponentSignature[j])
					{
						SameComponentIds.push_back(ThisComponentSignature[i]);
					}
				}
			}

			for (auto& ComponentId : SameComponentIds)
			{
				ComponentVectors[ComponentId].CopyValueFromVector(Right.ComponentVectors[ComponentId], ThisEntityIndex, RightEntityIndex);
			}

			Right.RemoveEntity(Entity);
		}

		constexpr bool IsEmpty() const noexcept
		{
			return Entities.empty();
		}

		constexpr size_t GetEntityCount() const noexcept
		{
			return Entities.size();
		}

		size_t GetEntityIndex(EntityId_T Entity) const noexcept
		{
			size_t Index = SIZE_MAX;

			for (size_t i = 0u; i < Entities.size(); ++i)
			{
				if (Entities[i] == Entity)
				{
					Index = i;

					break;
				}
			}

			return Index;
		}

		const ComponentSignature_T& GetComponentSignature() const noexcept
		{
			return ComponentSignature;
		}

		std::vector<FDataVector>& GetComponentVectors() noexcept
		{
			return ComponentVectors;
		}

		template<typename T>
		FDataVector& GetComponentVector()
		{
			return ComponentVectors[TComponentInfo<T>::Id];
		}

		FDataVector& GetComponentVector(ComponentId_T Id)
		{
			return ComponentVectors[Id];
		}

		template<typename T>
		T& GetComponent(EntityId_T Entity)
		{
			return ComponentVectors[TComponentInfo<T>::Id].operator[]<T>(this->GetEntityIndex(Entity));
		}

	private:

		ComponentSignature_T     ComponentSignature;
		std::vector<EntityId_T>  Entities;
		std::vector<FDataVector> ComponentVectors;
	};

	template<typename... Components_T>
	struct IJobForEach
	{
		virtual void operator () (Components_T&... Components) = 0;
	};

	template<template<typename> typename Tuple_T, typename... Args_T>
	inline auto CreateTuple(Tuple_T<Args_T...>& Tuple)
	{
		return std::tie(std::get<Args_T>(Tuple)...);
	}

	template<typename Tuple_T, size_t... Indices>
	inline auto CopyIndexedTuple(Tuple_T& Tuple, std::index_sequence<Indices...>)
	{
		return std::tie(std::get<Indices>(Tuple)...);
	}

	template<typename Tuple_T>
	inline auto CopyTuple(Tuple_T& Tuple)
	{
		return CopyIndexedTuple(Tuple, std::make_index_sequence<std::tuple_size<Tuple_T>::value>{});
	}

	template<typename Function_T, typename Tuple_T, size_t... Indices>
	inline auto CallIndexedArgumentSequence(Function_T& Function, Tuple_T& Tuple, std::index_sequence<Indices...>)
	{
		return Function(std::get<Indices>(Tuple)...);
	}

	template<typename Function_T, typename Tuple_T>
	inline auto CallArgumentSequence(Function_T& Function, Tuple_T&& Tuple)
	{
		return CallIndexedArgumentSequence(Function, Tuple, std::make_index_sequence<std::tuple_size<Tuple_T>::value>{});
	}

	class XEntityWorld
	{
	public:

	// Entity API:

		EntityId_T CreateEntity()
		{
			EntityId_T Entity = SIZE_MAX;

			if (RemovedEntities.empty())
			{
				Entity = Generations.size();

				Generations        .push_back(0);
				ComponentSignatures.push_back(ComponentSignature_T());
			}
			else
			{
				Entity = RemovedEntities.front();

				RemovedEntities.front() = RemovedEntities.back();

				RemovedEntities.pop_back();
			}

			ComponentSignature_T& ComponentSignature = ComponentSignatures[Entity];

			if (Archetypes.contains(ComponentSignature) == false)
			{
				Archetypes.insert({ ComponentSignature, XArchetype(ComponentSignature) });
			}

			Archetypes.at(ComponentSignature).AddEntity(Entity);

			return Entity;
		}

		void RemoveEntity(EntityId_T Entity)
		{
			ComponentSignature_T& ComponentSignature = ComponentSignatures[Entity];

			XArchetype& Archetype = Archetypes.at(ComponentSignature);

			Archetype.RemoveEntity(Entity);

			if (Archetype.IsEmpty())
			{
				Archetypes.erase(ComponentSignature);
			}

			++Generations[Entity];

			ComponentSignature.clear();

			RemovedEntities.push_back(Entity);
		}

	// Component API:

		template<typename T>
		void AddComponent(EntityId_T Entity, T Component)
		{
			ComponentSignature_T& ComponentSignature    = ComponentSignatures[Entity];
			ComponentSignature_T  OldComponentSignature = ComponentSignature;

			XArchetype& OldArchetype = Archetypes.at(OldComponentSignature);

			FComponentManager::AddComponentsToSignature<T>(ComponentSignature);

			if (Archetypes.contains(ComponentSignature) == false)
			{
				Archetypes.insert({ ComponentSignature, XArchetype(ComponentSignature) });
			}

			XArchetype& NewArchetype = Archetypes.at(ComponentSignature);

			NewArchetype.TransferEntityFromArchetype(OldArchetype, Entity);

			NewArchetype.GetComponent<T>(Entity) = Component;

			if (OldArchetype.IsEmpty())
			{
				Archetypes.erase(OldComponentSignature);
			}
		}

		template<typename T>
		void RemoveComponent(EntityId_T Entity)
		{
			ComponentSignature_T& ComponentSignature    = ComponentSignatures[Entity];
			ComponentSignature_T  OldComponentSignature = ComponentSignature;

			XArchetype& OldArchetype = Archetypes.at(OldComponentSignature);

			FComponentManager::RemoveComponentsFromSignature<T>(ComponentSignature);

			if (Archetypes.contains(ComponentSignature) == false)
			{
				Archetypes.insert({ ComponentSignature, XArchetype(ComponentSignature) });
			}

			XArchetype& NewArchetype = Archetypes.at(ComponentSignature);

			NewArchetype.TransferEntityFromArchetype(OldArchetype, Entity);

			if (OldArchetype.IsEmpty())
			{
				Archetypes.erase(OldComponentSignature);
			}
		}

		template<typename T>
		T& GetComponent(EntityId_T Entity)
		{
			return Archetypes.at(ComponentSignatures[Entity]).GetComponent<T>(Entity);
		}

		template<typename... Components_T>
		XArchetype& GetArchetype()
		{
			return Archetypes.at(FComponentManager::CreateComponentSignature<Components_T...>());
		}

		XArchetype& GetArchetype(EntityId_T Entity)
		{
			return Archetypes.at(ComponentSignatures[Entity]);
		}

		EntityGeneration_T GetGeneration(EntityId_T Entity) const
		{
			return Generations[Entity];
		}

		ComponentSignature_T GetComponentSignature(EntityId_T Entity) const
		{
			return ComponentSignatures[Entity];
		}

	// Queries:

		template<typename... Components_T>
		void ForEachOnly(auto Job)
		{
			static_assert(std::is_base_of<IJobForEach<Components_T...>, decltype(Job)>::value);

			auto ArchetypeIterator = Archetypes.find(FComponentManager::CreateComponentSignature<Components_T...>());

			if (ArchetypeIterator != Archetypes.end())
			{
				XArchetype& Archetype = (*ArchetypeIterator).second;

				auto& ComponentVectors = Archetype.GetComponentVectors();

				for (size_t i = 0u; i < Archetype.GetEntityCount(); ++i)
				{
					CallArgumentSequence(Job, std::tie([&]() -> auto& { return ComponentVectors[TComponentInfo<Components_T>::Id].operator[]<Components_T>(i); } () ...));
				}
			}
		}

	private:

		std::vector<EntityGeneration_T>   Generations;
		std::vector<ComponentSignature_T> ComponentSignatures;
		std::vector<EntityId_T>           RemovedEntities;

		std::unordered_map<ComponentSignature_T, XArchetype, THash<ComponentSignature_T>> Archetypes;
	};

	class FEntityQuery
	{
		template<typename T1>
		static void ForEachOnly(XEntityWorld& World, auto Job)
		{
			XArchetype& Archetype = World.GetArchetype<T1>();

			FDataVector& ComponentVector = Archetype.GetComponentVector<T1>();

			for (size_t i = 0u; i < ComponentVector.Size(); ++i)
			{
				Job(ComponentVector[i]);
			}
		}
	};
}