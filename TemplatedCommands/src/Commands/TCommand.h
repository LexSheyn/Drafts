#pragma once

#include "ICommand.h"

namespace cmd
{
	template<typename Object_T>
	class TCommand : public ICommand
	{
	public:

		TCommand(Object_T* Object)
			: Object(Object)
		{}

		virtual void Undo () = 0;
		virtual void Redo () = 0;

	private:

		Object_T* Object;
	};
}