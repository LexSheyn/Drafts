#pragma once

#include "Events.h"

namespace mvc
{
	class FView
	{
	public:

		FView()
		{}

		void Render(std::string Data)
		{
			std::cout << Data << std::endl;
		}
	};
}