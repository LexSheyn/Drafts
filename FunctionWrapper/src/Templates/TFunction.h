#pragma once

#include "Core/TrickAPI.h"
#include "Core/TrickTypes.h"
#include "Templates/TSharedPointer.h"

#include <exception>
#include <type_traits>
#include <utility>

#if defined T3D_PLATFORM_WINDOWS
#pragma pointers_to_members( full_generality, multiple_inheritance )
#endif // T3D_PLATFORM_WINDOWS

namespace t3d
{
	struct T3D_EXCEPTION_EMPTY_FUNCTION_CALL : std::exception
	{
		const char* what () const override
		{
			return T3D_AS_CHAR_ARRAY(T3D_EXCEPTION_EMPTY_FUNCTION_CALL);
		}
	};

	template<typename T> class TFunction;

	template<typename Return_T, typename... Args_T>
	class TFunction<Return_T(Args_T...)>
	{
	public:

	// Aliases:

		template<class C>
		using MemberFunction_T = Return_T(C::*)(Args_T... Args);
		
		using StaticFunction_T = Return_T(*)(Args_T... Args);

	// Constructors and Destructor:

		constexpr explicit TFunction() noexcept
			: InternalFunction (EmptyCall)
			, p_Instance       (nullptr)
			, MemberFunction   (nullptr)
			, StaticFunction   (nullptr)
			, AbstractFunctor  (nullptr)
		{}

		constexpr TFunction(std::nullptr_t) noexcept
			: InternalFunction (EmptyCall)
			, p_Instance       (nullptr)
			, MemberFunction   (nullptr)
			, StaticFunction   (nullptr)
			, AbstractFunctor  (nullptr)
		{}

		template<class C>
		TFunction(C* Instance, MemberFunction_T<C> MemberFunctionPointer)
			: InternalFunction (MemberCall)
			, p_Instance       (reinterpret_cast<FClass*>(Instance))
			, MemberFunction   (reinterpret_cast<MemberFunction_T<FClass>>(MemberFunctionPointer))
			, StaticFunction   (nullptr)
			, AbstractFunctor  (nullptr)
		{}

		TFunction(StaticFunction_T StaticFunctionPointer)
			: InternalFunction (StaticCall)
			, p_Instance       (nullptr)
			, MemberFunction   (nullptr)
			, StaticFunction   (StaticFunctionPointer)
			, AbstractFunctor  (nullptr)
		{}

		template<typename Functor_T
			, typename = typename std::enable_if<std::is_invocable_r<Return_T, Functor_T, Args_T...>::value, Functor_T>::type
			, typename = typename std::enable_if<std::is_same<std::invoke_result_t<Functor_T, Args_T...>, Return_T>::value, Return_T>::type>
		TFunction(const Functor_T& Functor)
			: InternalFunction (FunctorCall)
			, p_Instance       (nullptr)
			, MemberFunction   (nullptr)
			, StaticFunction   (nullptr)
			, AbstractFunctor  (new TFunctor<Functor_T>(Functor))
		{}

		TFunction(const TFunction& Right)
			: InternalFunction (Right.InternalFunction)
			, p_Instance       (Right.p_Instance)
			, MemberFunction   (Right.MemberFunction)
			, StaticFunction   (Right.StaticFunction)
			, AbstractFunctor  (Right.AbstractFunctor)
		{}

		TFunction(TFunction&& Right)
			: InternalFunction (std::move(Right.InternalFunction))
			, p_Instance       (std::move(Right.p_Instance))
			, MemberFunction   (std::move(Right.MemberFunction))
			, StaticFunction   (std::move(Right.StaticFunction))
			, AbstractFunctor  (std::move(Right.AbstractFunctor))
		{
			Right.InternalFunction = EmptyCall;
			Right.p_Instance       = nullptr;
			Right.MemberFunction   = nullptr;
			Right.StaticFunction   = nullptr;
		//	Right.AbstractFunctor  = nullptr;
		}

	// Functions:

		template<class C>
		constexpr void Bind(C* Instance, MemberFunction_T<C> MemberFunctionPointer)
		{
			InternalFunction = MemberCall;
			p_Instance       = reinterpret_cast<FClass*>(Instance);
			MemberFunction   = reinterpret_cast<MemberFunction_T<FClass>>(MemberFunctionPointer);
			StaticFunction   = nullptr;
			AbstractFunctor.Reset();
		}

		template<class C>
		constexpr bool8 Contains(C* Instance) const
		{
			return reinterpret_cast<C*>(p_Instance) == Instance;
		}

		template<class C>
		constexpr bool8 Contains(const MemberFunction_T<C>& MemberFunctionPointer) const
		{
			return reinterpret_cast<MemberFunction_T<C>>(MemberFunction) == MemberFunctionPointer;
		}

		constexpr bool8 Contains(const StaticFunction_T& StaticFunctionPointer) const noexcept
		{
			return StaticFunction == StaticFunctionPointer;
		}

		template<typename Functor_T
			, typename = typename std::enable_if<std::is_invocable_r<Return_T, Functor_T, Args_T...>::value, Functor_T>::type
			, typename = typename std::enable_if<std::is_same<std::invoke_result_t<Functor_T, Args_T...>, Return_T>::value, Return_T>::type
			, typename = typename std::enable_if<std::is_same<Functor_T, TFunction<Return_T(Args_T...)>>::value == false, Functor_T>::type>
		constexpr bool8 ContainsFunctorOfType()
		{	
			return dynamic_cast<TFunctor<Functor_T>*>(AbstractFunctor.Get());
		}

		constexpr bool8 HasSameInstance(const TFunction& Right) const noexcept
		{
			return p_Instance == Right.p_Instance;
		}

		constexpr bool8 HasSameMemberFunction(const TFunction& Right) const noexcept
		{
			return MemberFunction == Right.MemberFunction;
		}

		constexpr bool8 HasSameStaticFunction(const TFunction& Right) const noexcept
		{
			return StaticFunction == Right.StaticFunction;
		}

		constexpr bool8 IsEmpty() const noexcept
		{
			return InternalFunction == EmptyCall;
		}

		constexpr void Swap(TFunction& Right) noexcept
		{
			std::swap(InternalFunction, Right.InternalFunction);
			std::swap(p_Instance      , Right.p_Instance);
			std::swap(MemberFunction  , Right.MemberFunction);
			std::swap(StaticFunction  , Right.StaticFunction);
			AbstractFunctor.Swap(Right.AbstractFunctor);
		}

	// Operators:

		constexpr TFunction& operator = (const TFunction& Right)
		{
			InternalFunction = Right.InternalFunction;
			p_Instance       = Right.p_Instance;
			MemberFunction   = Right.MemberFunction;
			StaticFunction   = Right.StaticFunction;
			AbstractFunctor  = Right.AbstractFunctor;
	
			return *this;
		}

		constexpr TFunction& operator = (TFunction&& Right)
		{
			InternalFunction = std::move(Right.InternalFunction);
			p_Instance       = std::move(Right.p_Instance);
			MemberFunction   = std::move(Right.MemberFunction);
			StaticFunction   = std::move(Right.StaticFunction);
			AbstractFunctor  = std::move(Right.AbstractFunctor);

			Right.InternalFunction = EmptyCall;
			Right.p_Instance       = nullptr;
			Right.MemberFunction   = nullptr;
			Right.StaticFunction   = nullptr;
		//	Right.AbstractFunctor  = nullptr;

			return *this;
		}

		constexpr TFunction& operator = (std::nullptr_t) noexcept
		{
			InternalFunction = EmptyCall;
			p_Instance       = nullptr;
			MemberFunction   = nullptr;
			StaticFunction   = nullptr;
			AbstractFunctor.Reset();

			return *this;
		}

		constexpr TFunction& operator = (StaticFunction_T StaticFunctionPointer)
		{
			InternalFunction = StaticCall;
			p_Instance       = nullptr;
			MemberFunction   = nullptr;
			StaticFunction   = StaticFunctionPointer;
			AbstractFunctor.Reset();

			return *this;
		}

		template<typename Functor_T
			, typename = typename std::enable_if<std::is_invocable_r<Return_T, Functor_T, Args_T...>::value, Functor_T>::type
			, typename = typename std::enable_if<std::is_same<std::invoke_result_t<Functor_T, Args_T...>, Return_T>::value, Return_T>::type>
		constexpr TFunction& operator = (const Functor_T& Functor)
		{
			InternalFunction = FunctorCall;
			p_Instance       = nullptr;
			MemberFunction   = nullptr;
			StaticFunction   = nullptr;
			AbstractFunctor  = new TFunctor<Functor_T>(Functor);

			return *this;
		}

		constexpr Return_T operator () (Args_T... Args) const
		{
			return (*this->InternalFunction)(this, std::forward<Args_T>(Args)...);
		}

		constexpr explicit operator bool () const noexcept
		{
			return InternalFunction != EmptyCall;
		}

	private:

		class IFunctor
		{
		public:

			         IFunctor() = default;
			virtual ~IFunctor() = default;

			virtual Return_T operator () (Args_T... Args) = 0;
		};

		template<typename Functor_T
			, typename = typename std::enable_if<std::is_invocable_r<Return_T, Functor_T, Args_T...>::value, Functor_T>::type
			, typename = typename std::enable_if<std::is_same<std::invoke_result_t<Functor_T, Args_T...>, Return_T>::value, Return_T>::type>
		class TFunctor : public IFunctor
		{
		public:

			TFunctor(const Functor_T& Functor)
				: InternalFunctor (Functor)
			{}

			~TFunctor () override = default;

			Return_T operator () (Args_T... Args) override
			{
				return InternalFunctor(std::forward<Args_T>(Args)...);
			}

			const Functor_T& GetFunctor() const
			{
				return InternalFunctor;
			}

		private:

			Functor_T InternalFunctor;
		};

		using InternalFunction_T = Return_T(*)(const TFunction* This, Args_T... Args);

	// Private Functions:

		static constexpr Return_T T3D_CALL MemberCall(const TFunction* This, Args_T... Args)
		{
			return (This->p_Instance->*This->MemberFunction)(std::forward<Args_T>(Args)...);
		}

		static constexpr Return_T T3D_CALL StaticCall(const TFunction* This, Args_T... Args)
		{
			return This->StaticFunction(std::forward<Args_T>(Args)...);
		}

		static constexpr Return_T T3D_CALL FunctorCall(const TFunction* This, Args_T... Args)
		{
			return (*This->AbstractFunctor)(std::forward<Args_T>(Args)...);
		}

		static constexpr Return_T T3D_CALL EmptyCall(const TFunction* This, Args_T... Args)
		{
			throw T3D_EXCEPTION_EMPTY_FUNCTION_CALL();

			return Return_T();
		}

	// Variables:

		class FClass;

		InternalFunction_T       InternalFunction;
		FClass*                  p_Instance;
		MemberFunction_T<FClass> MemberFunction;
		StaticFunction_T         StaticFunction;
		TSharedPointer<IFunctor> AbstractFunctor;
	};

//	constexpr Size_T Size = sizeof(TFunction<void()>);
}