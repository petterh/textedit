
#include "precomp.h"
#include "Failure.h"

#include <stdio.h>
#include <string.h> 

Failure::Failure (const String&	theTestName, 
				  const String&	theFileName, 
		          long	theLineNumber,
		          const String&	theCondition) 
: message (theCondition), 
  testName (theTestName), 
  fileName (theFileName), 
  lineNumber (theLineNumber)
{
}

Failure::Failure (const String&	theTestName, 
			 	  const String&	theFileName, 
				  long	theLineNumber,
				  const String&	expected,
				  const String&	actual) 
: testName (theTestName), 
  fileName (theFileName), 
  lineNumber (theLineNumber)
{
	message = _T("Expected ") + expected + _T(" but was ") + actual;
}
