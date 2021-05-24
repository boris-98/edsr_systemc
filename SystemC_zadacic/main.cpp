#include <iostream>
#include "Cache.hpp"
#include "Generator.hpp"

using namespace std;

int sc_main(int argc ,char* argv[])
{
	Generator gen("Generator");
	sc_core::sc_start();

	return 0;
}
