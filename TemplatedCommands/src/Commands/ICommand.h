#pragma once

namespace cmd
{
	class ICommand
	{
	public:

		         ICommand () = default;
		virtual ~ICommand () = default;

		virtual void Undo () = 0;
		virtual void Redo () = 0;
	};
}