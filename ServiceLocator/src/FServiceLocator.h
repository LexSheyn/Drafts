#pragma once

#include <unordered_map>
#include <typeindex>

namespace sl
{
	class FServiceLocator
	{
	public:

		 FServiceLocator () = delete;
		~FServiceLocator () = delete;

		template<class C, typename... Args_T>
		static bool Create(Args_T&&... Args)
		{
			if (Services.contains(typeid(C)))
			{
				return false;
			}

			Services.insert({ typeid(C), new C(std::forward<Args_T>(Args)...) });

			return true;
		}

		template<class C>
		static bool Destroy()
		{
			if (Services.contains(typeid(C)))
			{
				delete Services[typeid(C)];

				Services.erase(typeid(C));

				return true;
			}

			return false;
		}

		template<class C>
		static C& Get()
		{
			return *reinterpret_cast<C*>(Services.at(typeid(C)));
		}

	private:

		static inline std::unordered_map<std::type_index, void*> Services;
	};

//	constexpr size_t Size = sizeof(FServiceLocator);
}