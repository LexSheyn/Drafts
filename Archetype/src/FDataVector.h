#pragma once

#include <vector>

namespace t3d
{
	class FDataVector
	{
	public:

	// Constructors and Destructor:

		constexpr FDataVector() noexcept
			: ElementSize (sizeof(uint8_t))
		{}

		~FDataVector () = default;

	// Functions:

		constexpr void PushBack(auto Value)
		{
			size_t LastIndex = Data.size();

			Data.resize(LastIndex + ElementSize);

			decltype(Value)* ValuePointer = new (&Data[LastIndex]) decltype(Value)(Value);

			Indices.push_back(LastIndex);
		}

		constexpr void GrowBack()
		{
			size_t LastIndex = Data.size();

			Data.resize(LastIndex + ElementSize);

			Indices.push_back(LastIndex);
		}

		constexpr void PopBack()
		{
			Data.resize(Indices.back());

			Indices.pop_back();
		}

		constexpr void Reserve(size_t ElementCount)
		{
			Indices.reserve(ElementCount);
			Data   .reserve(ElementCount * ElementSize);
		}

		constexpr void Resize(size_t ElementCount)
		{
			Indices.resize(ElementCount);
			Data   .resize(ElementCount * ElementSize);

			for (size_t i = 0u; i < Indices.size(); ++i)
			{
				Indices[i] = ElementSize * i;
			}
		}

		constexpr void Clear() noexcept
		{
			Indices.clear();
			Data   .clear();
		}

		constexpr bool Empty() const noexcept
		{
			return Indices.empty();
		}

	// Accessors:

		constexpr size_t Size() const noexcept
		{
			return Indices.size();
		}

		template<typename T>
		constexpr T& Front()
		{
			return reinterpret_cast<T&>(Data[Indices.front()]);
		}

		template<typename T>
		constexpr T& Back()
		{
			return reinterpret_cast<T&>(Data[Indices.back()]);
		}

		constexpr size_t GetElementSize() const noexcept
		{
			return ElementSize;
		}

#pragma warning(push)
#pragma warning(disable: 4996)

		void CopyValue(size_t ToIndex, size_t FromIndex)
		{
			std::memcpy(&Data[Indices[ToIndex]], &Data[Indices[FromIndex]], ElementSize);
		}

		void CopyValueFromVector(const FDataVector& Right, size_t ToIndex, size_t FromIndex)
		{
			std::memcpy(&Data[Indices[ToIndex]], &Right.Data[Right.Indices[FromIndex]], ElementSize);
		}

#pragma warning(pop)

	// Modifiers:

		constexpr void SetElementSize(size_t Size)
		{
			ElementSize = Size;
		}

		constexpr void SetValue(size_t Index, auto Value)
		{
			this->operator[]<decltype(Value)>(Index) = Value;
		}

	// Operators:

		template<typename T>
		constexpr T& operator [] (size_t Index)
		{
			return reinterpret_cast<T&>(Data[Indices[Index]]);
		}

		template<typename T>
		constexpr const T& operator [] (size_t Index) const
		{
			return reinterpret_cast<const T&>(Data[Indices[Index]]);
		}

	private:

	// Variables:

		std::vector<size_t>  Indices;
		std::vector<uint8_t> Data;
		size_t               ElementSize;
	};
}