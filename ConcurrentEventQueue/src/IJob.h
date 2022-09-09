#pragma once

#include <memory>

namespace t3d
{
	class IJob
	{
	public:

	// Constructors and Destructor:

		         IJob () = default;
		virtual ~IJob () = default;

	// Interface:

		virtual void Execute () = 0;
	};

	using Job_T = std::unique_ptr<IJob>;
}