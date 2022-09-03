#pragma once

#include "ICommand.h"

#include <utility>

namespace cmd
{
	template<typename Command_T>
	class TReversedCommand : public ICommand
	{
	public:

		template<typename... Args_T>
		TReversedCommand(Args_T&&... Args)
			: Command (std::forward<Args_T>(Args)...)
		{}

		virtual void Execute() override
		{
			Command.Revert();
		}

		virtual void Revert() override
		{
			Command.Execute();
		}

	private:

		Command_T Command;
	};
}