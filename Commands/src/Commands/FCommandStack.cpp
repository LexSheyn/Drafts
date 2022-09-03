#include "FCommandStack.h"

namespace cmd
{
// Functions:

	void FCommandStack::ExecuteCommand(ICommand* Command)
	{
		Command->Execute();

		UndoStack.push_back(Command);
		
		this->ClearInternalCommandStack(RedoStack);
	}

	void FCommandStack::Undo()
	{
		if (UndoStack.empty())
		{
			return;
		}

		ICommand* Command = UndoStack.back();

		UndoStack.pop_back();

		Command->Revert();

		RedoStack.push_back(Command);
	}

	void FCommandStack::Redo()
	{
		if (RedoStack.empty())
		{
			return;
		}

		ICommand* Command = RedoStack.back();

		RedoStack.pop_back();

		Command->Execute();

		UndoStack.push_back(Command);
	}

	void FCommandStack::Clear()
	{
		this->ClearInternalCommandStack(UndoStack);
		this->ClearInternalCommandStack(RedoStack);
	}

	bool FCommandStack::IsUndoStackEmpty() const
	{
		return UndoStack.empty();
	}

	bool FCommandStack::IsRedoStackEmpty() const
	{
		return RedoStack.empty();
	}


// Accessors:

	size_t FCommandStack::GetUndoCount() const
	{
		return UndoStack.size();
	}

	size_t FCommandStack::GetRedoCount() const
	{
		return RedoStack.size();
	}


// Private Functions:

	void FCommandStack::ClearInternalCommandStack(std::vector<ICommand*>& CommandStack)
	{
		for (auto Command : CommandStack)
		{
			delete Command;
		}

		CommandStack.clear();
	}

}