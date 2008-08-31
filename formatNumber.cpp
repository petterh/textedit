/*
 * $Header: /Cleaner/formatNumber.cpp 10    22.03.02 12:52 Oslph312 $
 * 
 * Defines the formatNumber function,
 */

#include "precomp.h"
#include "formatNumber.h"
#include "utils.h"
#include "trace.h"


/**
 * Does not support negative numbers.
 * If you need a more efficient implementation,
 * perform the GetLocaleInfo and initialization
 * only on startup and locale changes.
 * If you don't need a more efficient implementation,
 * this way is better, because it is simpler.
 * See also: GetNumberFormat
 */
// TODO: Unit test new safe string API
String formatNumber( int nValue ) {

   assert( 0 <= nValue );

   const int MAX_GROUPINGS = 10;

   // Win16:
   // GetProfileString( "intl", "sThousand", ",", szThousandSep, sizeof szThousandSep );

   TCHAR szThousandSep[ MAX_GROUPINGS ] = { 0 };
   verify( GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, szThousandSep, dim( szThousandSep ) ) );
   _tcsrev( szThousandSep ); // We will reverse the string later.

   TCHAR szGrouping[ 100 ] = { 0 };
   verify( GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_SGROUPING, szGrouping, dim( szGrouping ) ) );

   int anGrouping[ 10 ] = { 0 };
   int nGroupings = 0;
   TCHAR *psz;
   for ( psz = szGrouping; 0 != *psz && _T( '0' ) != *psz; psz = charNext( psz ) ) {
      if ( _istdigit( *psz ) ) {
         assert( nGroupings < dim( anGrouping ) );
         if ( dim( anGrouping ) <= nGroupings ) {
            trace( _T( "more than %d digit groupings; ignoring the rest\n" ), dim( anGrouping ) );
            break; //*** LOOP EXIT POINT
         }
         anGrouping[ nGroupings++ ] = *psz - _T( '0' );
      } else {
         assert( _T( ';' ) == *psz );
      }
   }

   if ( 0 == nGroupings ) {
      anGrouping[ 0 ] = 3;
      nGroupings = 1;
   }

#if 0 // Testing. TODO: Add to unit test
   nGroupings = 3;
   anGrouping[ 0 ] = 3;
   anGrouping[ 1 ] = 1;
   anGrouping[ 2 ] = 2;
#endif

#ifdef _DEBUG
   for ( int iGrouping = 0; iGrouping < nGroupings; ++iGrouping ) {
      assert( 0 != anGrouping[ iGrouping ] );
   }
#endif

   TCHAR szValue[ 100 ] = { 0 };
   psz = szValue;
   int nGroup = 0;
   int nDigitsInGroup = anGrouping[ nGroup ];

   for ( ;; ) {
      *psz++ = (TCHAR)(_T( '0' ) + nValue % 10);
      if ( (nValue /= 10) <= 0 ) {
         break; //*** LOOP EXIT POINT
      }
      if ( --nDigitsInGroup <= 0 ) {
		  const size_t charsLeft = szValue + dim( szValue ) - psz;
		  // This one is because _tcscpy_s fills the remainder of the string with a marker
		  // character (after the terminator). This code expects the whole thing to be full of terminators.
		  const size_t charactersToCopy = std::min( charsLeft, _tcslen( szThousandSep ) + 1 );
		  _tcscpy_s( psz, charactersToCopy, szThousandSep );
         psz += _tcsclen( psz );
         if ( nGroup < nGroupings - 1 ) {
            ++nGroup;
         }
         nDigitsInGroup = anGrouping[ nGroup ];
      }
   }

   // The string is now backwards, so switch:
   _tcsrev( szValue );
   return szValue;
}

// end of file
