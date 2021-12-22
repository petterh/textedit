#pragma once

#include "Failure.h"

class TestResult
{
public:
					TestResult (); 
	virtual void	testsStarted ();
	virtual void	addFailure (const Failure& failure);
	virtual void	testsEnded ();

private:
	int				failureCount;

public:
	inline bool getSuccess() { return failureCount == 0; }
};
