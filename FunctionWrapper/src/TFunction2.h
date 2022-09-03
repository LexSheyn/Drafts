#pragma once

typedef unsigned long long uint64;
typedef bool               bool8;

#if defined _MSC_VER
#pragma pointers_to_members( full_generality, multiple_inheritance )
#endif // T3D_PLATFORM_WINDOWS

namespace t3d
{
	template<typename T> class TFunction;

	template<typename Return_T, typename... Args_T>
	class TFunction<Return_T(Args_T...)>
	{
	public:
		
	// Aliases:

		template<class C>
		using CallbackMember_T = Return_T(C::*)(Args_T... Args);
		
		using CallbackStatic_T = Return_T(*)(Args_T... Args);

	// Constructors:

		TFunction()
			: FunctionPointers { &TFunction::CallMember, &TFunction::CallStatic } 
			, CallbackMember   (nullptr)
			, CallbackStatic   (nullptr)
			, Instance         (nullptr)
			, Index            (0)
		{
		}

		template<class C>
		TFunction(C* Instance, CallbackMember_T<C> Callback)
			: FunctionPointers { &TFunction::CallMember, &TFunction::CallStatic }
			, CallbackMember   (reinterpret_cast<CallbackMember_T<FClass>>(Callback))
			, CallbackStatic   (nullptr)
			, Instance         (reinterpret_cast<FClass*>(Instance))
			, Index            (0)
		{
		}

		TFunction(CallbackStatic_T Callback)
			: FunctionPointers { &TFunction::CallMember ,  &TFunction::CallStatic }
			, CallbackMember   (nullptr)
			, CallbackStatic   (Callback)
			, Instance         (nullptr)
			, Index            (1)
		{
		}

	// Functions:

		template<class C>
		constexpr void Bind(C* Instance, CallbackMember_T<C> Callback) noexcept
		{
			this->Instance       = reinterpret_cast<FClass*>(Instance);
			this->CallbackMember = reinterpret_cast<CallbackMember_T<FClass>>(Callback);
			this->CallbackStatic = nullptr;
			this->Index          = 0;
		}

		constexpr void Bind(CallbackStatic_T Callback) noexcept
		{
			this->Instance       = nullptr;
			this->CallbackMember = nullptr;
			this->CallbackStatic = Callback;
			this->Index          = 1;
		}

		constexpr Return_T Invoke(Args_T... Args) const
		{
			return (*this->FunctionPointers[Index])(this, std::forward<Args_T>(Args)...);
		}

		template<class C>
		constexpr bool8 IsEqual(CallbackMember_T<C> Callback) const
		{
			return reinterpret_cast<CallbackMember_T<C>>(CallbackMember) == Callback;
		}

		constexpr bool8 IsEqual(CallbackStatic_T Callback) const noexcept
		{
			return CallbackStatic == Callback;
		}

		constexpr bool8 IsEmpty() const noexcept
		{
			return (Instance == nullptr) && (CallbackMember == nullptr) && (CallbackStatic == nullptr);
		}

	// Operators:

		constexpr Return_T operator () (Args_T... Args) const
		{
			return (*this->FunctionPointers[Index])(this, std::forward<Args_T>(Args)...);
		}

		explicit operator bool () const noexcept
		{
			return (Instance && CallbackMember) || CallbackStatic;
		}

		constexpr bool8 operator == (const TFunction& Right) const noexcept
		{
			return (Instance == Right.Instance) && (CallbackMember == Right.CallbackMember) && (CallbackStatic == Right.CallbackStatic);
		}

		constexpr bool8 operator != (const TFunction& Right) const noexcept
		{
			return (Instance != Right.Instance) || (CallbackMember != Right.CallbackMember) || (CallbackStatic != Right.CallbackStatic);
		}

	private:

		using CallbackInternal_T = Return_T(*)(const TFunction* This, Args_T... Args);

	// Private Functions:

		static constexpr Return_T __fastcall CallMember(const TFunction* This, Args_T... Args)
		{
			return (This->Instance->*This->CallbackMember)(std::forward<Args_T>(Args)...);
		}

		static constexpr Return_T __fastcall CallStatic(const TFunction* This, Args_T... Args)
		{
			return This->CallbackStatic(std::forward<Args_T>(Args)...);
		}

	// Variables:

		class FClass;

		CallbackInternal_T       FunctionPointers[2];
		CallbackMember_T<FClass> CallbackMember;		
		CallbackStatic_T         CallbackStatic;
		FClass*                  Instance;
		uint64                   Index;
	};
}