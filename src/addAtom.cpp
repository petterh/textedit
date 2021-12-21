/*
 * $Header: /Book/addAtom.cpp 3     3.07.99 17:46 Oslph312 $
 *
 * This module provides improved error reporting from GlobalAddAtom.
 * This function fails when the global atom table fills up, but
 * the error reporting is buggy. So is the global atom table itself;
 * when it fills up, it stays full until a system reboot.
 *
 * This module owes its existence to Alf Steinbach.
 */

#include "precomp.h"
#include "Exception.h"
#include "AutoArray.h"
#include "addAtom.h"

ATOM globalAddAtom( const LPCTSTR pszAtomName )
   throw( MemoryException ) 
{
   assert( isGoodStringPtrOrAtom( pszAtomName ) );

   SetLastError( NOERROR );
   ATOM atom = GlobalAddAtom( pszAtomName );
   if ( ERROR_SUCCESS != GetLastError() ) {
      atom = 0;
   } else if ( 0 != atom && 0 != HIWORD( pszAtomName ) ) {
      const int nLength = _tcslen( pszAtomName );
      AutoString pszStoredName( new TCHAR[ nLength + 1 ] );
      pszStoredName[ 0 ] = 0;
      GlobalGetAtomName( atom, pszStoredName, nLength + 1 );
      if ( 0 != _tcsicmp( pszAtomName, pszStoredName ) ) {
         atom = 0;
      }
   }
   return atom;
}

// end of file
