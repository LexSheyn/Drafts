#pragma once

#include "TrickUtility.h"
#include "TReferenceWrapper.h"
#include "TrickPointers.h"

#include <memory>
#include <type_traits>
#include <typeinfo>
#include <atomic>
#include <exception>
#include <intrin.h>
#include <cassert>

namespace t3d
{
	// unsigned long.
	// Remember: "long" and "int" are NOT the same type!
	// "long" is required for external atomic functions in 2022 MSVC.
	using AtomicCounter_T = std::_Atomic_counter_t;

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

	// Private Functions:

		// Destroy managed resource.
		virtual void Destroy     () noexcept = 0;
		// Destroy self.
		virtual void Delete_This () noexcept = 0;

	// Variables:

		AtomicCounter_T Uses  = 1;
		AtomicCounter_T Weaks = 1;

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
	class TWeakPointer;

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

		void Swap_Contents(TPointerBase& Right) noexcept
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

		using Base_T::Get;

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

		template<typename X, std::enable_if_t<SP_Pointer_Compatible_T<X, T>::value, int32> = 0>
		TSharedPointer(const TSharedPointer<X>& Right) noexcept
		{
			this->Copy_Construct_From(Right);
		}

		TSharedPointer(TSharedPointer&& Right) noexcept
		{
			this->Move_Construct_From(Move(Right));
		}

		template<typename X, std::enable_if_t<SP_Pointer_Compatible_T<X, T>::value, int32> = 0>
		TSharedPointer(TSharedPointer<X>&& Right) noexcept
		{
			this->Move_Construct_From(Move(Right));
		}

		template<typename X, std::enable_if_t<SP_Pointer_Compatible_T<X, T>::value, int32> = 0>
		explicit TSharedPointer(const TWeakPointer<X>& Right)
		{
			if (!this->Construct_From_Weak(Right))
			{
				Throw_Bad_Weak_Pointer();
			}
		}

		template<typename X, typename Deleter_T, std::enable_if_t<std::conjunction_v<SP_Pointer_Compatible_T<typename TUniquePointer<X, Deleter_T>::pointer, element_type*>>, int32> = 0>
		TSharedPointer(TUniquePointer<X, Deleter_T>&& Right)
		{
			using Smart_T          = typename TUniquePointer<X, Deleter_T>::pointer;
			using Raw_T            = typename TUniquePointer<X, Deleter_T>::element_type*;
		//	using EnabledDeleter_T = std::conditional_t<std::is_reference_v<Deleter_T>, decltype(std::ref(Right.GetDeleter())), Deleter_T>;
			using EnabledDeleter_T = std::conditional_t<std::is_reference_v<Deleter_T>, decltype(Reference(Right.GetDeleter())), Deleter_T>;
			
			const Smart_T SmartPointer = Right.Get();

			if (SmartPointer)
			{
				const Raw_T RawPointer = SmartPointer;

				const auto ReferenceCounter = new TReferenceCounterResource<Smart_T, Deleter_T>(SmartPointer, Forward<Deleter_T>(Right.GetDeleter()));

				this->Set_Pointer_Reference_Counter_And_Enable_Shared(RawPointer, ReferenceCounter);

				Right.Release();
			}
		}

		~TSharedPointer() noexcept
		{
			this->Decrement_Reference_Count();
		}

		TSharedPointer& operator = (const TSharedPointer& Right) noexcept
		{
			TSharedPointer(Right).Swap(*this);

			return *this;
		}

		template<typename X>
		TSharedPointer& operator = (const TSharedPointer<X>& Right) noexcept
		{
			TSharedPointer(Right).Swap(*this);

			return *this;
		}

		TSharedPointer& operator = (TSharedPointer&& Right) noexcept
		{
			TSharedPointer(Move(Right)).Swap(*this);

			return *this;
		}

		template<typename X>
		TSharedPointer& operator = (TSharedPointer<X>&& Right) noexcept
		{
			TSharedPointer(Move(Right)).Swap(*this);

			return *this;
		}

		template<typename X, typename Deleter_T>
		TSharedPointer& operator = (TUniquePointer<X, Deleter_T>&& Right) noexcept
		{
			TSharedPointer(Move(Right)).Swap(*this);

			return *this;
		}

		void Swap(TSharedPointer& Right) noexcept
		{
			this->Swap_Contents(Right);
		}

		void Reset() noexcept
		{
			TSharedPointer().Swap(*this);
		}

		template<typename X>
		void Reset(X* Pointer)
		{
			TSharedPointer(Pointer).Swap(*this);
		}

		template<typename X, typename Deleter_T>
		void Reset(X* Pointer, Deleter_T Deleter)
		{
			TSharedPointer(Pointer, Deleter).Swap(*this);
		}

		template<typename X, typename Deleter_T, typename Allocator_T>
		void Reset(X* Pointer, Deleter_T Deleter, Allocator_T Allocator)
		{
			TSharedPointer(Pointer, Deleter, Allocator).Swap(*this);
		}

		template<typename X = T, std::enable_if_t<!std::disjunction_v<std::is_array<X>, std::is_void<X>>, int32> = 0>
		T3D_NO_DISCARD X& operator * () const noexcept
		{
			return *Get();
		}

		template<typename X = T, std::enable_if_t<!std::is_array_v<X>, int32> = 0>
		T3D_NO_DISCARD X* operator -> () const noexcept
		{
			return Get();
		}

		template<typename X = T, typename Element_T = element_type, std::enable_if_t<std::is_array_v<X>, int32> = 0>
		T3D_NO_DISCARD Element_T& operator [] (std::ptrdiff_t Index) const noexcept
		{
			return Get()[Index];
		}

		explicit operator bool () const noexcept
		{
			return Get() != nullptr;
		}

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

	template<typename T>
	TSharedPointer(TWeakPointer<T>) -> TSharedPointer<T>;

	template<typename T, typename Deleter_T>
	TSharedPointer(TUniquePointer<T, Deleter_T>) -> TSharedPointer<T>;

// Operators:

	template<typename T1, typename T2>
	T3D_NO_DISCARD bool8 operator == (const TSharedPointer<T1>& Left, const TSharedPointer<T2>& Right) noexcept
	{
		return Left.Get() == Right.Get();
	}

	template<typename T1, typename T2>
	T3D_NO_DISCARD std::strong_ordering operator <=> (const TSharedPointer<T1>& Left, const TSharedPointer<T2>& Right) noexcept
	{
		return Left.Get() <=> Right.Get();
	}

	template<typename T>
	T3D_NO_DISCARD bool8 operator == (const TSharedPointer<T>& Left, std::nullptr_t) noexcept
	{
		return Left.Get() == nullptr;
	}

	template<typename T>
	T3D_NO_DISCARD std::strong_ordering operator <=> (const TSharedPointer<T>& Left, std::nullptr_t) noexcept
	{
		return Left.Get() <=> static_cast<typename TSharedPointer<T>::element_type*>(nullptr);
	}

	template<typename Element_T, typename Traits_T, typename T>
	std::basic_ostream<Element_T, Traits_T>& operator << (std::basic_ostream<Element_T, Traits_T>& OutputStream, const TSharedPointer<T>& Pointer)
	{
		return OutputStream << Pointer.Get();
	}

// Swap:

	template<typename T>
	void Swap(TSharedPointer<T>& Left, TSharedPointer<T>& Right) noexcept
	{
		return Left.Swap(Right);
	}

// Type Casting:

	template<typename T1, typename T2>
	T3D_NO_DISCARD TSharedPointer<T1> StaticPointerCast(const TSharedPointer<T2>& Right) noexcept
	{
		const auto Pointer = static_cast<typename TSharedPointer<T1>::element_type*>(Right.Get());

		return TSharedPointer<T1>(Right, Pointer);
	}

	template<typename T1, typename T2>
	T3D_NO_DISCARD TSharedPointer<T1> StaticPointerCast(TSharedPointer<T2>&& Right) noexcept
	{
		const auto Pointer = static_cast<typename TSharedPointer<T1>::element_type*>(Right.Get());

		return TSharedPointer<T1>(Move(Right), Pointer);
	}

	template<typename T1, typename T2>
	T3D_NO_DISCARD TSharedPointer<T1> ConstPointerCast(const TSharedPointer<T2>& Right) noexcept
	{
		const auto Pointer = const_cast<typename TSharedPointer<T1>::element_type*>(Right.Get());

		return TSharedPointer<T1>(Right, Pointer);
	}

	template<typename T1, typename T2>
	T3D_NO_DISCARD TSharedPointer<T1> ConstPointerCast(TSharedPointer<T2>&& Right) noexcept
	{
		const auto Pointer = const_cast<typename TSharedPointer<T1>::element_type*>(Right.Get());

		return TSharedPointer<T1>(Move(Right), Pointer);
	}

	template<typename T1, typename T2>
	T3D_NO_DISCARD TSharedPointer<T1> ReinterpretPointerCast(const TSharedPointer<T2>& Right) noexcept
	{
		const auto Pointer = reinterpret_cast<typename TSharedPointer<T1>::element_type*>(Right.Get());

		return TSharedPointer<T1>(Right, Pointer);
	}

	template<typename T1, typename T2>
	T3D_NO_DISCARD TSharedPointer<T1> ReinterpretPointerCast(TSharedPointer<T2>&& Right) noexcept
	{
		const auto Pointer = reinterpret_cast<typename TSharedPointer<T1>::element_type*>(Right.Get());

		return TSharedPointer<T1>(Move(Right), Pointer);
	}

	template<typename T1, typename T2>
	T3D_NO_DISCARD TSharedPointer<T1> DynamicPointerCast(const TSharedPointer<T2>& Right) noexcept
	{
		const auto Pointer = dynamic_cast<typename TSharedPointer<T1>::element_type*>(Right.Get());

		if (Pointer)
		{
			return TSharedPointer<T1>(Right, Pointer);
		}

		return {};
	}

	template<typename T1, typename T2>
	T3D_NO_DISCARD TSharedPointer<T1> DynamicPointerCast(TSharedPointer<T2>&& Right) noexcept
	{
		const auto Pointer = dynamic_cast<typename TSharedPointer<T1>::element_type*>(Right.Get());

		if (Pointer)
		{
			return TSharedPointer<T1>(Right, Pointer);
		}

		return {};
	}

	template<typename Deleter_T, typename T>
	T3D_NO_DISCARD Deleter_T* GetDeleter(const TSharedPointer<T>& SharedPointer) noexcept
	{
		if (SharedPointer.ReferenceCounter)
		{
			return static_cast<Deleter_T*>(SharedPointer.ReferenceCounter->GetDeleter(typeid(Deleter_T)));
		}

		return nullptr;
	}

	struct For_Overwrite_Tag
	{
		explicit For_Overwrite_Tag () = default;
	};

	// <xmemory> 1923
#pragma warning (push)
#pragma warning (disable : 4624) // '%s': destructor was implicitly defined as deleted.
	template<typename T>
	struct Wrapper_T
	{
		T Value; // Workaround for VSO-586813 "T^ is not allowed in a union".
	};
#pragma warning (pop)

	template<typename T>
	class TReferenceCounterObject : public TReferenceCounterBase // Handle reference counting for object in control block, no allocator.
	{
	public:

		template<typename... Args_T>
		explicit TReferenceCounterObject(Args_T&&... Args) : TReferenceCounterBase()
		{
			if constexpr (sizeof...(Args_T) == 1 && (std::is_same_v<For_Overwrite_Tag, std::remove_cvref_t<Args_T>>&&...))
			{
				Default_Construct_In_Place(Storage.Value);

				((void)Args, ...);
			}
			else
			{
				Construct_In_Place(Storage.Value, Forward<Args_T>(Args)...);
			}
		}

		~TReferenceCounterObject() noexcept override
		{
			// MSVC: TRANSITION, should be non-virtual.
			// Nothing to do, Storage.Value was already destroyed in Destroy().
			
			// MSVC: N4849 [class.dtor]/7:
			// "A defaulted destructor for a class X is defined as deleted if:
			// X is a union-like class that has a variant member with a non-trivial destructor".
		}

		union
		{
			Wrapper_T<T> Storage;
		};

	private:

		void Destroy() noexcept override
		{
			Destroy_In_Place(Storage.Value);
		}

		void Delete_This() noexcept override
		{
			delete this;
		}

	};

	template<Size_T Alignment>
	struct Align_As_Storage_Unit_T
	{
		alignas(Alignment) char Space[Alignment];
	};

	enum class ECheckOverflow : bool8 { No, Yes };

	template<typename ReferenceCounter_T, ECheckOverflow Check>
	T3D_NO_DISCARD Size_T Calculate_Bytes_For_Flexible_Array(const Size_T Count) noexcept(Check == ECheckOverflow::No)
	{
		constexpr Size_T Alignment = alignof(ReferenceCounter_T);

		Size_T Bytes = sizeof(ReferenceCounter_T); // Contains storage for one element.

		if (Count > 1)
		{
			constexpr Size_T ElementSize = sizeof(typename ReferenceCounter_T::element_type);

			Size_T ExtraBytes = 0;

			if constexpr (Check == ECheckOverflow::Yes)
			{
				ExtraBytes = Get_Size_Of_N<ElementSize>(Count - 1); // Check multiplication overflow.

				if (ExtraBytes > static_cast<Size_T>(-1) - Bytes - (Alignment - 1)) // Assume worst case adjustment.
				{
					Throw_Bad_Array_New_Length();
				}
			}
			else
			{
				ExtraBytes = ElementSize * (Count - 1);
			}

			Bytes += ExtraBytes;

			Bytes = (Bytes + Alignment - 1) & ~(Alignment - 1);

			T3D_ASSERT(Bytes % sizeof(Align_As_Storage_Unit_T<Alignment>) == 0);

			return Bytes;
		}
	}

	template<typename ReferenceCounter_T>
	T3D_NO_DISCARD ReferenceCounter_T* Allocate_Flexible_Array(const Size_T Count)
	{
		const Size_T Bytes         = Calculate_Bytes_For_Flexible_Array<ReferenceCounter_T, ECheckOverflow::Yes>(Count);
		constexpr Size_T Alignment = alignof(ReferenceCounter_T);

		if constexpr (Alignment > __STDCPP_DEFAULT_NEW_ALIGNMENT__)
		{
			return static_cast<ReferenceCounter_T*>(::operator new (Bytes, std::align_val_t(Alignment)));
		}
		else
		{
			return static_cast<ReferenceCounter_T*>(::operator new (Bytes));
		}
	}

	template<typename ReferenceCounter_T>
	void Deallocate_Flexible_Array(ReferenceCounter_T* const Pointer) noexcept
	{
		constexpr Size_T Alignment = alignof(ReferenceCounter_T);

		if constexpr (Alignment > __STDCPP_DEFAULT_NEW_ALIGNMENT__)
		{
			::operator delete (static_cast<void*>(Pointer), std::align_val_t(Alignment));
		}
		else
		{
			::operator delete (static_cast<void*>(Pointer));
		}
	}

	template<typename Iterator_T>
	struct T3D_NO_DISCARD Uninitialized_Rev_Destroying_Backout_T
	{
		explicit Uninitialized_Rev_Destroying_Backout_T (Iterator_T Destination) noexcept
			: First {Destination}
			, Last  {Destination}
		{}

		Uninitialized_Rev_Destroying_Backout_T(const Uninitialized_Rev_Destroying_Backout_T&) = delete;

		Uninitialized_Rev_Destroying_Backout_T& operator = (const Uninitialized_Rev_Destroying_Backout_T&) = delete;

		~Uninitialized_Rev_Destroying_Backout_T()
		{
			while (Last != First)
			{
				--Last;

				Destroy_At(AddressOf(*Last));
			}
		}

		template<typename... Args_T>
		void Emplace_Back(Args_T&&... Args)
		{
			Construct_In_Place(*Last, Forward<Args_T>(Args)...);

			++Last;
		}

		void Emplace_Back_For_Overwrite()
		{
			Default_Construct_In_Place(*Last);

			++Last;
		}

		Iterator_T Release() noexcept
		{
			First = Last;

			return Last;
		}

		Iterator_T First;
		Iterator_T Last;
	};

	template<typename T>
	void Reverse_Destroy_Multidimensional_N(T* const Array, Size_T Size) noexcept
	{
		while (Size > 0)
		{
			--Size;

			if constexpr (std::is_array_v<T>)
			{
				Reverse_Destroy_Multidimensional_N(Array[Size], std::extent_v<T>);
			}
			else
			{
				Destroy_In_Place(Array[Size]);
			}
		}
	}

	template<typename T>
	struct T3D_NO_DISCARD TReverse_Destroy_Multidimensional_N_Guard
	{
		~TReverse_Destroy_Multidimensional_N_Guard()
		{
			if (Target)
			{
				Reverse_Destroy_Multidimensional_N(Target, Index);
			}
		}

		T*     Target;
		Size_T Index;
	};

	template<typename T, Size_T Size>
	void Uninitialized_Copy_Multidimensional(const T(&In)[Size], T(&Out)[Size])
	{
		if constexpr (std::is_trivial_v<T>)
		{
			std::_Copy_memmove(In, In + Size, Out);
		}
		else if constexpr (std::is_array_v<T>)
		{
			TReverse_Destroy_Multidimensional_N_Guard<T> Guard{ Out, 0 };

			for (Size_T& Index = Guard.Index; Index < Size; ++Index)
			{
				Uninitialized_Copy_Multidimensional(In[Index], Out[Index]);
			}

			Guard.Target = nullptr;
		}
		else
		{
			Uninitialized_Rev_Destorying_Backout_T Backout{ Out };

			for (Size_T Index = 0u; Index < Size; ++Index)
			{
				Backout.Emplace_Back(In[Index]);
			}

			Backout.Release();
		}
	}

	template<typename T>
	void Uninitialized_Value_Construct_Multidimensional_N(T* const Out, const Size_T Size)
	{
		using Item_T = std::remove_all_extents_t<T>;

		if constexpr (std::_Use_memset_value_construct_v<Item_T*>)
		{
			std::_Zero_range(Out, Out + Size);
		}
		else if constexpr (std::is_array_v<T>)
		{
			TReverse_Destroy_Multidimensional_N_Guard<T> Guard{ Out, 0 };

			for (Size_T& Index = Guard.Index; Index < Size; ++Index)
			{
				Uninitialized_Value_Construct_Multidimensional_N(Out[Index], std::extent_v<T>);
			}

			Guard.Target = nullptr;
		}
		else
		{
			Uninitialized_Rev_Destroying_Backout_T Backout{ Out };

			for (Size_T Index = 0u; Index < Size; ++Index)
			{
				Backout.Emplace_Back();
			}

			Backout.Release();
		}
	}

	template<typename T>
	void Uninitialized_Default_Construct_Multidimensional_N(T* const Out, const Size_T Size)
	{
		if constexpr (!std::is_trivially_default_constructible_v<T>)
		{
			if constexpr (std::is_array_v<T>)
			{
				TReverse_Destroy_Multidimensional_N_Guard<T> Guard{ Out, 0 };

				for (Size_T& Index = Guard.Index; Index < Size; ++Index)
				{
					Uninitialized_Default_Construct_Multidimensional_N(Out[Index], std::extent_v<T>);
				}

				Guard.Target = nullptr;
			}
			else
			{
				Uninitialized_Rev_Destroying_Backout_T Backout{ Out };

				for (Size_T Index = 0u; Index < Size; ++Index)
				{
					Backout.Emplace_Back_For_Overwrite();
				}

				Backout.Release();
			}
		}
	}

	template<typename T>
	void Uninitialized_Fill_Multidimensional_N(T* const Out, const Size_T Size, const T& Value)
	{
		if constexpr (std::is_array_v<T>)
		{
			TReverse_Destroy_Multidimensional_N_Guard<T> Guard{ Out, 0 };
			
			for (Size_T& Index = Guard.Index; Index < Size; ++Index)
			{
				Uninitialized_Copy_Multidimensional(Value, Out[Index]);
			}

			Guard.Target = nullptr;
		}
		else if constexpr (std::_Fill_memset_is_safe<T*, T>)
		{
			std::_Fill_memset(Out, Value, Size);
		}
		else
		{
			if constexpr (std::_Fill_zero_memset_is_safe<T*, T>)
			{
				if (std::_Is_all_bits_zero(Value))
				{
					std::_Fill_zero_memset(Out, Size);

					return;
				}
			}

			Uninitialized_Rev_Destroying_Backout_T Backout{ Out };

			for (Size_T Index = 0u; Index < Size; ++Index)
			{
				Backout.Emplace_Back(Value);
			}

			Backout.Release();
		}
	}

	// <memory> 2255
	// ...
	// <memory> 2719

	template<typename T, typename... Args_T>
	T3D_NO_DISCARD std::enable_if_t<!std::is_array_v<T>, TSharedPointer<T>> MakeShared(Args_T&&... Args)
	{
		const auto ReferenceCounter = new TReferenceCounterObject<T>(Forward<Args_T>(Args)...);

		TSharedPointer<T> SharedPointer;

		SharedPointer.Set_Pointer_Reference_Counter_And_Enable_Shared(AddressOf(ReferenceCounter->Storage.Value), SharedPointer);

		return SharedPointer;
	}

	template<typename ReferenceCounter_T>
	struct T3D_NO_DISCARD TGlobal_Delete_Guard
	{
		~TGlobal_Delete_Guard()
		{
			// MSVC:
			// While this branch is technically unnecessary because N4849 [new.delete.single]/17 requires
			// `::operator delete(nullptr)` to be a no-op, it's here to help optimizers see that after
			// `_Guard._Target = nullptr;`, this destructor can be eliminated.

			if (Target)
			{
				Deallocate_Flexible_Array(Target);
			}
		}

		ReferenceCounter_T* Target;
	};

	// <memory> 2751

} // namespace t3d

namespace std
{
	// Trick injected specialization.
	template<typename T>
	void swap(t3d::TSharedPointer<T>& Left, t3d::TSharedPointer<T>& Right) noexcept
	{
		Left.Swap(Right);
	}

} // namespace std