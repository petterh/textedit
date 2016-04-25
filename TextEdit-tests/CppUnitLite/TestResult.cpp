#include "..\..\precomp.h"
#include "TestResult.h"
#include "TestRegistry.h"

#include <stdio.h>

TestResult::TestResult()
	: failureCount(0)
{
}

void TestResult::testsStarted()
{
}

void TestResult::addFailure(const Failure& failure) 
{
	_ftprintf(stdout, _T("%s(%ld): %s\n"), failure.fileName.c_str(), failure.lineNumber, failure.message.c_str());
	failureCount++;
}

void TestResult::testsEnded() 
{
	switch (failureCount) {
	case 0:
		_ftprintf(stdout, _T("%d unit tests passed\n"), TestRegistry::getCount());
		break;
	case 1:
		_ftprintf(stdout, _T("One unit test failure\n"));
		break;
	default:
		_ftprintf(stdout, _T("%ld unit test failures\n"), failureCount);
		break;
	}
}
