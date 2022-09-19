#pragma once

#include <exception>

namespace t3d
{
	typedef unsigned long long Size_T;
	typedef bool bool8;

#define T3D_NO_DISCARD [[nodiscard]]
#define T3D_NO_RETURN [[noreturn]]
#define T3D_INLINE inline

	T3D_NO_RETURN T3D_INLINE void Throw_Bad_Array_New_Length()
	{
		throw std::bad_array_new_length();
	}

	template<typename Size_T TypeSize>
	T3D_NO_DISCARD constexpr Size_T Get_Size_Of_N(const Size_T Count)
	{
		constexpr bool8 OverflowIsPossible = TypeSize > 1;

		if constexpr (OverflowIsPossible)
		{
			constexpr Size_T MaxPossible = static_cast<Size_T>(-1) / TypeSize;

			if (Count > MaxPossible)
			{
				Throw_Bad_Array_New_Length();
			}
		}

		return Count * TypeSize;
	}

	// <xmemory> 68
}