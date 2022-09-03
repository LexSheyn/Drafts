#pragma once

#include <utility>

namespace test
{
	template<typename T>
	class TSingleton
	{
	public:

	// Constructors and Destructor:

		TSingleton()
			: Instance (nullptr)
		{
			StaticInstance = Instance;
		}

		TSingleton(T* Pointer)
			: Instance (Pointer)
		{
			StaticInstance = Instance;
		}

		TSingleton(TSingleton&& Right) noexcept
			: Instance (std::move(Right.Instance))
		{
			Right.Instance = nullptr;

			StaticInstance = Instance;
		}

		~TSingleton()
		{
			this->TryDelete();
		}

	// Operators:

		TSingleton& operator = (TSingleton&& Right) noexcept
		{
			this->TryDelete();

			Instance = std::move(Right.Instance);

			Right.Instance = nullptr;

			StaticInstance = Instance;

			return *this;
		}

	// Accessors:

		static T* Get() noexcept
		{
			return StaticInstance;
		}

	private:

	// Private Functions:

		void TryDelete() const
		{
			if (Instance)
			{
				delete Instance;

				StaticInstance = nullptr;
			}
		}

	// Variables:

		T*        Instance;
		static T* StaticInstance;
	};

	template<typename T, typename... Args_T>
	constexpr TSingleton<T> __fastcall MakeSingleton(Args_T&&... Args)
	{
		return TSingleton<T>(new T(std::forward<Args_T>(Args)...));
	}
}