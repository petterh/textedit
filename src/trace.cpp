/*
 * $Header: /Book/trace.cpp 18    16.07.04 10:42 Oslph312 $
 */

#include "precomp.h"
#include "String.h" // TODO -- questionable. Put stringFromAtom elsewhere!
#include "trace.h"
#include "AutoArray.h"

#ifdef _DEBUG

void __cdecl traceDebug( LPCTSTR pszFmt, ... ) {

   AutoString pszDebugString( new TCHAR[ 1000 ] );

   va_list vl;
   va_start( vl, pszFmt );
   wvsprintf( pszDebugString, pszFmt, vl );
   va_end( vl );

   OutputDebugString( pszDebugString );
}


#ifdef _STRING_DEFINED_

String _stringFromAtom( LPCTSTR pszStringOrAtom ) {
   
   if ( 0 != HIWORD( pszStringOrAtom ) ) {
      return pszStringOrAtom;
   }

   TCHAR szAtomName[ 100 ] = { 0 };
   GetAtomName( LOWORD( pszStringOrAtom ), 
      szAtomName, dim( szAtomName ) );
   return szAtomName;
}

#endif // _STRING_DEFINED_

#endif // _DEBUG

// end of file
