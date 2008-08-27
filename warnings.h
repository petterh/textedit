/*
 * $Header: /Book/warnings.h 6     5.09.99 13:07 Oslph312 $
 * 
 * Compiler-specific header file that removes bothersome warnings
 * emanating from system header files.
 */

#pragma once

#pragma warning( disable: 4201 )
// nonstandard extension used : nameless struct/union
// unreferenced inline/local function has been removed : 4514 

#pragma warning( disable: 4663 )
// C++ language change: to explicitly specialize class template 
//    'char_traits' use the following syntax:
// template<> struct char_traits<unsigned short> ...

#pragma warning( disable: 4511 )
// 'codecvt_base' : copy constructor could not be generated

#pragma warning( disable: 4512 )
// 'codecvt_base' : assignment operator could not be generated

#pragma warning( disable: 4244 )
// 'return' : conversion from 'const int' to 'char', 
//    possible loss of data

#pragma warning( disable: 4100 )
// '_P' : unreferenced formal parameter

#pragma warning( disable: 4018 )
// '<' : signed/unsigned mismatch

#pragma warning( disable: 4290 )
// C++ Exception Specification ignored

#pragma warning( disable: 4710 )
// function  not inlined

#pragma warning( disable: 4786 )
// identifier was truncated to '255' characters in the 
//    debug information

#ifdef NDEBUG
#pragma warning( disable: 4702 )
// unreachable code
#endif

// end of file
