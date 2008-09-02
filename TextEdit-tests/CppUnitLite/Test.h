#pragma once

#include <cmath>
#include "../../string.h"

class TestResult;


class Test
{
public:
	Test (const String& testName);

	virtual void	run (TestResult& result) = 0;


	void			setNext(Test *test);
	Test			*getNext () const;

protected:

	bool check (long expected, long actual, TestResult& result, const String& fileName, long lineNumber);
	bool check (const String& expected, const String& actual, TestResult& result, const String& fileName, long lineNumber);

	String name_;
	Test   *next_;

};

String StringFrom (bool value);
String StringFrom (const TCHAR *value);
String StringFrom (long value);
String StringFrom (double value);
String StringFrom (const String& other);

#define TEST(testName, testGroup)\
  class testGroup##testName##Test : public Test \
	{ public: testGroup##testName##Test () : Test(_T(#testName) _T("Test")) {} \
            void run (TestResult& result_); } \
    testGroup##testName##Instance; \
	void testGroup##testName##Test::run (TestResult& result_) 



#define CHECK(condition)\
{ if (!(condition)) \
{ result_.addFailure (Failure (name_, _T(__FILE__),__LINE__, _T(#condition))); return; } }



#define CHECK_EQUAL(expected,actual)\
{ if (StringFrom(expected) != StringFrom(actual)) \
{ result_.addFailure(Failure(name_, _T(__FILE__), __LINE__, StringFrom(expected), StringFrom(actual))); return; } }


#define LONGS_EQUAL(expected,actual)\
{ long actualTemp = actual; \
  long expectedTemp = expected; \
  if ((expectedTemp) != (actualTemp)) \
{ result_.addFailure (Failure (name_, __FILE__, __LINE__, StringFrom(expectedTemp), \
StringFrom(actualTemp))); return; } }



#define DOUBLES_EQUAL(expected,actual,threshold)\
{ double actualTemp = actual; \
  double expectedTemp = expected; \
  if (fabs ((expectedTemp)-(actualTemp)) > threshold) \
{ result_.addFailure (Failure (name_, __FILE__, __LINE__, \
StringFrom((double)expectedTemp), StringFrom((double)actualTemp))); return; } }



#define FAIL(text) \
{ result_.addFailure (Failure (name_, __FILE__, __LINE__,(text))); return; }
