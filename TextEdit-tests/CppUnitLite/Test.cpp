#include "precomp.h"
#include "CppUnitLite.h"


Test::Test (const String& testName) 
	: name_ (testName) 
{
	TestRegistry::addTest (this);
}


Test *Test::getNext() const
{
	return next_;
}


void Test::setNext(Test *test)
{	
	next_ = test;
}

bool Test::check(long expected, long actual, TestResult& result, const String& fileName, long lineNumber)
{
	if (expected == actual)
		return true;
	result.addFailure (
		Failure (
			name_, 
			StringFrom (_T(__FILE__)), 
			__LINE__, 
			StringFrom (expected), 
			StringFrom (actual)));

	return false;

}


bool Test::check(const String& expected, const String& actual, TestResult& result, const String& fileName, long lineNumber)
{
	if (expected == actual)
		return true;
	result.addFailure (
		Failure (
			name_, 
			StringFrom (_T(__FILE__)), 
			__LINE__, 
			expected, 
			actual));

	return false;

}

String StringFrom (bool value) {
	return value ? _T("true") : _T("false");
}

String StringFrom (const TCHAR *value) {
	return value;
}

static const int DEFAULT_SIZE = 20;

String StringFrom (long value) {
	TCHAR buffer [DEFAULT_SIZE];
	_stprintf_s(buffer, _T("%ld"), value);
	return buffer;
}

String StringFrom (double value) {
	TCHAR buffer [DEFAULT_SIZE];
	_stprintf_s(buffer, _T("%lf"), value);
	return buffer;
}

String StringFrom (const String& other) {
	return other;
}
