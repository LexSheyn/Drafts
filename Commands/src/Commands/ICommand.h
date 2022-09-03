#pragma once

#include "TMemento.h"

namespace cmd
{
	class ICommand
	{
	protected:

		ICommand () = default;

	public:

		virtual ~ICommand () = default;

		virtual void Execute () = 0;
		virtual void Revert  () = 0;
	};
}