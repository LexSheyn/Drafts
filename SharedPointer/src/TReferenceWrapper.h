#pragma once

#include "TrickUtility.h"

#include <type_traits>

namespace t3d
{
	// Not implemented!
	template<typename T>
	void ReferenceWrapper_Constructor_Function(std::_Identity_t<T&>) noexcept {}

	template<typename T>
	void ReferenceWrapper_Constructor_Function(std::_Identity_t<T&&>) = delete;

	template<typename T, typename X, typename = void>
	struct ReferenceWrapper_Has_Constructor_From_T : std::false_type {};

	template<typename T, typename X>
	struct ReferenceWrapper_Has_Constructor_From_T<T, X, std::void_t<decltype(ReferenceWrapper_Constructor_Function<T>(DeclaredValue<X>()))>> : std::true_type {};

	template<typename T>
	class TReferenceWrapper
	{
	public:

		static_assert(std::is_object_v<T> || std::is_function_v<T>, "T required to be an object type or a function type!");

		using type = T;

		template<typename X, std::enable_if_t<std::conjunction_v<std::negation<std::is_same<std::remove_cvref_t<X>, TReferenceWrapper>>, ReferenceWrapper_Has_Constructor_From_T<T, X>>, int32> = 0>
		constexpr TReferenceWrapper(X&& Value) noexcept(noexcept(ReferenceWrapper_Constructor_Function<T>(DeclaredValue<X>())))
		{
			T& Reference = static_cast<X&&>(Value);

			Pointer = AddressOf(Reference);
		}

		T3D_NO_DISCARD constexpr T& Get() const noexcept
		{
			return *Pointer;
		}

		constexpr operator T& () const noexcept
		{
			return *Pointer;
		}

		template<typename... Args_T>
		constexpr auto operator () (Args_T&&... Args) const noexcept(noexcept(std::invoke(*Pointer, static_cast<Args_T&&>(Args)...))) -> decltype(std::invoke(*Pointer, static_cast<Args_T&&>(Args)...))
		{
			return std::invoke(*Pointer, static_cast<Args_T&&>(Args)...);
		}

	private:

		T* Pointer {};
	};

	template<typename T>
	TReferenceWrapper(T&) -> TReferenceWrapper<T>;

	template<typename T>
	T3D_NO_DISCARD constexpr TReferenceWrapper<T> Reference(T& Value) noexcept
	{
		return TReferenceWrapper<T>(Value);
	}

	template<typename T>
	void Reference(const T&&) = delete;

	template<typename T>
	T3D_NO_DISCARD constexpr TReferenceWrapper<T> Reference(TReferenceWrapper<T> Value) noexcept
	{
		return Value;
	}

	template<typename T>
	T3D_NO_DISCARD constexpr TReferenceWrapper<const T> ConstReference(const T& Value) noexcept
	{
		return TReferenceWrapper<const T>(Value);
	}

	template<typename T>
	void ConstReference(const T&&) = delete;

	template<typename T>
	T3D_NO_DISCARD constexpr TReferenceWrapper<const T> ConstReference(TReferenceWrapper<T> Value) noexcept
	{
		return Value;
	}

	template<typename T>
	struct TUnwrap_Reference
	{
		using type = T;
	};

	template<typename T>
	struct TUnwrap_Reference<TReferenceWrapper<T>>
	{
		using type = T&;
	};

	template<typename T>
	using Unwrap_Reference_T = typename TUnwrap_Reference<T>::type;

	template<typename T>
	using Unwrap_Reference_Decay_T = Unwrap_Reference_T<std::decay_t<T>>;

	template<typename T>
	struct TUnwrap_Reference_Decay
	{
		using type = Unwrap_Reference_Decay_T<T>;
	};
}