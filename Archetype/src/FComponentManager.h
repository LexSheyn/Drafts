#pragma once

#include "TrickTypesECS.h"

#include <algorithm>
#include <vector>

namespace t3d
{
	template<typename T>
	struct TComponentInfo
	{
		static inline size_t Id;
	};

	class FComponentManager
	{
	public:

		template<typename T>
		static void __fastcall RegisterComponent()
		{
			TComponentInfo<T>::Id = ComponentSizes.size();

			ComponentSizes.push_back(sizeof(T));
		}

		static size_t __fastcall GetComponentSize(ComponentId_T Id)
		{
			return ComponentSizes[Id];
		}

		template<typename... Components_T>
		static ComponentSignature_T __fastcall CreateComponentSignature()
		{
			ComponentSignature_T Signature = { TComponentInfo<Components_T>::Id... };

			std::sort(Signature.begin(), Signature.end());

			return Signature;
		}

		template<typename... ComponentIds_T>
		static ComponentSignature_T __fastcall CreateComponentSignature(ComponentIds_T... Ids)
		{
			ComponentSignature_T Signature = { Ids... };

			std::sort(Signature.begin(), Signature.end());

			return Signature;
		}

		template<typename... Components_T>
		static void __fastcall AddComponentsToSignature(ComponentSignature_T& Signature)
		{
			((Signature.push_back(TComponentInfo<Components_T>::Id)), ...);

			std::sort(Signature.begin(), Signature.end());
		}

		template<typename... ComponentIds_T>
		static void __fastcall AddComponentsToSignature(ComponentSignature_T& Signature, ComponentIds_T... Ids)
		{
			((Signature.push_back(Ids)), ...);

			std::sort(Signature.begin(), Signature.end());
		}

		template<typename... Components_T>
		static void __fastcall RemoveComponentsFromSignature(ComponentSignature_T& Signature)
		{
			((std::erase(Signature, TComponentInfo<Components_T>::Id)), ...);

			std::sort(Signature.begin(), Signature.end());
		}

		template<typename... ComponentIds_T>
		static void __fastcall RemoveComponentsFromSignature(ComponentSignature_T& Signature, ComponentIds_T... Ids)
		{
			((std::erase(Signature, Ids)), ...);

			std::sort(Signature.begin(), Signature.end());
		}

		template<typename... Components_T>
		static bool __fastcall Contains(const ComponentSignature_T& Left)
		{
			ComponentSignature_T Signature = CreateComponentSignature<Components_T...>();

			return std::includes(Left.begin(), Left.end(), Signature.begin(), Signature.end());
		}

		static bool __fastcall LeftContainsRight(const ComponentSignature_T& Left, const ComponentSignature_T& Right)
		{
			return std::includes(Left.begin(), Left.end(), Right.begin(), Right.end());
		}

	private:

		static inline std::vector<size_t> ComponentSizes;
	};
}