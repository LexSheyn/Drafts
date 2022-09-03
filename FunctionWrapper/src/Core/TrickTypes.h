#pragma once

namespace t3d
{
	typedef          bool      bool8;

//	typedef          char      char;

	typedef          char8_t   char8;
	typedef          char16_t  char16;
	typedef          char32_t  char32;

	typedef signed   char      int8;
	typedef signed   short     int16;
	typedef signed   int       int32;
	typedef signed   long long int64;

	typedef unsigned char      uint8;
	typedef unsigned short     uint16;
	typedef unsigned int       uint32;
	typedef unsigned long long uint64;

	typedef float              float32;
	typedef double             float64;

	typedef uint32 Flags_T;

	typedef uint64 Size_T;

	using ComponentSignature_T = uint64;
	using ComponentIndex_T     = uint32; // 32-bit

	using EntityGeneration_T   = uint64;	
	using EntityId_T           = uint64;

#define T3D_ECS_INVALID_ENTITY_ID         0xffffffffffffffffui64 // UINT64_MAX
#define T3D_ECS_INVALID_COMPONENT_INDEX   0xffffffffui32         // UINT32_MAX
#define T3D_ECS_COMPONENT_LIMIT           64
#define T3D_ECS_EMPTY_COMPONENT_SIGNATURE 0
}