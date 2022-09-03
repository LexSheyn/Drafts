#pragma once

#include "FDelegate.h"

namespace mvc
{
	class FModel
	{
	public:

		FModel()
		{}

		FModel(const std::string& Data)
			: Data (Data)
		{}

		std::string GetData() const
		{
			return Data;
		}

		void SetData(const std::string& Data)
		{
			this->Data = Data;
		}

	private:

		std::string Data;
	};
}