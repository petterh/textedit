#include "precomp.h"
#include "CppUnitLite\CppUnitLite.h"

int _tmain()
{
	TestResult tr;
    TestRegistry::runAllTests(tr);

	return 0;
}
