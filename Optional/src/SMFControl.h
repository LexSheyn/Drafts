#pragma once

#include <type_traits>

template<typename Base_T>
struct FNonTrivialCopy : Base_T
{
	using Base_T::Base_T;

	FNonTrivialCopy () = default;

	constexpr FNonTrivialCopy(const FNonTrivialCopy& Right) noexcept(noexcept(Base_T::))
	{

	}
};