#include "FMultiCommand.h"

namespace cmd
{
	void FMultiCommand::Execute()
	{
		for (auto& Command : Commands)
		{
			Command->Execute();
		}
	}

	void FMultiCommand::Revert()
	{
		for (auto& Command : Commands)
		{
			Command->Revert();
		}
	}

}