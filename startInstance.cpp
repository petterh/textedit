/*
 * $Header: /Book/startInstance.cpp 6     6-09-01 12:54 Oslph312 $
 */

#include "precomp.h"
#include "startInstance.h"
#include "formatMessage.h"
#include "Exception.h"
#include "utils.h"

/**
 * Starts a new instance of TextEdit with the given
 * parameter list:
 */ 
bool startInstance( const String &strArguments, int nShow ) {

   const String strProgram = getModuleFileName();
   const String strCommandLine = formatMessage( 
      _T( "\"%1\" %2" ), strProgram.c_str(), strArguments.c_str() );

   STARTUPINFO su_info = { sizeof su_info };
   GetStartupInfo( &su_info );
   su_info.lpTitle = 0;
   su_info.dwFlags = STARTF_USESHOWWINDOW;
   su_info.wShowWindow = LOWORD( nShow );
   PROCESS_INFORMATION p_info = { 0 };

   const BOOL bOK = CreateProcess( 0, 
      const_cast< LPTSTR >( strCommandLine.c_str() ),
      0, 0, FALSE, CREATE_DEFAULT_ERROR_MODE,
      0, 0, &su_info, &p_info );

   assert( bOK );
   if ( bOK ) {
      CloseHandle( p_info.hThread  );
      CloseHandle( p_info.hProcess );
   } else {
      trace( _T( "startInstance: Unable to start TextEdit: %s\n" ),
         getError().c_str() );
   }
   return FALSE != bOK;
}

// end of file

