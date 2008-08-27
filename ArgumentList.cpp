/*
 * $Header: /Book/ArgumentList.cpp 9     20.08.99 16:33 Oslph312 $
 *
 * Implements command line handling.
 */

#include "precomp.h"
#include "ArgumentList.h"

ArgumentList::ArgumentList( LPCTSTR /* pszCmdLine */ ) {

   assert( isGoodPtr( this ) );
   assert( isGoodReadPtr( &__argc, sizeof __argc ) );
   assert( isGoodReadPtr( &__targv, sizeof __targv ) );
   assert( 
      isGoodReadPtr( __targv, __argc * sizeof( __targv[ 0 ] ) ) );

   m_argc = __argc;

   // Using the system-supplied array appears to work OK,
   // but I am wary of messing with parameters that really
   // should have been const in the first place.
   // Hence the semi-deep copy. Since the strings themselves are
   // const, a deep copy is not necessary.

#if 0
   m_argv = __targv;
   assert( 0 == m_argv[ m_argc ] );
#else
   m_argv.alloc( m_argc + 1 );
   for ( int iArg = 0; iArg < m_argc; ++iArg ) {
      m_argv[ iArg ] = __targv[ iArg ];
   }
   assert( 0 == __targv[ m_argc ] );
   m_argv[ m_argc ] = 0;
#endif
}


bool ArgumentList::hasOption( LPCTSTR pszOption ) {

   assert( isValid() );

   bool bOption = false;
   for ( int iArg = 1; iArg < m_argc; ++iArg ) {
      if ( isOption( iArg ) && 
         0 == _tcsicmp( pszOption, getArg( iArg ) + 1 ) ) 
      {
         bOption = true;
         consume( iArg-- );
      }
   }
   return bOption;
}


void ArgumentList::consume( int nArg ) {

   assert( isValid() );
   assert( 0 <= nArg && nArg < m_argc );

   for ( int iArg = nArg; iArg < m_argc; ++iArg ) {
      m_argv[ iArg ] = m_argv[ iArg + 1 ];
   }

   --m_argc;
   assert( 0 == m_argv[ m_argc ] );
}


#ifdef _DEBUG

bool ArgumentList::isValid( void ) const {
   
   bool bValid = isGoodConstPtr( this ) && 0 < m_argc &&
      isGoodReadPtr( m_argv, (m_argc + 1) * sizeof( *m_argv ) );
   if ( bValid ) {
      for ( int iArg = 0; iArg < m_argc; ++iArg ) {
         if ( !isGoodStringPtr( m_argv[ iArg ] ) ) {
            bValid = false;
            break;
         }
      }
      assert( 0 == m_argv[ m_argc ] );
   }
   return bValid;
}

#endif // _DEBUG

// end of file
