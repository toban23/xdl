
#include <iostream>
#include "stdtools.h"

void DEBUG(const char* x)
{
#ifdef _DEBUG
	std::cerr << x << std::endl;
#endif
}
