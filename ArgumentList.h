/*
 * $Header: /Book/ArgumentList.h 12    5.09.99 13:06 Oslph312 $
 * 
 * Command-line parsing.
 *
 * The command line parameter to the ArgumentList constructor is
 * unused; it's left in for the benefit of implementations that
 * don't have access to argc/argv and need to do their own parsing.
 */

#pragma once

#include "String.h"
#include "AutoArray.h"


class ArgumentList {
private:
   int     m_argc;
   AutoArray< LPCTSTR > m_argv;

#ifdef _DEBUG
   bool isValid( void ) const;
#endif // _DEBUG

public:
   ArgumentList( LPCTSTR /* pszCmdLine */ );

   int getNumArgs( void ) const;
   LPCTSTR getArg( int nArg ) const;
   LPCTSTR getArg( int nArg, bool bConsume = false );
   bool isOption( int nArg ) const; // ??
   bool hasOption( LPCTSTR pszOption );
   void consume( int nArg );
};


inline int ArgumentList::getNumArgs( void ) const {
   
   assert( isValid() );
   return m_argc;
}


inline LPCTSTR ArgumentList::getArg( int nArg ) const {
   
   assert( isValid() );
   assert( 0 <= nArg && nArg < m_argc );
   return m_argv[ nArg ];
}


inline LPCTSTR ArgumentList::getArg( int nArg, bool bConsume ) {
   
   assert( isValid() );
   assert( 0 <= nArg && nArg < m_argc );
   const LPCTSTR pszArgument = m_argv[ nArg ];
   if ( bConsume ) {
      consume( nArg );
   }
   return pszArgument;
}


/**
 * An argument is considered an option 
 * if preceeded by a slash or a minus.
 * The point of call can choose to consider unrecognized 
 * options to be file names. TextEdit does this.
 */
inline bool ArgumentList::isOption( int nArg ) const {
   
   assert( isValid() );
   assert( 0 <= nArg && nArg < m_argc );

   LPCTSTR const pszArg = getArg( nArg );
   return _T( '/' ) == *pszArg || _T( '-' ) == *pszArg;
}

// end of file
