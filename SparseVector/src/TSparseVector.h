#pragma once

#include <type_traits>
#include <vector>

namespace t3d
{
	template<typename T, typename = typename std::enable_if<std::is_default_constructible<T>::value, T>::type>
	class TSparseVector
	{
	public:

	// Constructors and Destructor:

		 TSparseVector () = default;
		~TSparseVector () = default;

	// Functions:

		size_t Emplace(T Value)
		{
			size_t Index = this->GetAwailableIndex();

			Data[Index] = Value;

			return Index;
		}

		void MarkFree(size_t Index)
		{
		//	T3D_ASSERT(Index < Indices.size(), "Failed to free element, index is out of range!");

			FreeIndices.push_back(Index);
		}

		constexpr void Clear() noexcept
		{
			Indices    .clear();
			FreeIndices.clear();
			Data       .clear();
		}

	// Accessors:

		constexpr T& At(size_t Index)
		{
			return Data.at(Indices.at(Index));
		}

		constexpr const T& At(size_t Index) const
		{
			return Data.at(Indices.at(Index));
		}

		constexpr size_t Size() const noexcept
		{
			return Indices.size();
		}

	// Operators:

		constexpr T& operator [] (size_t Index)
		{
			return Data[Indices[Index]];
		}

		constexpr const T& operator [] (size_t Index) const
		{
			return Data[Indices[Index]];
		}

	private:

	// Private Functions:

		size_t GetAwailableIndex()
		{
			size_t Index = SIZE_MAX;

			if (FreeIndices.empty())
			{
				Index = Data.size();

				Indices.push_back(Index);

				Data.push_back(T());

				return Index;
			}

			Index = FreeIndices.front();

			FreeIndices.front() = FreeIndices.back();

			FreeIndices.pop_back();

			return Index;
		}

	// Variables:

		std::vector<size_t> Indices;
		std::vector<size_t> FreeIndices;
		std::vector<T>      Data;
	};

} // namespace t3d