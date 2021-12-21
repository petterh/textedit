#pragma once

#include "../../src/string.h"

class Failure
{
public:
	Failure (const String&		theTestName, 
			 const String&		theFileName, 
			 long	  			theLineNumber,
			 const String&		theCondition);

	Failure (const String&		theTestName, 
			 const String&		theFileName, 
			 long				theLineNumber,
			 const String&		expected,
			 const String&		actual);

	String message;
	String testName;
	String fileName;
	long   lineNumber;
};
