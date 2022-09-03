#include "pch.h"
#include "Commands/FCommandStack.h"

class FClass
{
public:

	int32_t Value;
};

namespace cmd
{
	class FDoStuffCommand : public ICommand
	{
	public:

		explicit FDoStuffCommand(FClass* Class)
			: Class   (Class)
			, Memento (*Class)
		{}

		void Execute()
		{
			++Class->Value;
		}

		void Revert()
		{
			*Class = Memento.Load();
		}

	private:

		FClass*          Class;
		TMemento<FClass> Memento;
	};
}

int32_t main(int32_t ArgC, char* ArgV[])
{
	cmd::FCommandStack CommandStack;

	FClass Class;

	Class.Value = 100;

	std::cout << "Value: " << Class.Value << std::endl;

	CommandStack.ExecuteCommand(new cmd::FDoStuffCommand(&Class)); // Executed 100 becomes 101

	std::cout << "Value: " << Class.Value << std::endl;

	CommandStack.Redo(); // Nothing 101

	std::cout << "Value: " << Class.Value << std::endl;

	CommandStack.Undo(); // Undone 101 back to 100

	std::cout << "Value: " << Class.Value << std::endl;

	CommandStack.Undo(); // Nothing 100

	std::cout << "Value: " << Class.Value << std::endl;

	CommandStack.Redo(); // Redone 100 becomes 101

	std::cout << "Value: " << Class.Value << std::endl;

	CommandStack.Undo(); // Undone 101 back to 100

	std::cout << "Value: " << Class.Value << std::endl;

	return 0;
}