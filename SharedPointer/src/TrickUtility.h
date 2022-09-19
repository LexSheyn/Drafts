#pragma once

#include <type_traits>
#include <utility>
#include <memory>
#include <cstdint>

namespace t3d
{
#if defined _WIN64 && _MSC_VER
#define T3D_NO_VTABLE __declspec(novtable)
#else
#define T3D_NO_VTABLE
#endif

#define T3D_NO_DISCARD [[nodiscard]]
#define T3D_NO_RETURN  [[noreturn]]
#define T3D_INLINE inline
#define T3D_ASSERT(Expression, Message) assert(Expression)
#define T3D_ASSERT(Expression) assert(Expression)

	typedef bool bool8;
	typedef int  int32;
	typedef unsigned long long Size_T;

	

	// False value attached to a dependent name (for static_assert).
	template<typename>	
	T3D_INLINE constexpr bool8 Always_False_V = false;

	template<typename T>
	T3D_NO_DISCARD constexpr T&& Forward(std::remove_reference_t<T>& Argument) noexcept
	{
		return static_cast<T&&>(Argument);
	}

	template<typename T>
	T3D_NO_DISCARD constexpr T&& Forward(std::remove_reference_t<T>&& Argument) noexcept
	{
		static_assert(!std::is_rvalue_reference_v<T>, "Argument must not be lvalue reference!");

		return static_cast<T&&>(Argument);
	}

	template<typename T>
	T3D_NO_DISCARD constexpr std::remove_reference_t<T>&& Move(T&& Argument) noexcept
	{
		return static_cast<std::remove_reference_t<T>&&>(Argument);
	}

	template<typename T>
	T3D_NO_DISCARD constexpr std::conditional_t<!std::is_nothrow_move_constructible_v<T>&& std::is_copy_constructible_v<T>, const T&, T&&> Move_If_Noexcept(T& Argument) noexcept
	{
		return Move(Argument);
	}

	template<typename T>
	T3D_NO_DISCARD constexpr T* AddressOf(T& Value) noexcept
	{
		return __builtin_addressof(Value);
	}

	template<typename T>
	const T* AddressOf(const T&&) = delete;

	// MSVC _Unfancy. Extract plain pointer from smart pointer.
	template<typename Pointer_T>
	T3D_NO_DISCARD constexpr auto Unwrap_Pointer(Pointer_T Pointer) noexcept
	{
		return AddressOf(*Pointer);
	}

	// MSVC _Unfancy. Does nothing for plain pointers.
	template<typename T>
	T3D_NO_DISCARD constexpr T* Unwrap_Pointer(T* Pointer) noexcept
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
	T3D_NO_DISCARD constexpr void* Voidify_Iterator(Iterator_T Iterator) noexcept
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

	struct Identity_T
	{
		template<typename T>
		T3D_NO_DISCARD constexpr T&& operator () (T&& Right) const noexcept
		{
			return Forward<T>(Right);
		}

		using IsTransparent_T = int32;
	};

	template<typename T>
	struct std::pointer_traits<T*>
	{
		using pointer = T*;
		using element_type = T;
		using difference_type = std::ptrdiff_t;

		template<typename X>
		using rebind = X*;

		using Reference_T = std::conditional_t<std::is_void_v<T>, char, T>&;

		T3D_NO_DISCARD static constexpr pointer PointerTo(Reference_T Value) noexcept
		{
			return AddressOf(Value);
		}
	};

	template<typename T>
	T3D_NO_DISCARD constexpr T* ToAddress(T* const Value) noexcept
	{
		static_assert(!std::is_function_v<T>, "T must not be function type!");

		return Value;
	}

	template<typename Pointer_T>
	T3D_NO_DISCARD constexpr auto ToAddress(const Pointer_T& Pointer) noexcept
	{
		if constexpr (requires{ std::pointer_traits<Pointer_T>::to_address(Pointer); })
		{
			return std::pointer_traits<Pointer_T>::to_address(Pointer);
		}
		else
		{
			return ToAddress(Pointer.operator->()); // MSVC: Plain pointer overload must come first!
		}
	}

	template<typename Iterator_T>
	T3D_INLINE constexpr bool8 Iterator_Is_Contiguous_V = std::contiguous_iterator<Iterator_T>;

	template<typename Iterator_T>
	T3D_NO_DISCARD constexpr auto To_Address(const Iterator_T& Iterator) noexcept
	{
		static_assert(std::contiguous_iterator<Iterator_T>);

		return ToAddress(Iterator);
	}

	template<typename Iterator_T>
	T3D_NO_DISCARD constexpr auto To_Address(const std::move_iterator<Iterator_T>& Iterator) noexcept
	{
		return To_Address(Iterator.base());
	}
}