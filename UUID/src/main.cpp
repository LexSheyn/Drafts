#include <iostream>

int32_t main(int32_t ArgC, char* ArgV[])
{
	double A = 1.0;
	double B = -0.0;
	double C = 0.0;
	double D = A * B;
	double E = A * C;

	std::cout << "D: " << D << std::endl;
	std::cout << "E: " << E << std::endl;

	return 0;
}