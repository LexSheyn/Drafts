#pragma once

#include <cstdint>
#include <vector>

namespace t3d
{
	using EntityId_T         = uint64_t;
	using EntityGeneration_T = uint64_t;

	using ComponentId_T = uint64_t;

	using ComponentSignature_T = std::vector<ComponentId_T>;
}