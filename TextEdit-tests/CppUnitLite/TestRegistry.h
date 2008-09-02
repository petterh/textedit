///////////////////////////////////////////////////////////////////////////////
// 
// TestRegistry is a singleton collection of all the tests to run in a system.  
// 
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Test.h"
#include "TestResult.h"

class TestRegistry
{
private:
	Test *tests;
	int	count;

public:
	static void addTest(Test *test);
	static void runAllTests(TestResult& result);
	static int getCount() { return instance().count; }

private:
	TestRegistry() : count(0), tests(0) {}

	static TestRegistry& instance();
	void add(Test *test);
	void run(TestResult& result);
};
