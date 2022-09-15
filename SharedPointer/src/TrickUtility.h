#pragma once

#include <type_traits>
#include <utility>
#include <memory>

namespace t3d
{
	typedef int int32;

	// False value attached to a dependent name (for static_assert).
	template<typename>	
	inline constexpr bool Always_False_V = false;

	template<typename T>
	[[nodiscard]] constexpr T* AddressOf(T& Value) noexcept
	{
		return __builtin_addressof(Value);
	}

	template<typename T>
	const T* AddressOf(const T&&) = delete;

	// MSVC _Unfancy. Extract plain pointer from smart pointer.
	template<typename Pointer_T>
	[[nodiscard]] constexpr auto Unwrap_Pointer(Pointer_T Pointer) noexcept
	{
		return AddressOf(*Pointer);
	}

	// MSVC _Unfancy. Does nothing for plain pointers.
	template<typename T>
	[[nodiscard]] constexpr T* Unwrap_Pointer(T* Pointer) noexcept
	{
		return Pointer;
	}

	// MSVC _Refancy. Wrap plain pointer into smart pointer.
	template<typename Pointer_T, std::enable_if_t<!std::is_pointer_v<Pointer_T>, int32> = 0>
	constexpr Pointer_T Wrap_Pointer(typename std::pointer_traits<Pointer_T>::element_type* Pointer) noexcept
	{
		return std::pointer_traits<Pointer_T>::pointer_to(*Pointer);
	}

	// MSVC _Refancy. Does nothing for plain pointers.
	template<typename Pointer_T, std::enable_if_t<std::is_pointer_v<Pointer_T>, int32> = 0>
	constexpr Pointer_T Wrap_Pointer(Pointer_T Pointer) noexcept
	{
		return Pointer;
	}

	template<typename Iterator_T>
	[[nodiscard]] constexpr void* Voidify_Iterator(Iterator_T Iterator) noexcept
	{
		if constexpr (std::is_pointer_v<Iterator_T>)
		{
			return const_cast<void*>(static_cast<const volatile void*>(Iterator));
		}
		else
		{
			return const_cast<void*>(static_cast<const volatile void*>(AddressOf(*Iterator)));
		}
	}

	template<typename T>
	std::add_rvalue_reference_t<T> DeclaredValue() noexcept
	{
		static_assert(Always_False_V<T>, "DeclaredValue is not allowed in an evaluated context!");
	}

	template<typename T, typename... Args_T, typename = std::void_t<decltype(new(DeclaredValue<void*>()) T(DeclaredValue<Args_T>()...))>>
	constexpr T* Construct_At(const T* Location, Args_T&&... Args) noexcept(noexcept(new(Voidify_Iterator(Location)) T(std::forward<Args_T>(Args)...)))
	{
		return new (Voidify_Iterator(Location)) T(std::forward<Args_T>(Args)...);
	}

	template<typename T, typename... Args_T>
	constexpr void Construct_In_Place(T& Object, Args_T&&... Args) noexcept(std::is_nothrow_constructible_v<T, Args_T...>)
	{
		if (std::is_constant_evaluated())
		{
			Construct_At(AddressOf(Object), std::forward<Args_T>(Args)...);
		}
		else
		{
			new (Voidify_Iterator(AddressOf(Object))) T(std::forward<Args_T>(Args)...);
		}
	}

	template<typename T>
	void Default_Construct_In_Place(T& Object) noexcept(std::is_nothrow_default_constructible_v<T>)
	{
		new (Voidify_Iterator(AddressOf(Object))) T;
	}

	template<typename NoThrowForwardIterator_T, typename NoThrowSentinel_T>
	constexpr void Destroy_Range(NoThrowForwardIterator_T First, const NoThrowSentinel_T Last) noexcept;

	template<typename T>
	constexpr void Destroy_In_Place(T& Object) noexcept
	{
		if constexpr (std::is_array_v<T>)
		{
			Destroy_Range(Object, Object + std::extent_v<T>);
		}
		else
		{
			Object.~T();
		}
	}

	template<typename NoThrowForwardIterator_T, typename NoThrowSentinel_T>
	constexpr void Destroy_Range(NoThrowForwardIterator_T First, const NoThrowSentinel_T Last) noexcept
	{
		if constexpr (!std::is_trivially_destructible_v<std::iter_value_t<NoThrowForwardIterator_T>>)
		{
			for (; First != Last; ++First)
			{
				Destroy_In_Place(*First);
			}
		}
	}

	template<typename T>
	constexpr void Destroy_At(const T* Location) noexcept
	{
		if constexpr (std::is_array_v<T>)
		{
			Destroy_Range(std::begin(*Location), std::end(*Location));
		}
		else
		{
			Location->~T();
		}
	}
}