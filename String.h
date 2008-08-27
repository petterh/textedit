/*
 * $Header: /Book/String.h 9     17.04.00 11:17 Oslph312 $
 *
 * Wraps the string type from the standard C++ library.
 */

#pragma once

#define _STRING_DEFINED_

typedef std::wstring StringW;
typedef std::string  StringA;

#ifdef _UNICODE

typedef StringW String;

#else

typedef StringA String;

#endif

// end of file
