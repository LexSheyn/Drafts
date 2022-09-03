#pragma once

#include "FModel.h"
#include "Events.h"

namespace mvc
{
	class FController
	{
	public:

		FController()
		{}

		FController(FModel* Model)
			: Model (Model)
		{}

		void SetModel(FModel* Model)
		{
			this->Model = Model;
		}

		void ChangeData()
		{
			Model->SetData("New Data");

			Event::DataChanged.Invoke(Model->GetData());
		}

	private:

		FModel* Model;
	};
}