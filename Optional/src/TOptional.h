#pragma once

#include <utility>
#include <exception>
#include <new>
#include <type_traits>
#include <cassert>
#include <cstdint>
#include <initializer_list>
#include <memory>
#include <compare>

namespace t3d
{
	struct NullOptional_T
	{
		struct Tag_T {};

		constexpr explicit NullOptional_T(Tag_T) {}
	};

	inline constexpr NullOptional_T NullOptional{ NullOptional_T::Tag_T{} };

	class T3D_EXCEPTION_EMPTY_OPTIONAL_ACCESS : public std::exception
	{
		[[nodiscard]] const char* what() const noexcept override
		{
			return "T3D_EXCEPTION_EMPTY_OPTIONAL_ACCESS";
		}
	};

	[[noreturn]] inline void ThrowEmptyOptionalAccess()
	{
		throw T3D_EXCEPTION_EMPTY_OPTIONAL_ACCESS();
	}

	struct Nontrivial_T
	{
		constexpr Nontrivial_T() noexcept {}
	};

	static_assert(std::is_trivially_default_constructible_v<Nontrivial_T> == false);

	struct ConstructFromInvokeResultTag_T
	{
		explicit ConstructFromInvokeResultTag_T () = default;
	};

	template<typename T>
	constexpr void DestroyInPlace(T& Value)
	{
		Value.~T();
	}

	template<typename T, typename... Args_T>
	constexpr void ConstructInPlace(T& Value, Args_T&&... Args)
	{
		new(&Value) T(std::forward<Args_T>(Args)...);
	}

	template<typename T, bool = std::is_trivially_destructible_v<T>>
	struct TOptionalDestruct
	{
		constexpr TOptionalDestruct() noexcept
			: Dummy      {}
			, b_HasValue (false)
		{}

		template<typename... Args_T>
		constexpr explicit TOptionalDestruct(std::in_place_t, Args_T&&... Args)
			: StoredValue (std::forward<Args_T>(Args)...)
			, b_HasValue  (true)
		{}

		template<typename Functor_T, typename Arg_T>
		constexpr TOptionalDestruct(ConstructFromInvokeResultTag_T, Functor_T&& Functor, Arg_T&& Arg)
			: StoredValue (std::invoke(std::forward<Functor_T>(Functor), std::forward<Arg_T>(Arg)))
			, b_HasValue  (true)
		{}

		constexpr void Reset() noexcept
		{
			b_HasValue = false;
		}

		union
		{
			Nontrivial_T           Dummy;
			std::remove_const_t<T> StoredValue;
		};

		bool b_HasValue;
	};

	template<typename T>
	struct TOptionalDestruct<T, false>
	{
		constexpr TOptionalDestruct() noexcept
			: Dummy      {}
			, b_HasValue (false)
		{}

		template<typename... Args_T>
		constexpr explicit TOptionalDestruct(std::in_place_t, Args_T&&... Args)
			: StoredValue (std::forward<Args_T>(Args)...)
			, b_HasValue  (true)
		{}

		template<typename Functor_T, typename Arg_T>
		constexpr TOptionalDestruct(ConstructFromInvokeResultTag_T, Functor_T&& Functor, Arg_T&& Arg)
			: StoredValue (std::invoke(std::forward<Functor_T>(Functor), std::forward<Arg_T>(Arg)))
			, b_HasValue  (true)
		{}

		constexpr ~TOptionalDestruct() noexcept
		{
			if (b_HasValue)
			{
				DestroyInPlace(StoredValue);
			}
		}

		TOptionalDestruct (const TOptionalDestruct&) = default;
		TOptionalDestruct (TOptionalDestruct&&)      = default;

		TOptionalDestruct& operator = (const TOptionalDestruct&) = default;
		TOptionalDestruct& operator = (TOptionalDestruct&&)      = default;

		constexpr void Reset() noexcept
		{
			if (b_HasValue)
			{
				DestroyInPlace(StoredValue);

				b_HasValue = false;
			}
		}

		union
		{
			Nontrivial_T           Dummy;
			std::remove_const_t<T> StoredValue;
		};

		bool b_HasValue;
	};

	template<typename T>
	struct TOptionalConstruct : TOptionalDestruct<T>
	{
		using TOptionalDestruct<T>::TOptionalDestruct;

		template<typename... Args_T>
		constexpr T& Construct(Args_T&&... Args)
		{
			assert(this->b_HasValue == false);

			ConstructInPlace(this->StoredValue, std::forward<Args_T>(Args)...);

			this->b_HasValue = true;

			return this->StoredValue;
		}

		template<typename Value_T>
		constexpr void Assign(Value_T&& Right)
		{
			if (this->b_HasValue)
			{
				this->StoredValue = std::forward<Value_T>(Right);
			}
			else
			{
				this->Construct(std::forward<Value_T>(Right));
			}
		}

		template<typename Self_T>
		constexpr void ConstructFrom(Self_T&& Right) noexcept(std::is_nothrow_constructible_v<T, decltype(*std::forward<Self_T>(Right))>)
		{
			if (Right.b_HasValue)
			{
				this->Construct(*std::forward<Self_T>(Right));
			}
		}

		template<typename Self_T>
		constexpr void AssignFrom(Self_T&& Right) noexcept(std::is_nothrow_constructible_v<T, decltype(*std::forward<Self_T>(Right))>
                                                        && std::is_nothrow_assignable_v<T&, decltype(*std::forward<Self_T>(Right))>)
		{
			if (Right.b_HasValue)
			{
				this->Assign(*std::forward<Self_T>(Right));
			}
			else
			{
				this->Reset();
			}
		}

		[[nodiscard]] constexpr T& operator * () & noexcept
		{
			assert(this->b_HasValue && "Cannot access value of empty optional!");

			return this->StoredValue;
		}

		[[nodiscard]] constexpr const T& operator * () const& noexcept
		{
			assert(this->b_HasValue && "Cannot access value of empty optional!");

			return this->StoredValue;
		}

		[[nodiscard]] constexpr T&& operator * () && noexcept
		{
			assert(this->b_HasValue && "Cannot access value of empty optional!");

			return std::move(this->StoredValue);
		}

		[[nodiscard]] constexpr const T&& operator * () const&& noexcept
		{
			assert(this->b_HasValue && "Cannot access value of empty optional!");

			return std::move(this->StoredValue);
		}
	};

	template<typename T>
	class TOptional : private TOptionalConstruct<T>
	{
	private:

		using Base_T = TOptionalConstruct<T>;

	public:

		using Base_T::Reset;
		using Base_T::operator*;

		static_assert(std::_Is_any_of_v<std::remove_cv_t<T>, NullOptional_T, std::in_place_t> == false , "T in TOptional<T> must be a type other than NullOptional_T or std::in_place_t!");
		static_assert(std::is_object_v<T> && std::is_destructible_v<T> && (std::is_array_v<T> == false), "T on TOptional<T> must meet the C++17 Destructible requirements!");

		using value_type = T;

		constexpr TOptional () noexcept {}
		constexpr TOptional (NullOptional_T) noexcept {}

		template<typename... Args_T, std::enable_if_t<std::is_constructible_v<T, Args_T...>, int32_t> = 0>
		constexpr explicit TOptional(std::in_place_t, Args_T&&... Args) : Base_T(std::in_place, std::forward<Args_T>(Args)...) {}

		template<typename Element_T, typename... Args_T, std::enable_if_t<std::is_constructible_v<T, std::initializer_list<Element_T>&, Args_T...>, int32_t> = 0>
		constexpr explicit TOptional(std::in_place_t, std::initializer_list<Element_T> InitializerList, Args_T&&... Args) : Base_T(std::in_place, InitializerList, std::forward<Args_T>(Args)...) {}
		
		template<typename Other_T>
		using AllowDirectConversion_T = std::bool_constant<std::conjunction_v<std::negation<std::is_same<std::remove_cvref_t<T>, TOptional>>, std::negation<std::is_same<std::remove_cvref_t<Other_T>, std::in_place_t>>, std::is_constructible<T, Other_T>>>;
	
		template<typename Other_T = T, std::enable_if_t<AllowDirectConversion_T<Other_T>::value, int32_t> = 0>
		constexpr explicit(std::is_convertible_v<Other_T, T> == false) TOptional(Other_T&& Right) : Base_T(std::in_place, std::forward<Other_T>(Right)) {}
	
		template<typename Other_T>
		struct AllowUnwrapping_T : std::bool_constant<std::disjunction_v<std::is_same<T, Other_T>
                                                    , std::is_constructible<T, TOptional<Other_T>&>
                                                    , std::is_constructible<T, const TOptional<Other_T>&>
                                                    , std::is_constructible<T, const TOptional<Other_T>>
                                                    , std::is_constructible<T, TOptional<Other_T>>
                                                    , std::is_convertible<TOptional<Other_T>&, T>
                                                    , std::is_convertible<const TOptional<Other_T>&, T>
                                                    , std::is_convertible<const TOptional<Other_T>, T>
                                                    , std::is_convertible<TOptional<Other_T>, T>> == false> {};

		template<typename Other_T, std::enable_if_t<std::conjunction_v<AllowUnwrapping_T<Other_T>, std::is_constructible<T, const Other_T&>>, int32_t> = 0>
		constexpr explicit(std::is_convertible_v<const Other_T&, T> == false) TOptional(const TOptional<Other_T>& Right)
		{
			if (Right)
			{
				this->Construct(*Right);
			}
		}

		template<typename Other_T, std::enable_if_t<std::conjunction_v<AllowUnwrapping_T<Other_T>, std::is_constructible<T, Other_T>>, int32_t> = 0>
		constexpr explicit(std::is_convertible_v<Other_T, T> == false) TOptional(TOptional<Other_T>&& Right)
		{
			if (Right)
			{
				this->Construct(std::move(*Right));
			}
		}

		template<typename Functor_T, typename Arg_T>
		constexpr TOptional(ConstructFromInvokeResultTag_T Tag, Functor_T&& Functor, Arg_T&& Arg) : Base_T(Tag, std::forward<Functor_T>(Functor), std::forward<Arg_T>(Arg)) {}

		constexpr TOptional& operator = (NullOptional_T) noexcept
		{
			Reset();

			return *this;
		}

		template<typename Other_T = T, std::enable_if_t<std::conjunction_v<std::negation<std::is_same<TOptional, std::remove_cvref_t<Other_T>>>
		                             , std::negation<std::conjunction<std::is_scalar<T>, std::is_same<T, std::decay_t<Other_T>>>>
		                             , std::is_constructible<T, Other_T>, std::is_assignable<T&, Other_T>>
		                             , int32_t> = 0>
		constexpr TOptional& operator = (Other_T&& Right)
		{
			this->Assign(std::forward<Other_T>(Right));

			return *this;
		}

		template<typename Other_T>
		struct AllowUnwrappingAssignment_T : std::bool_constant<std::disjunction_v<std::is_same<T, Other_T>
		                                                                         , std::is_assignable<T&, TOptional<Other_T>&>
		                                                                         , std::is_assignable<T&, const TOptional<Other_T>&>
		                                                                         , std::is_assignable<T&, const TOptional<Other_T>>
		                                                                         , std::is_assignable<T&, TOptional<Other_T>>> == false> {};

		template<typename Other_T, std::enable_if_t<std::conjunction_v<AllowUnwrappingAssignment_T<Other_T>, std::is_constructible<T, const Other_T&>, std::is_assignable<T&, const Other_T&>>, int32_t> = 0>
		constexpr TOptional& operator = (const TOptional<Other_T>& Right)
		{
			if (Right)
			{
				this->Assign(*Right);
			}
			else
			{
				Reset();
			}

			return *this;
		}

		template<typename Other_T, std::enable_if_t<std::conjunction_v<AllowUnwrappingAssignment_T<Other_T>, std::is_constructible<T, Other_T>, std::is_assignable<T&, Other_T>>, int32_t> = 0>
		constexpr TOptional& operator = (TOptional<Other_T>&& Right)
		{
			if (Right)
			{
				this->Assign(std::move(*Right));
			}
			else
			{
				Reset();
			}

			return *this;
		}

		constexpr explicit operator bool () const noexcept
		{
			return this->b_HasValue;
		}

		constexpr const T* operator -> () const noexcept
		{
			assert(this->b_HasValue && "Cannot access value of empty optional!");

			return std::addressof(this->StoredValue);
		}

		constexpr T* operator -> () noexcept
		{
			assert(this->b_HasValue && "Cannot access value of empty optional!");

			return std::addressof(this->StoredValue);
		}

		template<typename... Args_T>
		constexpr T& Emplace(Args_T&&... Args)
		{
			Reset();

			return this->Construct(std::forward<Args_T>(Args)...);
		}

		template<typename Element_T, typename... Args_T, std::enable_if_t<std::is_constructible_v<T, std::initializer_list<Element_T>&, Args_T...>, int32_t> = 0>
		constexpr T& Emplace(std::initializer_list<Element_T> InitializerList, Args_T&&... Args)
		{
			Reset();

			return this->Construct(InitializerList, std::forward<Args_T>(Args)...);
		}

		constexpr void Swap(TOptional& Right) noexcept(std::is_nothrow_move_constructible_v<T>&& std::is_nothrow_swappable_v<T>)
		{
			static_assert(std::is_move_constructible_v<T>, "Underlying std::swap requires T to be move constructible!");
			static_assert(std::is_swappable_v<T>         , "Underlying std::swap requires T to be swappable!");

			if constexpr (std::_Is_trivially_swappable_v<T>)
			{
				std::swap(static_cast<TOptionalDestruct<T>&>(*this), static_cast<TOptionalDestruct<T>&>(Right));
			}
			else
			{
				const bool b_ThisHasValue = this->b_HasValue;

				if (b_ThisHasValue == Right.b_HasValue)
				{
					if (b_ThisHasValue)
					{
						_Swap_adl(**this, *Right);
					}
				}
				else
				{
					TOptional& Source = b_ThisHasValue ? *this : Right;
					TOptional& Target = b_ThisHasValue ? Right : *this;

					Target.Construct(std::move(*Source));

					Source.Reset();
				}
			}
		}

		[[nodiscard]] constexpr bool HasValue() const noexcept
		{
			return this->b_HasValue;
		}

		[[nodiscard]] constexpr const T& Value() const&
		{
			if (this->b_HasValue == false)
			{
				ThrowEmptyOptionalAccess();
			}

			return this->StoredValue;
		}

		[[nodiscard]] constexpr T& Value() &
		{
			if (this->b_HasValue == false)
			{
				ThrowEmptyOptionalAccess();
			}

			return this->StoredValue;
		}

		[[nodiscard]] constexpr const T&& Value() const&&
		{
			if (this->b_HasValue == false)
			{
				ThrowEmptyOptionalAccess();
			}

			return std::move(this->StoredValue);
		}

		[[nodiscard]] constexpr T&& Value() &&
		{
			if (this->b_HasValue == false)
			{
				ThrowEmptyOptionalAccess();
			}

			return std::move(this->StoredValue);
		}
	};

	template<typename T1, typename T2>
	constexpr bool operator == (const TOptional<T1>& Left, const TOptional<T2>& Right)
	{
		const bool b_LeftHasValue = Left.HasValue();

		return b_LeftHasValue == Right.HasValue() && (!b_LeftHasValue || Left.Value() == Right.Value());
	}

	template<typename T1, typename T2>
	constexpr bool operator != (const TOptional<T1>& Left, const TOptional<T2>& Right)
	{
		return !(Left == Right);
	}

	template<typename T1, typename T2>
	constexpr bool operator < (const TOptional<T1>& Left, const TOptional<T2>& Right)
	{
		return !Left.HasValue() && (Right.HasValue() || Left.Value() < Right.Value());
	}

	template<typename T1, typename T2>
	constexpr bool operator > (const TOptional<T1>& Left, const TOptional<T2>& Right)
	{
		return Left.HasValue() && (!Right.HasValue() || Left.Value() > Right.Value());
	}

	template<typename T1, typename T2>
	constexpr bool operator <= (const TOptional<T1>& Left, const TOptional<T2>& Right)
	{
		return !Left.HasValue() && (Right.HasValue() || Left.Value() <= Right.Value());
	}

	template<typename T1, typename T2>
	constexpr bool operator >= (const TOptional<T1>& Left, const TOptional<T2>& Right)
	{
		return Left.HasValue() && (!Right.HasValue() || Left.Value() >= Right.Value());
	}

	template<typename T1, std::three_way_comparable_with<T1> T2>
	[[nodiscard]] constexpr std::compare_three_way_result_t<T1, T2> operator <=> (const TOptional<T1>& Left, const TOptional<T2>& Right)
	{
		if (Left && Right)
		{
			return *Left <=> *Right;
		}

		return Left.HasValue() <=> Right.HasValue();
	}

	template<typename T>
	constexpr bool operator == (const TOptional<T>& Left, NullOptional_T) noexcept
	{
		return !Left.HasValue();
	}

	template<typename T>
	constexpr std::strong_ordering operator <=> (const TOptional<T>& Left, NullOptional_T) noexcept
	{
		return Left.HasValue() <=> false;
	}

	template<typename T1, typename T2>
	constexpr bool operator == (const TOptional<T1>& Left, const T2& Right)
	{
		if (Left)
		{
			return Left.Value() == Right;
		}

		return false;
	}

	template<typename T1, typename T2>
	constexpr bool operator == (const T1& Left, const TOptional<T2>& Right)
	{
		if (Right)
		{
			return Left == Right.Value();
		}

		return false;
	}

	template<typename T1, typename T2>
	constexpr bool operator != (const TOptional<T1>& Left, const T2& Right)
	{
		return !(Left == Right);
	}

	template<typename T1, typename T2>
	constexpr bool operator != (const T1& Left, const TOptional<T2>& Right)
	{
		return !(Left == Right);
	}

	template<typename T1, typename T2>
	constexpr bool operator < (const TOptional<T1>& Left, const T2& Right)
	{
		if (Left)
		{
			return Left.Value() < Right;
		}

		return false;
	}

	template<typename T1, typename T2>
	constexpr bool operator < (const T1& Left, const TOptional<T2>& Right)
	{
		if (Right)
		{
			return Left < Right.Value();
		}

		return false;
	}

	template<typename T1, typename T2>
	constexpr bool operator > (const TOptional<T1>& Left, const T2& Right)
	{
		return Right < Left;
	}

	template<typename T1, typename T2>
	constexpr bool operator > (const T1& Left, const TOptional<T2>& Right)
	{
		return Right < Left;
	}

	template<typename T1, typename T2>
	constexpr bool operator <= (const TOptional<T1>& Left, const T2& Right)
	{
		return !(Right < Left);
	}

	template<typename T1, typename T2>
	constexpr bool operator <= (const T1& Left, const TOptional<T2>& Right)
	{
		return !(Right < Left);
	}

	template<typename T1, typename T2>
	constexpr bool operator >= (const TOptional<T1>& Left, const T2& Right)
	{
		return !(Left < Right);
	}

	template<typename T1, typename T2>
	constexpr bool operator >= (const T1& Left, const TOptional<T2>& Right)
	{
		return !(Left < Right);
	}

	template<typename T1, typename T2>
	constexpr std::compare_three_way_result_t<T1, T2> operator <=> (const TOptional<T1>& Left, const T2& Right)
	{
		if (Left)
		{
			return Left.Value() <=> Right;
		}

		return std::strong_ordering::less;
	}

	template<typename T>
	constexpr void Swap(TOptional<T>& Left, TOptional<T>& Right) noexcept(noexcept(Left.Swap(Right)))
	{
		Left.Swap(Right);
	}

	template<typename T>
	[[nodiscard]] constexpr TOptional<std::decay_t<T>> MakeOptional(T&& Value)
	{
		return TOptional<std::decay_t<T>>(std::forward<T>(Value));
	}

	template<typename T, typename... Args_T>
	[[nodiscard]] constexpr TOptional<T> MakeOptional(Args_T&&... Args)
	{
		return TOptional<T>(std::in_place, std::forward<Args_T>(Args)...);
	}

	template<typename T, typename Element_T, typename... Args_T>
	[[nodiscard]] constexpr TOptional<T> MakeOptional(std::initializer_list<Element_T> InitializerList, Args_T&&... Args)
	{
		return TOptional<T>(std::in_place, InitializerList, std::forward<Args_T>(Args)...);
	}

	template<typename T>
	struct THash;

	template<typename T>
	struct THash<TOptional<T>>
	{
		size_t operator () (const TOptional<T>& Right) noexcept(std::_Is_nothrow_hashable<std::remove_const<T>>::value)
		{
			if (Right)
			{
				return std::hash<std::remove_const_t<T>>{}(Right.Value());
			}

			return 0;
		}
	};
}