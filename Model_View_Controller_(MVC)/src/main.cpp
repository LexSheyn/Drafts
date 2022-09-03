#include <iostream>

#include "FModel.h"
#include "FView.h"
#include "FController.h"

int32_t main(int32_t ArgC, const char* ArgV[])
{
	mvc::FModel Model("Model");
	mvc::FView  View;

	mvc::Event::DataChanged.Subscribe(&View, &mvc::FView::Render);
	
	mvc::FController Controller(&Model);

	Controller.ChangeData();
	Controller.ChangeData();
	Controller.ChangeData();

	return 0;
}