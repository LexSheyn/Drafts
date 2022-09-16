#pragma once

#include "TrickUtility.h"

#include <memory>
#include <type_traits>
#include <typeinfo>
#include <atomic>
#include <exception>
#include <intrin.h>
#include <cassert>

namespace t3d
{
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
	class TReferenceCounterResourceAllocator : public TReferenceCounterBase // Handle reference counting for object with deleter and allocator.
	{
	public:

		TReferenceCounterResourceAllocator(Resource_T Resource, Deleter_T Deleter, const Allocator_T& Allocator)
			: TReferenceCounterBase()
			, DeleterResourcePair(std::_One_then_variadic_args_t{}, std::move(Deleter), std::_One_then_variadic_args_t{}, Allocator, Deleter)
		{}

		~TReferenceCounterResourceAllocator() noexcept override {}

		void* GetDeleter(const std::type_info& TypeId) const noexcept override
		{
			if (TypeId == typeid(Deleter_T))
			{
				return const_cast<Deleter_T*>(std::addressof(DeleterResourcePair._Get_first()));
			}

			return nullptr;
		}

	private:

		using ReboundAllocator_T = std::_Rebind_alloc_t<Allocator_T, TReferenceCounterResourceAllocator>;

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
	struct TDefaultDeleter;

	template<typename T, typename Deleter_T = TDefaultDeleter<T>>
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

		template<typename X>
		T3D_NO_DISCARD bool8 OwnerBefore(const TPointerBase<X>& Right) const noexcept // Compare addresses of managed objects.
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

		template<typename X>
		void Move_Construct_From(TPointerBase<X>&& Right) noexcept
		{
			Pointer          = Right.Pointer;
			ReferenceCounter = Right.ReferenceCounter;

			Right.Pointer          = nullptr;
			Right.ReferenceCounter = nullptr;
		}

		template<typename X>
		void Copy_Construct_From(const TSharedPointer<X>& Right) noexcept
		{
			Right.Increment_Reference_Count();

			Pointer          = Right.Pointer;
			ReferenceCounter = Right.ReferenceCounter;
		}

		template<typename X>
		void Alias_Construct_From(const TSharedPointer<X>& Right, element_type* Pointer) noexcept
		{
			Right.Increment_Reference_Count();

			this->Pointer    = Pointer;
			ReferenceCounter = Right.ReferenceCounter;
		}

		template<typename X>
		void Alias_Move_Construct_From(TSharedPointer<X>&& Right, element_type* Pointer) noexcept
		{
			this->Pointer    = Pointer;
			ReferenceCounter = Right.ReferenceCounter;

			Right.Pointer          = nullptr;
			Right.ReferenceCounter = nullptr
		}

		template<typename X>
		friend class TWeakPointer; // Specifically TWeakPointer::Lock().

		template<typename X>
		bool8 Construct_From_Weak(const TWeakPointer<X>& Right) noexcept
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

		template<typename X>
		void Weakly_Construct_From(const TPointerBase<X>& Right) noexcept
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

		template<typename X>
		void Weakly_Convert_lvalue_Avoiding_Expired_Conversions(const TPointerBase<X>& Right) noexcept
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

		template<typename X>
		void Wearly_Convert_rvalue_Avoiding_Expired_Conversions(TPointerBase<X>&& Right) noexcept
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

		template<typename X>
		friend class TPointerBase;

		friend TSharedPointer<T>;

		template<typename X>
		friend struct std::atomic;

		friend T3D_EXCEPTION_POINTER_ACCESS;

		template<typename Deleter_T, typename X>
		friend Deleter_T* GetDeleter(const TSharedPointer<X>& SharedPointer) noexcept;
	};

	template<typename T, typename = void>
	struct Can_Scalar_Delete_T : std::false_type {};
	template<typename T>
	struct Can_Scalar_Delete_T<T, std::void_t<decltype(delete std::declval<T*>())>> : std::true_type {};

	template<typename T, typename = void>
	struct Can_Array_Delete_T : std::false_type {};
	template<typename T>
	struct Can_Array_Delete_T<T, std::void_t<decltype(delete[] std::declval<T*>())>> : std::true_type {};

	template<typename Function_T, typename Arg_T, typename = void>
	struct Can_Call_Function_Object_T : std::false_type {};
	template<typename Function_T, typename Arg_T>
	struct Can_Call_Function_Object_T<Function_T, Arg_T, std::void_t<decltype(std::declval<Function_T>()(std::declval<Arg_T>()))>> : std::true_type {};

	template<typename T, typename X>
	struct SP_Convertible_T : std::is_convertible<T*, X*>::type {};
	template<typename T, typename X>
	struct SP_Convertible_T<T, X[]> : std::is_convertible<T(*)[], X(*)[]>::type {};
	template<typename T, typename X, Size_T Size>
	struct SP_Convertible_T<T, X[Size]> : std::is_convertible<T(*)[Size], X(*)[Size]>::type {};

	template<typename T, typename X>
	struct SP_Pointer_Compatible_T : std::is_convertible<T*, X*>::type {};

	template<typename T, Size_T Size>
	struct SP_Pointer_Compatible_T<T[Size], T[]> : std::true_type {};
	template<typename T, Size_T Size>
	struct SP_Pointer_Compatible_T<T[Size], const T[]> : std::true_type {};
	template<typename T, Size_T Size>
	struct SP_Pointer_Compatible_T<T[Size], volatile T[]> : std::true_type {};
	template<typename T, Size_T Size>
	struct SP_Pointer_Compatible_T<T[Size], const volatile T[]> : std::true_type {};

	template<typename T>
	struct TemporaryOwner_T
	{
		explicit TemporaryOwner_T(T* const Pointer) noexcept
			: Pointer (Pointer)
		{}

		~TemporaryOwner_T()
		{
			delete Pointer;
		}

		TemporaryOwner_T(const TemporaryOwner_T&) = delete;

		TemporaryOwner_T& operator = (const TemporaryOwner_T&) = delete;

		T* Pointer;
	};

	template<typename Pointer_T, typename Deleter_T>
	struct TemporaryOwnerWithDeleter_T
	{
		explicit TemporaryOwnerWithDeleter_T(const Pointer_T Pointer, Deleter_T& Deleter) noexcept
			: Pointer (Pointer)
			, Deleter (Deleter)
		{}

		~TemporaryOwnerWithDeleter_T()
		{
			if (b_CallDeleter)
			{
				Deleter(Pointer);
			}
		}

		TemporaryOwnerWithDeleter_T(const TemporaryOwnerWithDeleter_T&) = delete;

		TemporaryOwnerWithDeleter_T& operator = (const TemporaryOwnerWithDeleter_T&) = delete;

		Pointer_T  Pointer;
		Deleter_T& Deleter;
		bool8      b_CallDeleter = true;
	};

	template<typename T>
	class TSharedPointer : public TPointerBase<T>
	{
	private:

		using Base_T = TPointerBase<T>;

	public:

		using typename Base_T::element_type;

		using weak_type = TWeakPointer<T>;

		constexpr TSharedPointer () noexcept = default;

		constexpr TSharedPointer (std::nullptr_t) noexcept {} // Construct empty TSharedPointer.

		template<typename X, std::enable_if_t<std::conjunction_v<std::conditional_t<std::is_array_v<T>, Can_Array_Delete_T<X>, Can_Scalar_Delete_T<X>>, SP_Convertible_T<X, T>>, int32> = 0>
		explicit TSharedPointer(X* Pointer)
		{
			if constexpr (std::is_array_v<T>)
			{
				this->Set_Pointer_Deleter(Pointer, TDefaultDeleter<X[]>{});
			}
			else
			{
				TemporaryOwner_T<X> Owner(Pointer);

				this->Set_Pointer_Reference_Counter_And_Enable_Shared(Owner.Pointer, new TReferenceCounter<X>(Owner.Pointer));

				Owner.Pointer = nullptr;
			}
		}

		template<typename X, typename Deleter_T, std::enable_if_t<std::conjunction_v<std::is_move_constructible<Deleter_T>, Can_Call_Function_Object_T<Deleter_T&, X*&>, SP_Convertible<X, T>>, int32> = 0>
		TSharedPointer(X* Pointer, Deleter_T Deleter)
		{
			this->Set_Pointer_Deleter(Pointer, std::move(Deleter));
		}

		template<typename X, typename Deleter_T, typename Allocator_T, std::enable_if_t<std::conjunction_v<std::is_move_constructible<Deleter_T>, Cal_Call_Function_Object_T<Deleter_T&, X*&>, SP_Convertible_T<X, T>>, int32> = 0>
		TSharedPointer(X* Pointer, Deleter_T Deleter, Allocator_T Allocator)
		{
			this->Set_Pointer_Deleter_Allocator(Pointer, std::move(Deleter), Allocator);
		}

		template<typename Deleter_T, std::enable_if_t<std::conjunction_v<std::is_move_constructible<Deleter_T>, Can_Call_Function_Object_T<Deleter_T&, std::nullptr_t&>>, int32> = 0>
		TSharedPointer(std::nullptr_t, Deleter_T Deleter)
		{
			this->Set_Pointer_Deleter(nullptr, std::move(Deleter));
		}

		template<typename Deleter_T, typename Allocator_T, std::enable_if_t<std::conjunction_v<std::is_move_constructible<Deleter_T>, Can_Call_Function_Object_T<Deleter_T&, std::nullptr_t&>>, int32> = 0>
		TSharedPointer(std::nullptr_t, Deleter_T Deleter, Allocator_T Allocator)
		{
			this->Set_Pointer_Deleter_Allocator(nullptr, std::move(Deleter), Allocator);
		}

		template<typename X>
		TSharedPointer(const TSharedPointer<X>& Right, element_type* Pointer) noexcept
		{
			this->Alias_Construct_From(Right, Pointer);
		}

		template<typename X>
		TSharedPointer(TSharedPointer<X>&& Right, element_type* Pointer) noexcept
		{
			this->Alias_Move_Construct_From(std::move(Right), Pointer);
		}

		TSharedPointer(const TSharedPointer& Right) noexcept
		{
			this->Copy_Construct_From(Right);
		}

		// <memory> 1566

	private:

		template<typename Pointer_T, typename Deleter_T>
		void Set_Pointer_Deleter(const Pointer_T Pointer, Deleter_T Deleter)
		{
			TemporaryOwnerWithDeleter_T<Pointer_T, Deleter_T> Owner(Pointer, Deleter);

			this->Set_Pointer_Reference_Counter_And_Enable_Shared(Owner.Pointer, new TReferenceCounterResource<Pointer_T, Deleter_T>(Owner.Pointer, std::move(Deleter)));

			Owner.b_CallDeleter = false;
		}

		template<typename Pointer_T, typename Deleter_T, typename Allocator_T>
		void Set_Pointer_Deleter_Allocator(const Pointer_T Pointer, Deleter_T Deleter, Allocator_T Allocator)
		{
			using AllocatorReference_T = std::_Rebind_alloc_t<Allocator_T, TReferenceCounterResourceAllocator<Pointer_T, Deleter_T, Allocator_T>>;

			TemporaryOwnerWithDeleter_T<Pointer_T, Deleter_T> Owner(Pointer, Deleter);

			AllocatorReference_T AllocatorReference(Allocator);

			std::_Alloc_construct_ptr<AllocatorReference_T> Constructor(AllocatorReference);

			Constructor._Allocate();

			Construct_In_Place(*Constructor._Ptr, Owner.Pointer, std::move(Deleter), Allocator);

			this->Set_Pointer_Reference_Counter_And_Enable_Shared(Owner.Pointer, Unwrap_Pointer(Constructor._Ptr));

			Constructor._Ptr = nullptr;

			Owner.b_CallDeleter = false;
		}

		template<typename X, typename... Args_T>
		friend std::enable_if_t<!std::is_array_v<X>, TSharedPointer<X>> MakeShared(Args_T&&... Args);

		template<typename X, typename Allocator_T, typename... Args_T>
		friend std::enable_if_t<!std::is_array_v<X>, TSharedPointer<X>> AllocateShared(const Allocator_T& Allocator, Args_T&&... Args);

		template<typename X>
		friend std::enable_if_t<std::is_bounded_array_v<X>, TSharedPointer<X>> MakeShared();

		template<typename X, typename Allocator_T>
		friend std::enable_if_t<std::is_bounded_array_v<X>, TSharedPointer<X>> AllocateShared(const Allocator_T& Allocator);

		template<typename X>
		friend std::enable_if_t<std::is_bounded_array_v<X>, TSharedPointer<X>> MakeShared(const std::remove_extent_t<X>& Value);

		template<typename X, typename Allocator_T>
		friend std::enable_if_t<std::is_bounded_array_v<X>, TSharedPointer<X>> AllocateShared(const Allocator_T& Allocator, const std::remove_extent_t<X>& Value);

		template<typename X>
		friend std::enable_if_t<!std::is_unbounded_array_v<X>, TSharedPointer<X>> MakeSharedForOverwrite();

		template<typename X, typename Allocator_T>
		friend std::enable_if_t<!std::is_unbounded_array_v<X>, TSharedPointer<X>> AllocateSharedForOverwrite(const Allocator_T& Allocator);

		template<typename X, typename... Args_T>
		friend TSharedPointer<X> Make_Shared_Unbounded_Array(Size_T Count, const Args_T&... Args);

		template<typename X, typename Allocator_T, typename... Args_T>
		friend TSharedPointer<X> Allocate_Shared_Unbounded_Array(const Allocator_T& Allocator, Size_T Count, const Args_T&... Args);

		template<typename X>
		void Set_Pointer_Reference_Counter_And_Enable_Shared(X* const Pointer, TReferenceCounterBase* const ReferenceCounter) noexcept
		{
			this->Pointer          = Pointer;
			this->ReferenceCounter = ReferenceCounter;

			if constexpr (std::conjunction_v<std::negation<std::is_array<T>>, std::negation<std::is_volatile<X>>, Can_Enable_Shared_T<X>>)
			{
				if (Pointer && Pointer->WeakPointer.Expired())
				{
					Pointer->WeakPointer = TSharedPointer<std::remove_cv_t<X>>(*this, const_cast<std::remove_cv_t<X>*>(Pointer));
				}
			}
		}

		void Set_Pointer_Reference_Counter_And_Enable_Shared(std::nullptr_t, TReferenceCounterBase* const ReferenceCounter) noexcept
		{
			this->Pointer          = nullptr;
			this->ReferenceCounter = ReferenceCounter;
		}
	};



















































































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