#pragma once

#include "ICommand.h"

#include <utility>
#include <vector>

namespace cmd
{
	class FMultiCommand : public ICommand
	{
	public:

		template<typename... Commands_T>
		FMultiCommand(Commands_T&&... Commands)
			: Commands { std::forward<Commands_T>(Commands)... }
		{}

		virtual void Execute () override;
		virtual void Revert  () override;

	private:

		std::vector<ICommand*> Commands;
	};
}