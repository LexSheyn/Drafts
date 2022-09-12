#pragma once

#include <memory>
#include <type_traits>
#include <typeinfo>
#include <atomic>
#include <exception>
#include <intrin.h>
#include <cassert>

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

	class T3D_EXCEPTION_BAD_WEAK_POINTER : public std::exception
	{
	public:

		T3D_EXCEPTION_BAD_WEAK_POINTER () noexcept {}

		T3D_NO_DISCARD const char* what() const noexcept override
		{
			return "T3D_EXCEPTION_BAD_WEAK_POINTER";
		}
	};

	T3D_NO_RETURN T3D_INLINE void Throw_Bad_Weak_Pointer()
	{
		throw T3D_EXCEPTION_BAD_WEAK_POINTER();
	}

	class T3D_NO_VTABLE TReferenceCounterBase
	{
	private:

		virtual void Destroy    () noexcept = 0; // Destroy managed resource.
		virtual void Delete_This () noexcept = 0; // Destroy self.

		std::_Atomic_counter_t Uses  = 1;
		std::_Atomic_counter_t Weaks = 1;

	protected:

		constexpr TReferenceCounterBase () noexcept = default; // Non-atomic initializations.

	public:

		TReferenceCounterBase(const TReferenceCounterBase&) = delete;

		TReferenceCounterBase& operator = (const TReferenceCounterBase&) = delete;

		virtual ~TReferenceCounterBase () noexcept {} // MSVC: Transition, should be non-virtual.

		bool8 Increment_Reference_Count_If_Not_Zero () noexcept // Increment use count if not zero, return true if successful.
		{
			auto& VolatileUses = reinterpret_cast<volatile long&>(Uses);

			long Count = __iso_volatile_load32(reinterpret_cast<volatile int32*>(&VolatileUses));

			while (Count != 0)
			{
				const int32 OldValue = _INTRIN_RELAXED(_InterlockedCompareExchange)(&VolatileUses, Count + 1, Count);

				if (OldValue == Count)
				{
					return true;
				}

				Count = OldValue;
			}

			return false;
		}

		void Increment_Reference_Count() noexcept
		{
			_MT_INCR(Uses);
		}

		void Increment_Weak_Reference_Count() noexcept
		{
			_MT_INCR(Weaks);
		}

		void Decrement_Reference_Count() noexcept
		{
			if (_MT_DECR(Uses) == 0)
			{
				this->Destroy();
				this->Decrement_Weak_Reference_Count();
			}
		}

		void Decrement_Weak_Reference_Count() noexcept
		{
			if (_MT_DECR(Weaks) == 0)
			{
				this->Delete_This();
			}
		}

		long UseCount() const noexcept
		{
			return static_cast<long>(Uses);
		}

		virtual void* GetDeleter(const std::type_info&) const noexcept
		{
			return nullptr;
		}
	};

	template<typename T>
	class TReferenceCounter : public TReferenceCounterBase
	{
	public:

		explicit TReferenceCounter(T* Pointer)
			: TReferenceCounterBase ()
			, Pointer               (Pointer)
		{}

	private:

		void Destroy() noexcept override
		{
			delete Pointer;
		};

		void Delete_This() noexcept override
		{
			delete this;
		};

		T* Pointer;
	};

	template<typename Resource_T, typename Deleter_T>
	class TReferenceCounterResource : public TReferenceCounterBase // Handle reference counting for object with deleter.
	{
	public:

		TReferenceCounterResource(Resource_T Resource, Deleter_T Deleter)
			: TReferenceCounterBase ()
			, DeleterResourcePair   (std::_One_then_variadic_args_t{}, std::move(Deleter), Resource)
		{}

		~TReferenceCounterResource () noexcept override {}

		void* GetDeleter(const std::type_info& TypeId) const noexcept override
		{
			if (TypeId == typeid(Deleter_T))
			{
				return const_cast<Deleter_T*>(std::addressof(DeleterResourcePair._Get_first()));
			}

			return nullptr;
		}

	private:

		void Destroy() noexcept override
		{
			DeleterResourcePair._Get_first()(DeleterResourcePair._Myval2);
		}

		void Delete_This() noexcept override
		{
			delete this;
		}

		std::_Compressed_pair<Deleter_T, Resource_T> DeleterResourcePair;
	};

	template<typename Resource_T, typename Deleter_T, typename Allocator_T>
	class TReferenceCounterResourceAllocated : public TReferenceCounterBase // Handle reference counting for object with deleter and allocator.
	{
	public:

		TReferenceCounterResourceAllocated(Resource_T Resource, Deleter_T Deleter, const Allocator_T& Allocator)
			: TReferenceCounterBase()
			, DeleterResourcePair(std::_One_then_variadic_args_t{}, std::move(Deleter), std::_One_then_variadic_args_t{}, Allocator, Deleter)
		{}

		~TReferenceCounterResourceAllocated() noexcept override {}

		void* GetDeleter(const std::type_info& TypeId) const noexcept override
		{
			if (TypeId == typeid(Deleter_T))
			{
				return const_cast<Deleter_T*>(std::addressof(DeleterResourcePair._Get_first()));
			}

			return nullptr;
		}

	private:

		using ReboundAllocator_T = std::_Rebind_alloc_t<Allocator_T, TReferenceCounterResourceAllocated>;

		void Destroy() noexcept override
		{
			DeleterResourcePair._Get_first()(DeleterResourcePair._Myval2._Myval2);
		}

		void Delete_This() noexcept override
		{
			ReboundAllocator_T Allocator = DeleterResourcePair._Myval2._Get_first();

			this->~TReferenceCounterBase();

			std::_Deallocate_plain(Allocator, this);
		}

		std::_Compressed_pair<Deleter_T, std::_Compressed_pair<ReboundAllocator_T, Resource_T>> DeleterResourcePair;
	};

	template<typename T>
	struct FDefaultDeleter;

	template<typename T, typename Deleter_T = FDefaultDeleter<T>>
	class TUniquePointer;

	template<typename T>
	class TSharedPointer;

	template<typename T>
	class FWeakPointer;

	template<typename T, typename = void>
	struct Can_Enable_Shared_T : std::false_type {}; // Detect unambiguous and accessible inheritance from Enable_Shared_From_This.

	template<typename T>
	struct Can_Enable_Shared_T<T, std::void_t<typename T::_Esft_type>> : std::is_convertible<std::remove_cv_t<T>*, typename T::_Esft_type*>::type
	{};

	struct T3D_EXCEPTION_POINTER_ACCESS;

	template<typename T>
	class TPointerBase // Base class for TSharedPointer and TWeakPointer.
	{
	public:

		using element_type = std::remove_extent_t<T>;

		T3D_NO_DISCARD long UseCount() const noexcept
		{
			if (ReferenceCounter)
			{
				return ReferenceCounter->UseCount();
			}

			return 0;
		}

		template<typename Other_T>
		T3D_NO_DISCARD bool8 OwnerBefore(const TPointerBase<Other_T>& Right) const noexcept // Compare addresses of managed objects.
		{
			return ReferenceCounter < Right.ReferenceCounter;
		}

		TPointerBase(const TPointerBase&) = delete;

		TPointerBase& operator = (const TPointerBase&) = delete;

	protected:

		T3D_NO_DISCARD element_type* Get() const noexcept
		{
			return Pointer;
		}

		constexpr TPointerBase () noexcept = default;
		         ~TPointerBase ()          = default;

		template<typename Other_T>
		void Move_Construct_From(TPointerBase<Other_T>&& Right) noexcept
		{
			Pointer          = Right.Pointer;
			ReferenceCounter = Right.ReferenceCounter;

			Right.Pointer          = nullptr;
			Right.ReferenceCounter = nullptr;
		}

		template<typename Other_T>
		void Copy_Construct_From(const TSharedPointer<Other_T>& Right) noexcept
		{
			Right.Increment_Reference_Count();

			Pointer          = Right.Pointer;
			ReferenceCounter = Right.ReferenceCounter;
		}

		template<typename Other_T>
		void Alias_Construct_From(const TSharedPointer<Other_T>& Right, element_type* Pointer) noexcept
		{
			Right.Increment_Reference_Count();

			this->Pointer    = Pointer;
			ReferenceCounter = Right.ReferenceCounter;
		}

		template<typename Other_T>
		void Alias_Move_Construct_From(TSharedPointer<Other_T>&& Right, element_type* Pointer) noexcept
		{
			this->Pointer    = Pointer;
			ReferenceCounter = Right.ReferenceCounter;

			Right.Pointer          = nullptr;
			Right.ReferenceCounter = nullptr
		}

		template<typename Other_T>
		friend class TWeakPointer; // Specifically TWeakPointer::Lock().

		template<typename Other_T>
		bool8 Construct_From_Weak(const TWeakPointer<Other_T>& Right) noexcept
		{
			if (Right.ReferenceCounter && Right.ReferenceCounter->Increment_Reference_Count_If_Not_Zero())
			{
				Pointer          = Right.Pointer;
				ReferenceCounter = Right.ReferenceCounter;

				return true;
			}

			return false;
		}

		void Increment_Reference_Count() const noexcept
		{
			if (ReferenceCounter)
			{
				ReferenceCounter->Increment_Reference_Count();
			}
		}

		void Decrement_Reference_Count() noexcept
		{
			if (ReferenceCounter)
			{
				ReferenceCounter->Decrement_Reference_Count();
			}
		}

		void Swap(TPointerBase& Right) noexcept
		{
			std::swap(Pointer         , Right.Pointer);
			std::swap(ReferenceCounter, Right.ReferenceCounter);
		}

		template<typename Other_T>
		void Weakly_Construct_From(const TPointerBase<Other_T>& Right) noexcept
		{
			if (Right.ReferenceCounter)
			{
				Pointer          = Right.Pointer;
				ReferenceCounter = Right.ReferenceCounter;

				ReferenceCounter->Increment_Weak_Reference_Count();
			}
			else
			{
				T3D_ASSERT(!Ponter && !ReferenceCounter);
			}
		}

		template<typename Other_T>
		void Weakly_Convert_lvalue_Avoiding_Expired_Conversions(const TPointerBase<Other_T>& Right) noexcept
		{
			if (Right.ReferenceCounter)
			{
				ReferenceCounter = Right.ReferenceCounter; // Always share ownership.

				ReferenceCounter->Increment_Weak_Reference_Count();

				if (ReferenceCounter->Increment_Reference_Count_If_Not_Zero())
				{
					Pointer = Right.Pointer; // Keep resource alive during conversion, handling virtual inheritance.

					ReferenceCounter->Decrement_Reference_Count();
				}
				else
				{
					T3D_ASSERT(!Pointer);
				}
			}
			else
			{
				T3D_ASSERT(!Pointer && !ReferenceCounter);
			}
		}

		template<typename Other_T>
		void Wearly_Convert_rvalue_Avoiding_Expired_Conversions(TPointerBase<Other_T>&& Right) noexcept
		{
			ReferenceCounter = Right.ReferenceCounter;

			Right.ReferenceCounter = nullptr;

			if (ReferenceCounter && ReferenceCounter->Increment_Reference_Count_If_Not_Zero())
			{
				Pointer = Right.Pointer;

				ReferenceCounter->Decrement_Reference_Count();
			}
			else
			{
				T3D_ASSERT(!Pointer);
			}

			Right.Pointer = nullptr;
		}

		void Increment_Weak_Reference_Count() const noexcept
		{
			if (ReferenceCounter)
			{
				ReferenceCounter->Increment_Weak_Reference_Count();
			}
		}

		void Decrement_Weak_Reference_Count() noexcept
		{
			if (ReferenceCounter)
			{
				ReferenceCounter->Decrement_Weak_Reference_Count();
			}
		}

	private:

		element_type*          Pointer          { nullptr };
		TReferenceCounterBase* ReferenceCounter { nullptr };

		template<typename Other_T>
		friend class TPointerBase;

		friend TSharedPointer<T>;

		template<typename Other_T>
		friend struct std::atomic;

		friend T3D_EXCEPTION_POINTER_ACCESS;

		template<typename Deleter_T, typename Other_T>
		friend Deleter_T* GetDeleter(const TSharedPointer<Other_T>& SharedPointer) noexcept;
	};

	// <memory> Line: 1420



















































































	// Base class for TSharedPointer and TWeakPointer
	template<typename T>
	class TPointer
	{
	public:

		using element_type = std::remove_reference_t<T>;

	//	[[nodiscard]]

	protected:

		//

	private:

		element_type* Pointer { nullptr };

	};
}