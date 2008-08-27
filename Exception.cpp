/*
 * $Header: /Book/Exception.cpp 18    17.12.02 9:40 Oslph312 $
 */

#include "precomp.h"
#include "Exception.h"
#include "formatMessage.h"


#if INTERCEPT_SEH

String sehException::getDescr( void ) const {

   return _T( "Unknown System Exception" );
};


String DivideByZeroException::getDescr( void ) const {

   return _T( "Division by zero" );
};


String StackOverflowException::getDescr( void ) const {

   return _T( "Stack overflow" );
};


String InvalidHandleException::getDescr( void ) const {

   return _T( "Invalid handle" );
};


String AccessViolationException::getDescr( void ) const {

   debugBreak();
   return _T( "Access violation" );
};


static StackOverflowException theStackOverflowException;

StackOverflowException& 
StackOverflowException::getStackOverflowException( void ) {

   return theStackOverflowException;
}

#endif // INTERCEPT_SEH


String getError( const String& strDescr, DWORD dwErr ) {
   
   String strError( strDescr );
   if ( !strDescr.empty() ) {
      strError += _T( "\n" );
   }

   LPTSTR pszMsgBuf = 0;
   
#ifdef _DEBUG
   const DWORD dwLen = 
#endif
      
      FormatMessage ( 
         FORMAT_MESSAGE_FROM_SYSTEM     | 
         FORMAT_MESSAGE_ALLOCATE_BUFFER | 
         FORMAT_MESSAGE_IGNORE_INSERTS  ,
         0, dwErr,
         // LANGIDFROMLCID( GetThreadLocale() ),
         MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
         reinterpret_cast< LPTSTR >( &pszMsgBuf ),
         0, 0 );
   
   if ( 0 == pszMsgBuf ) {
      pszMsgBuf = reinterpret_cast< LPTSTR >( 
         LocalAlloc( LPTR, 50 * sizeof( TCHAR ) ) );
      assert( 0 != pszMsgBuf );
      wsprintf( pszMsgBuf, _T( "Unknown API error (%lu)" ), dwErr );
   }
   
   assert( 0 == dwLen || _tcsclen( pszMsgBuf ) == dwLen);
   strError += pszMsgBuf;
   LocalFree( pszMsgBuf );
   reset_pointer( pszMsgBuf );
   
#ifdef _DEBUG
   strError += formatMessage( _T( " [%1!u!]" ), dwErr );
#endif

   return strError;
}


WinException::WinException( const String& strDescr, DWORD dwErr )
   : m_dwErr( dwErr )
   , m_strDescr( strDescr )
{
}


WinException::WinException( DWORD dwErr )
   : m_dwErr( dwErr )
   , m_strDescr( _T( "" ) )
{
}


String WinException::getDescr( void ) const {

   return getError( m_strDescr, m_dwErr );
}


String ComException::getDescr( void ) const {

   switch ( m_dwErr ) {
   case S_OK: 
      return _T( "Success." );

   case S_FALSE:
      return _T( "The COM library is already initialized." );

   case REGDB_E_CLASSNOTREG:
      return
         _T( "A specified class is not registered in the " )
         _T( "registration database. Also can indicate that the " )
         _T( "type of server you requested in the CLSCTX " )
         _T( "enumeration is not registered or the values for " )
         _T( "the server types in the registry are corrupt." );

   case CLASS_E_NOAGGREGATION:
      return 
         _T( "This class can't be created as part of an aggregate." );
   }

   return getError( _T( "Com Error" ), m_dwErr );
}


String SubclassException::getDescr( void ) const {

   String strDescr( 
      _T( "Internal error: Window subclassing failed\n\n" ) );
   strDescr += WinException::getDescr();
   return strDescr;
}


static MemoryException theMemoryException; // ERROR_OUTOFMEMORY

MemoryException& MemoryException::getMemoryException( void ) {

   return theMemoryException;
}


String MemoryException::getDescr( void ) const {

   return _T( "Out of memory" );
}


CommonDialogException::CommonDialogException( DWORD dwErr ) 
   : WinException( dwErr )
{
}


String CommonDialogException::getDescr( void ) const {

   static struct {
      DWORD   dwErr;
      LPCTSTR pszErr;
   } const aErrorTable[] = {
      { CDERR_FINDRESFAILURE , _T( "Unable to find resource"   ) },
      { CDERR_NOHINSTANCE    , _T( "No HINSTANCE"              ) },
      { CDERR_INITIALIZATION , _T( "Initialization error"      ) },
      { CDERR_NOHOOK         , _T( "No hook"                   ) },
      { CDERR_LOCKRESFAILURE , _T( "Unable to lock resource"   ) },
      { CDERR_NOTEMPLATE     , _T( "No template"               ) },
      { CDERR_LOADRESFAILURE , _T( "Unable to load resource"   ) },
      { CDERR_STRUCTSIZE     , _T( "Wrong structure size"      ) },
      { CDERR_LOADSTRFAILURE , _T( "Unable to load string"     ) },
      { FNERR_BUFFERTOOSMALL , _T( "Buffer too small"          ) },
      { CDERR_MEMALLOCFAILURE, _T( "Memory allocation failure" ) },
      { FNERR_INVALIDFILENAME, _T( "Invalid file name"         ) },
      { CDERR_MEMLOCKFAILURE , _T( "Unable to lock memory"     ) },
      { FNERR_SUBCLASSFAILURE, _T( "Unable to subclass"        ) },
   };

   for ( int iErr = 0; iErr < dim( aErrorTable ); ++iErr ) {
      if ( aErrorTable[ iErr ].dwErr == m_dwErr ) {
         return aErrorTable[ iErr ].pszErr; //*** METHOD EXIT POINT
      }
   }

   return formatMessage( 
      _T( "Unknown common dialog error %1!lu!" ), m_dwErr );
}


void throwException( const String& strDescr, DWORD dwErr ) {

   if ( ERROR_NOT_ENOUGH_MEMORY == dwErr || 
        ERROR_OUTOFMEMORY == dwErr ) 
   {
      throw MemoryException::getMemoryException();
   }

   switch ( dwErr ) {
   case ERROR_FILE_NOT_FOUND: throw FileNotFoundException( strDescr );
   case ERROR_PATH_NOT_FOUND: throw PathNotFoundException( strDescr );
   case ERROR_ACCESS_DENIED : throw AccessDeniedException( strDescr );
#if 0
   case ERROR_SHARING_VIOLATION: 
                          throw SharingViolationException( strDescr );
#endif
   }

   throw WinException( strDescr, dwErr );
}

// end of file
