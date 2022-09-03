#pragma once

#include "ICommand.h"

#include <vector>

namespace cmd
{
	class FCommandStack
	{
	public:

	// Functions:

		void ExecuteCommand   (ICommand* Command);
		void Undo             ();
		void Redo             ();
		void Clear            ();
		bool IsUndoStackEmpty () const;
		bool IsRedoStackEmpty () const;

	// Accessors:

		size_t GetUndoCount () const;
		size_t GetRedoCount () const;

	private:

	// Private Functions:

		void ClearInternalCommandStack (std::vector<ICommand*>& CommandStack);

	// Variables:

		std::vector<ICommand*> UndoStack;
		std::vector<ICommand*> RedoStack;
	};
}