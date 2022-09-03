#pragma once

#include <vector>
#include <unordered_map>
#include <set>

using ComponentId_T = uint64_t;
using ArchetypeId_T = uint64_t;
using EntityId_T    = uint64_t;

// A list of component ids:

using Type_T = std::vector<ComponentId_T>;


// Type used to store each unique component list only once:

using Column_T = std::vector<void*>;

struct FColumn
{
	void*  Elements;
	size_t ElementSize;
	size_t Count;
};

struct FEntityRecord
{
	FArchetype* Archetype;
	size_t      Row;
};

struct FArchetype
{
	ArchetypeId_T         Id;
	Type_T                Type;
	std::vector<Column_T> Columns;
};


// Find an archetype bu its list of component ids:

std::unordered_map<Type_T, FArchetype> ArchetypeIndex;


// Find the archetype for an entity:

std::unordered_map<EntityId_T, FEntityRecord> EntityIndex;


// Find the archetypes for a component:

using ArchetypeSet_T = std::set<ArchetypeId_T>;

struct FArchetypeRecord
{
	size_t Column;
};

using ArchetypeMap_T = std::unordered_map<ArchetypeId_T, FArchetypeRecord>;

std::unordered_map<ComponentId_T, ArchetypeMap_T> ComponentIndex;

void* GetComponent(EntityId_T Entity, ComponentId_T ComponentId)
{
	FEntityRecord& Record = EntityIndex[Entity];

	FArchetype& Archetype = *Record.Archetype;

	ArchetypeMap_T& Archetypes = ComponentIndex[ComponentId];

	if (Archetypes.contains(Archetype.Id))
	{
		return Archetype.Columns[Archetypes[Archetype.Id].Column][Record.Row];
	}

	return nullptr;
}