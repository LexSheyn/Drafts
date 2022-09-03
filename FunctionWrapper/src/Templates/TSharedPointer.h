#pragma once

#include "Core/TrickAPI.h"
#include "Core/TrickTypes.h"

#include <type_traits>
#include <atomic>
#include <utility>

namespace t3d
{
	template<typename T>
	class TSharedPointer
	{
	public:

	// Constructors and Destructor:

		constexpr TSharedPointer() noexcept
			: Object  (nullptr)
			, Counter (nullptr)
		{}

		constexpr TSharedPointer(std::nullptr_t) noexcept
			: Object  (nullptr)
			, Counter (nullptr)
		{}

		template<typename Other_T>
		TSharedPointer(Other_T* Pointer)
			: Object  (Pointer)
			, Counter (new std::atomic<uint32>(1))
		{}

		template<typename Other_T>
		TSharedPointer(const TSharedPointer<Other_T>& Right)
			: Object  (Right.Object)
			, Counter (Right.Counter)
		{
			this->IncrementCounter();
		}

		template<typename Other_T>
		TSharedPointer(TSharedPointer<Other_T>&& Right) noexcept
			: Object  (std::move(Right.Object))
			, Counter (std::move(Right.Counter))
		{
			Right.TryDelete();

			Right.Object  = nullptr;
			Right.Counter = nullptr;
		}

		~TSharedPointer()
		{
			this->TryDelete();
		}

	// Functions:

		void Reset()
		{
			this->TryDelete();
		}

		template<typename Other_T>
		void Reset(Other_T* Pointer)
		{
			this->TryDelete();

			Object  = Pointer;
			Counter = new std::atomic<uint32>(1);
		}

		void Swap(TSharedPointer& Right) noexcept
		{
			std::swap(Object , Right.Object);
			std::swap(Counter, Right.Counter);
		}

	// Accessors:

		T* Get() const noexcept
		{
			return Object;
		}

		uint32 Count() const noexcept
		{
			if (Counter == nullptr)
			{
				return 0;
			}

			return (*Counter).load();
		}

	// Operators:

		template<typename Other_T>
		TSharedPointer& operator = (const TSharedPointer<Other_T>& Right) noexcept
		{
			this->TryDelete();

			Object  = Right.Object;
			Counter = Right.Counter;

			this->IncrementCounter();

			return *this;
		}

		template<typename Other_T>
		TSharedPointer& operator = (TSharedPointer<Other_T>&& Right) noexcept
		{
			this->TryDelete();

			Object  = std::move(Right.Object);
			Counter = std::move(Right.Counter);

			Right.TryDelete();

			Right.Object  = nullptr;
			Right.Counter = nullptr;

			return *this;
		}

		explicit operator bool() const noexcept
		{
			return Object != nullptr;
		}

		T& operator * () const
		{
			return *Object;
		}

		T* operator -> () const noexcept
		{
			return Object;
		}

	private:

	// Private Functions:

		void TryDelete() noexcept
		{
			if (Counter && ((*Counter).load() > 0))
			{
				this->DecrementCounter();

				Object  = nullptr;
				Counter = nullptr;
			}
			else if (Object)
			{
				delete Object;
				delete Counter;
			}
		}

		void IncrementCounter() noexcept
		{
			++(*Counter);
		}

		void DecrementCounter() noexcept
		{
			--(*Counter);
		}

	// Variables:

		T*                   Object;
		std::atomic<uint32>* Counter;
	};

	template<typename T, typename... Args_T>
	constexpr TSharedPointer<T> T3D_CALL MakeShared(Args_T&&... Args)
	{
		return TSharedPointer<T>(new T(std::forward<Args_T>(Args)...));
	}

// Operators:

	template<typename Right_T, typename Left_T>
	constexpr bool8 operator == (const TSharedPointer<Right_T>& Left, const TSharedPointer<Left_T>& Right) noexcept
	{
		return Left.Get() == Right.Get();
	}

	template<typename Right_T, typename Left_T>
	constexpr bool8 operator != (const TSharedPointer<Right_T>& Left, const TSharedPointer<Left_T>& Right) noexcept
	{
		return !(Left.Get() == Right.Get());
	}

	template<typename T>
	constexpr bool8 operator == (const TSharedPointer<T>& Left, std::nullptr_t Right) noexcept
	{
		return !Left;
	}

	template<typename T>
	constexpr bool8 operator == (std::nullptr_t Left, const TSharedPointer<T>& Right) noexcept
	{
		return !Right;
	}

	template<typename T>
	constexpr bool8 operator != (const TSharedPointer<T>& Left, std::nullptr_t Right) noexcept
	{
		return Left;
	}

	template<typename T>
	constexpr bool8 operator != (std::nullptr_t Left, const TSharedPointer<T>& Right) noexcept
	{
		return Right;
	}
}