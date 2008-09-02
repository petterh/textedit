/*
 * $Header: /Book/WinMain.cpp 20    5.03.02 10:05 Oslph312 $
 */

#include "precomp.h"
#include "resource.h"
#include "init.h"
#include "handlers.h"
#include "Exception.h"
#include "ComSupport.h"
#include "language.h"
#include "mainwnd.h"
#include "winUtils.h"
#include "persistence.h"
#include "os.h"
#include "combobug.h"


int WINAPI _tWinMain( 
   HINSTANCE hinst, HINSTANCE, LPTSTR pszCmdLine, int nShow )
{
// MessageBox( HWND_DESKTOP, pszCmdLine, "Hi from TextEdit", MB_OK );
   trace( _T( "TextEdit _tWinMain( \"%s\" )\n" ), pszCmdLine );

   fixComboBug();

#if defined( _DEBUG ) && defined( _CRTDBG_REPORT_FLAG )
   {
      int nDebugFlags = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
      nDebugFlags |= _CRTDBG_LEAK_CHECK_DF;
      nDebugFlags |= _CRTDBG_CHECK_ALWAYS_DF;
      _CrtSetDbgFlag( nDebugFlags );
   }
#if 0 // Test if leaks are detected. 
      // (Leaks are reported in the debug trace.)
   TCHAR *testLeak = new TCHAR[ 100 ];
#endif
#endif

   setNewLanguage( getLanguage() );

   int nRetCode = -1;
   ThreadErrorHandling teh;

   try {
      ComSupport cs;
      AutoEditor pEditor;
      // In case of exception thrown during init.
      pEditor = init( pszCmdLine, nShow );
      assert( isGoodPtr( (Editor *) pEditor ) );
      nRetCode = pEditor->run();
   }
   catch ( const CancelException& ) {
      ; // OK, just terminate.
   }
   catch ( const AccessDeniedException& x ) {
      messageBox( HWND_DESKTOP, MB_ICONERROR | MB_OK, IDS_ACCESS_DENIED, x.what() );
   }
   catch ( const ComException& x ) {
      // In case the ComSupport constructor fails
      messageBox( HWND_DESKTOP, MB_ICONERROR | MB_OK, IDS_COM_INIT_ERROR, x.what() );
   }
   catch ( const Exception& x ) {
      // We get here in case of accidents during init.
      messageBox( HWND_DESKTOP, MB_ICONERROR | MB_OK, IDS_INIT_ERROR, x.what() );
   }
   catch ( ... ) {
      // We get here in case of SEHs that we don't translate.
      assert( false );
      messageBox( HWND_DESKTOP, MB_ICONERROR | MB_OK, IDS_FATAL_ERROR );
   }

   return nRetCode; // Goodbye, and thanks for all the fish.
}

// end of file
