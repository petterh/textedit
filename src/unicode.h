/*
 * $Header: /Book/unicode.h 2     5.03.02 10:06 Oslph312 $
 * 
 * Ensures UNICODE/_UNICODE compatibility.
 */

#pragma once

#ifdef UNICODE
#undef _UNICODE
#define _UNICODE
#endif

#ifdef _UNICODE
#undef  UNICODE
#define UNICODE
#endif

// end of file
