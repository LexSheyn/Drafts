#pragma once

namespace sl
{
	class C {};

	template<class C>
	class TService
	{
	public:

		 TService() = delete;
		~TService() = delete;

		template<typename... Args_T>
		static bool Create(Args_T&&... Args)
		{
			if (Instance != nullptr)
			{
				return false;
			}

			Instance = new C(std::forward<Args_T>(Args)...);

			return true;
		}

		static bool Destroy()
		{
			if (Instance == nullptr)
			{
				return false;
			}

			delete Instance;

			Instance = nullptr;

			return true;
		}

		static C& Get()
		{
			return *reinterpret_cast<C*>(Instance);
		}

	private:

		static inline void* Instance = nullptr;
		void* Instance = nullptr;
	};

//	constexpr size_t Size = sizeof(TService<C>);
}