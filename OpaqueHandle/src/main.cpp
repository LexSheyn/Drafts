#include <iostream>

#include "FHandle.h"

int32_t main(int32_t ArgC, const char* ArgV[])
{
	FHandle Handle = nullptr;

	bool Result = CreateHandle(&Handle);
	Result = PrintHandle(Handle);

	return 0;
}