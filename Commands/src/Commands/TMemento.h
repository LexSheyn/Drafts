#pragma once

namespace cmd
{
	template<typename T>
	class TMemento
	{
	public:

		TMemento(T Memento)
			: Memento (Memento)
		{}

		const T& Load() const
		{
			return Memento;
		}

	private:

		T Memento;
	};
}