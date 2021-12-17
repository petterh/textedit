/*
 * $Header: /Book/search.cpp 5     20.08.99 16:33 Oslph312 $
 */

#include "precomp.h"
#include "search.h"
#include "utils.h"


// A possible optimization would be to create a copy of the text 
// and convert this to lower-case, then do case-insensitive compare.
PRIVATE LPTSTR findI( LPCTSTR pszSource, LPCTSTR pszSearch ) {

   int nSrcLength = _tcslen( pszSource );
   const int nSearchLength = _tcslen( pszSearch );

   while ( nSearchLength <= nSrcLength-- ) {
      if ( 0 == _tcsncicmp( pszSource, pszSearch, nSearchLength ) ) {
         return const_cast< LPTSTR >( pszSource );
      }
      pszSource = charNext( pszSource );
   }
   return 0;
}


PRIVATE bool isWordMatch( 
   LPCTSTR pszText, LPCTSTR pszMatch, int nLength ) 
{
   if ( 0 == pszMatch ) {
      return false;
   }
   if ( !_istalnum( *pszMatch ) ) {
      return false;
   }
   if ( pszText < pszMatch ) {
      const LPCTSTR pszPrev = charPrev( pszText, pszMatch );
      if ( _istalnum( *pszPrev ) ) {
         return false;
      }
   }
   const LPCTSTR pszLast = pszMatch + nLength - 1;
   if ( !_istalnum( *pszLast ) ) {
      return false;
   }
   const LPCTSTR pszNext = pszLast + 1;
   if ( _istalnum( *pszNext ) ) {
      return false;
   }

   return true;
}


LPCTSTR find( 
   LPCTSTR pszText, LPCTSTR pszStart, LPCTSTR pszSearch, 
   bool bMatchWholeWord, bool bMatchCase ) 
{
   LPCTSTR pszMatch = 0;
   for ( ;; ) {
      pszMatch = bMatchCase 
         ? _tcsstr( pszStart, pszSearch )
         : findI  ( pszStart, pszSearch );
      if ( bMatchWholeWord && 0 != pszMatch ) {
         if ( isWordMatch( 
            pszText, pszMatch, _tcslen( pszSearch ) ) )
         {
            break; //*** LOOP EXIT POINT
         }
      } else {
         break; //*** LOOP EXIT POINT
      }
      pszStart = pszMatch + 1;
   }
   return pszMatch;
}


LPCTSTR findBackwards( 
   LPCTSTR pszText, LPCTSTR pszLast, LPCTSTR pszSearch, 
   bool bMatchWholeWord, bool bMatchCase ) 
{
   const int nLength = _tcslen( pszSearch );
   while ( pszText <= pszLast ) {
      const int nResult = (bMatchCase ? _tcsncmp : _tcsnicmp)( 
         pszLast, pszSearch, nLength );
      if ( 0 == nResult ) {
         if ( !bMatchWholeWord || 
            isWordMatch( pszText, pszLast, nLength ) )
         {
            return pszLast; //*** FUNCTION EXIT POINT
         }
      }
      LPCTSTR pszNewLast = charPrev( pszText, pszLast );
      if ( pszLast <= pszNewLast ) {
         break; //*** LOOP EXIT POINT
      }
      pszLast = pszNewLast;
   }
   return 0;
}

// end of file
