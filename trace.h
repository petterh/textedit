/*
 * $Header: /Book/trace.h 11    16.07.04 10:42 Oslph312 $
 */

#pragma once

#ifdef _DEBUG

   extern void __cdecl traceDebug( LPCTSTR pszFmt, ... );
   #define trace traceDebug

#ifdef _STRING_DEFINED_
   String _stringFromAtom( LPCTSTR pszStringOrAtom );
   #define stringFromAtom( X ) _stringFromAtom( X ).c_str()
#endif

#else

   #define trace (void)
   #define stringFromAtom( pszStringOrAtom ) 0

#endif

// end of file
