#pragma once

#include <utility>
#include <exception>

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

	struct FNontrivialType
	{
		constexpr FNontrivialType() noexcept {}
	};

	template<typename T>
	class TOptional
	{
	public:

		constexpr TOptional() noexcept
			: b_HasValue (false)
		{}

		constexpr TOptional(NullOptional_T) noexcept
			: b_HasValue (false)
		{}

		template<typename... Args_T>
		constexpr explicit TOptional(Args_T&&... Args) noexcept
			: StoredValue (std::forward<Args_T>(Args)...)
			, b_HasValue  (true)
		{}

		template<typename Other_T>
		constexpr explicit TOptional(Other_T&& Right) noexcept
			: StoredValue (std::forward<Other_T>(Right))
			, b_HasValue  (true)
		{}

		template<typename Other_T>
		constexpr explicit TOptional(const TOptional& Right) noexcept
		{
			if (Right)
			{
				StoredValue = Right.StoredValue;
				b_HasValue  = true;
			}
		}

		template<typename Other_T>
		constexpr explicit TOptional(TOptional<Other_T>&& Right) noexcept
		{
			if (Right)
			{
				StoredValue = std::move(Right.StoredValue);
				b_HasValue  = true;
			}

			Right.Reset();
		}

		constexpr void Reset() noexcept
		{
			b_HasValue = false;
		}

		constexpr void Swap(TOptional& Right) noexcept
		{
			std::swap(StoredValue, Right.StoredValue);
			std::swap(b_HasValue , Right.b_HasValue);
		}

		constexpr bool HasValue() const noexcept
		{
			return b_HasValue;
		}

		constexpr const T& Value() const noexcept
		{
			if (b_HasValue == false)
			{
				throw T3D_EXCEPTION_EMPTY_OPTIONAL_ACCESS();
			}

			return StoredValue;
		}

		constexpr T& Value() noexcept
		{
			if (b_HasValue == false)
			{
				throw T3D_EXCEPTION_EMPTY_OPTIONAL_ACCESS();
			}

			return StoredValue;
		}

		constexpr const T&& Value() const noexcept
		{
			if (b_HasValue == false)
			{
				throw T3D_EXCEPTION_EMPTY_OPTIONAL_ACCESS();
			}

			return std::move(StoredValue);
		}

		constexpr T&& Value() noexcept
		{
			if (b_HasValue == false)
			{
				throw T3D_EXCEPTION_EMPTY_OPTIONAL_ACCESS();
			}

			return std::move(StoredValue);
		}

		constexpr explicit operator bool () const noexcept
		{
			return b_HasValue;
		}

		constexpr TOptional& operator = (NullOptional_T) noexcept
		{
			this->Reset();

			return *this;
		}

		template<typename Other_T>
		constexpr TOptional& operator = (Other_T&& Right) noexcept
		{
			this->Assign(Right);

			return *this;
		}

		template<typename Other_T>
		constexpr TOptional& operator = (const TOptional<Other_T>& Right) noexcept
		{
			if (Right)
			{
				this->Assign(Right);
			}
			else
			{
				this->Reset();
			}

			return *this;
		}

		template<typename Other_T>
		constexpr TOptional& operator = (TOptional<Other_T>&& Right) noexcept
		{
			if (Right)
			{
				this->Assign(Right);
			}
			else
			{
				this->Reset();
			}

			return *this;
		}

	private:

		template<typename Other_T>
		void Assign(Other_T&& Right)
		{
			StoredValue = std::forward<Other_T>(Right);

			b_HasValue = true;
		}

		template<typename Other_T>
		void Assign(const TOptional<Other_T>& Right)
		{
			StoredValue = std::forward<Other_T>(Right.StoredValue);

			b_HasValue = true;
		}

		template<typename Other_T>
		void Assign(TOptional<Other_T>&& Right)
		{
			StoredValue = std::forward<Other_T>(Right.StoredValue);

			b_HasValue = true;

			Right.Reset();
		}

		T StoredValue;

		bool b_HasValue;
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
		return TOptional<T>(std::forward<Args_T>(Args)...);
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