#include "..\src\precomp.h"
#include "CppUnitLite\CppUnitLite.h"

int _tmain()
{
    TestResult tr;
    TestRegistry::runAllTests(tr);

    return tr.getSuccess() ? 0 : 1;
}
