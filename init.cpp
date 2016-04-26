/*
 * $Header: /Book/init.cpp 22    6-09-01 12:32 Oslph312 $
 */

#include "precomp.h"
#include "init.h"
#include "resource.h"
#include "main_class.h"
#include "ArgumentList.h"
#include "Document.h"
#include "WaitCursor.h"
#include "Exception.h"
#include "Editor.h"
#include "MRU.h"
#include "Registry.h"
#include "VersionInfo.h"
#include "mainwnd.h"
#include "resolveName.h"
#include "formatMessage.h"
#include "fileUtils.h"
#include "printFile.h"
#include "winUtils.h"
#include "utils.h"
#include "setup.h"
#include "reboot.h"
#include "activateOldInstance.h"
#include "startInstance.h"
#include "createNewFile.h"

#ifdef _DEBUG
#include "AbstractEditWnd.h"
#endif


// Mark this code segment as discardable; we won't need after startup:
#if defined( _MSC_VER ) && (1020 < _MSC_VER)
#pragma code_seg( "INIT" )
#pragma comment( linker, "/section:INIT,ERD" )
#endif


PRIVATE String makeCommandLine( 
   bool bPrinting, const ArgumentList &argumentList ) 
{
   assert( 1 < argumentList.getNumArgs() );
   String strCommandLine;
   for ( int iArg = 1; iArg < argumentList.getNumArgs(); ++iArg ) {
      strCommandLine += formatMessage( 
         _T( " \"%1\"" ), argumentList.getArg( iArg ) );
   }
   if ( bPrinting ) {
      strCommandLine += _T( " /p" );
   }
   return strCommandLine;
}


PRIVATE String makeCommandLine( 
   bool bPrinting, const String &strFile ) 
{
   String strCommandLine = 
      formatMessage( _T( " \"%1\"" ), strFile.c_str() );
   if ( bPrinting ) {
      strCommandLine += _T( " /p" );
   }
   return strCommandLine;
}


PRIVATE bool isValidHandle( HANDLE hfile )  {
   
   if ( hfile == INVALID_HANDLE_VALUE ) {
      return false;
   }
   const DWORD dwType = GetFileType( hfile ) & ~FILE_TYPE_REMOTE;
   return FILE_TYPE_PIPE == dwType || FILE_TYPE_DISK == dwType;
}


PRIVATE void initCommonControls( void ) {

   // This has been tested with 5.0 to throw exception.
   // This test caught one bug in WinException::getDescr,
   // and another one in formatMessageV, which goes to
   // show the value of testing all code paths.
   const WORD  wMinimumVersionHi = 4;
   const WORD  wMinimumVersionLo = 70; // 71;
   const DWORD dwMinimumVersion = 
      MAKELONG( wMinimumVersionLo, wMinimumVersionHi );

   INITCOMMONCONTROLSEX init_cc_ex = { 
      sizeof init_cc_ex, ICC_WIN95_CLASSES,
   };
   if ( !InitCommonControlsEx( &init_cc_ex ) ) {
      throwException( _T( "InitCommonControlsEx failed" ) );
   }

   // Verify version of common controls:
   VersionInfo versionInfo( _T( "COMCTL32.DLL" ) );
   if ( !versionInfo.isValid() ) {
      throwException( _T( 
         "Unable to get version information for Common Controls" ) );
   }

   DWORD dwHigh = 0;
   if ( !versionInfo.getFileVersion( 0, &dwHigh ) ) {
      throwException( _T( 
         "Unable to determine version of Common Controls" ) );
   }

   if ( dwHigh < dwMinimumVersion ) {
      const String strError = formatMessage( 
         IDS_WRONG_COMCTL32_VERSION, 
         (UINT) wMinimumVersionHi, (UINT) wMinimumVersionLo, 
         (UINT) HIWORD( dwHigh  ), (UINT) LOWORD( dwHigh ) );
      throwException( strError, NOERROR );
   }
}


PRIVATE void initReboot( void ) {

   const String strProgram = getModuleFileName();
   Registry::setString( HKEY_CURRENT_USER,
      RUNONCE_PATH, _T( "TextEdit Restart" ),
      _T( "%1 /boot" ), strProgram.c_str() );
}


PRIVATE bool shouldUpgrade( const String& strInstalled ) {

   const VersionInfo viOld( strInstalled.c_str() );
   const VersionInfo viNew( getModuleHandle() );
   DWORD dwVersionLoOld = 0;
   DWORD dwVersionHiOld = 0;
   DWORD dwVersionLoNew = 0;
   DWORD dwVersionHiNew = 0;
   if ( viOld.isValid() && viNew.isValid() &&
      viOld.getFileVersion( &dwVersionLoOld, &dwVersionHiOld ) &&
      viNew.getFileVersion( &dwVersionLoNew, &dwVersionHiNew ) )
   {
      return
         dwVersionHiNew < dwVersionHiOld ? false :
         dwVersionHiOld < dwVersionHiNew ? true  :
         dwVersionLoOld < dwVersionLoNew ;
   }
   return false;
}


inline bool match( LPCTSTR str, LPCTSTR pattern ) {

   return 0 == _tcsncicmp( str, pattern, _tcsclen( pattern ) );
}

PRIVATE bool isSetup( ArgumentList *pArgumentList ) {
   
   assert( isGoodPtr( pArgumentList ) );

   if ( pArgumentList->hasOption( _T( "setup" ) ) ) {
      return true;
   }

   const String strProgram = getModuleFileName();
   PATHNAME szBaseName = { 0 };
   _tsplitpath_s( strProgram.c_str(), 0, 0, 0, 0, szBaseName, dim( szBaseName ), 0, 0 );

   if ( match( szBaseName, _T( "setup" ) ) ||
        match( szBaseName, _T( "install" ) ) )
   {
      return true;
   }

   const String strInstalledProgram = getInstallPath();
   if ( strInstalledProgram.empty() ) {
      return true;
   }

   // If others are already running, never mind.
   if ( FindWindow( MAIN_CLASS, 0 ) ) {
      return false;
   }

   // So far, no setup. If we're running a different exe than the
   // installed one, check if the installed one would like to be
   // upgraded. Unless, of course, there were parameters, in which
   // case that would be an unwarranted intrusion.
   if ( 1 == pArgumentList->getNumArgs() &&
        shouldUpgrade( strInstalledProgram ) ) 
   {
#if 0 // Don't self-upgrade. TODO: Thread to check the web
      const UINT uiRet = messageBox( HWND_DESKTOP,
         MB_ICONQUESTION | MB_YESNOCANCEL, IDS_UPGRADE_WARNING,
         strProgram.c_str(), strInstalledProgram.c_str() );
      if ( IDCANCEL == uiRet ) {
         throw CancelException();
      }
      return IDYES == uiRet;
#endif
   }

   return false;
}

Editor *init( LPCTSTR pszCmdLine, int nShow ) {

   const HINSTANCE hinst = getModuleHandle();

   initCommonControls();

   assert( isGoodStringPtr( pszCmdLine ) );
   ArgumentList argumentList( pszCmdLine );

   if ( argumentList.hasOption( _T( "boot" ) ) ) {
      reboot();
      throw CancelException();
   }

   if ( argumentList.hasOption( _T( "clean" ) ) ) {
      clean();
      throw CancelException();
   }

   startInstance( _T( "-clean" ) );

   // Better not do this *before* calling reboot, or we'll loop:
   initReboot();

   if ( isSetup( &argumentList ) ) {
#if 0
      const bool bSilent = argumentList.hasOption( _T( "silent" ) );
      setup( bSilent );
      throw CancelException();
#endif
   }

   const bool bMin      = argumentList.hasOption( _T( "min"     ) );
   const bool bMax      = argumentList.hasOption( _T( "max"     ) );
   const bool bOpenLast = argumentList.hasOption( _T( "last"    ) );
   const bool bPrintTo  = argumentList.hasOption( _T( "pt"      ) ) ||
                          argumentList.hasOption( _T( "printto" ) );
   const bool bPrint    = argumentList.hasOption( _T( "p"       ) ) ||
                          argumentList.hasOption( _T( "print"   ) );

#ifdef _DEBUG
   AbstractEditWnd::bForceEdit = argumentList.hasOption( _T( "edit" ) );
#endif

   if ( bMin ) {
      nShow = SW_SHOWMINIMIZED;
   }
   if ( bMax ) {
      nShow = SW_SHOWMAXIMIZED;
   }

#if 0
   // Catch unrecognized options and complain.
   // We don't do this, but rather attempt to open them as files.
   for ( int iArg = 1; iArg < argumentList.getNumArgs(); ++iArg ) {
      if ( argumentList.isOption( iArg ) ) {
         LPCTSTR const pszArg = argumentList.getArg( iArg );
         messageBox( HWND_DESKTOP, 
            MB_OK | MB_ICONERROR, IDS_UNKNOWN_OPTION, pszArg );
         throw CancelException();
      }
   }
#endif

   // Strip the first file name off the command line, if indicated:
   LPCTSTR pszDocName = 0;
   LPCTSTR pszPrinter = 0;
   LPCTSTR pszDriver  = 0;
   LPCTSTR pszPort    = 0;
   String strArgAll; // Must be at this scope!
   
   HANDLE hIn = GetStdHandle( STD_INPUT_HANDLE );
   trace( _T( "hIn = %d\n" ), hIn );

   if ( !isValidHandle( hIn ) ) {
      if ( bOpenLast ) {
         const MRU mru;
         if ( 0 < mru.getCount() ) {
            static PATHNAME szLastFile = { 0 };
            _tcscpy_s( szLastFile, mru.getFile( ID_MRU_1 ).c_str() );
            pszDocName = szLastFile;
         } else {
            ; // When there are no files in the MRU list, the
              // -last option fails quietly by creating a new file.
         }
      } 
      if ( bPrintTo ) {
         if ( 0 == pszDocName ) {
            pszDocName = argumentList.getArg( 1, true );
         }
         if ( argumentList.getNumArgs() < 4 ) {
            messageBox( HWND_DESKTOP, 
               MB_OK | MB_ICONERROR, IDS_PRINT_ARG_ERROR );
            throw CancelException();
         }
         pszPrinter = argumentList.getArg( 1, true );
         pszDriver  = argumentList.getArg( 1, true );
         pszPort    = argumentList.getArg( 1, true );
      } 
      if ( 0 == pszDocName && 1 < argumentList.getNumArgs() ) { 
         strArgAll = argumentList.getArg( 1 );
         const int nArgs = argumentList.getNumArgs();
         for ( int iArg = 2; iArg < nArgs; ++iArg ) {
            strArgAll += _T( ' ' );
            strArgAll += argumentList.getArg( iArg );
         }

         bool bFound = fileExists( strArgAll.c_str() );
         if ( bFound ) {
            pszDocName = strArgAll.c_str();
         } else {
            strArgAll += getDefaultExtension();
            bFound = fileExists( strArgAll.c_str() );
            // TODO: If ERROR_NOT_READY -- quit?
            if ( bFound ) {
               pszDocName = strArgAll.c_str();
            } else {
               pszDocName = argumentList.getArg( 1 );
            }
         }
         do {
            argumentList.consume( 1 );
         } while ( bFound && 1 < argumentList.getNumArgs() );
      }
   }

   // Must we start another instance?
   if ( 1 < argumentList.getNumArgs() ) {
      startInstance( makeCommandLine( bPrint, argumentList ), nShow );
      // No reasonable way to handle a failure, so don't.
   }

   // Load the document, if we can:
   AutoDocument pDocument( 0 );
   if ( 0 != pszDocName ) {
      assert( isGoodStringPtr( pszDocName ) );

      PATHNAME szRealDocName = { 0 };
      resolveName( szRealDocName, pszDocName );

      if ( !bPrintTo && activateOldInstance( szRealDocName, bPrint ) )
      {
         throw CancelException();
      }
      pDocument = new Document( HWND_DESKTOP, szRealDocName );
   } else if ( isValidHandle( hIn ) ) {
      const String strNewFile = 
         createNewFile( HWND_DESKTOP, file_for_stdin, hIn );
      verify( CloseHandle( hIn ) );
      startInstance( makeCommandLine( bPrint, strNewFile ), nShow );
      throw CancelException();
   } else {
      pDocument = 
         new Document( HWND_DESKTOP, createNewFile().c_str() );
   }
   assert( isGoodPtr( pDocument ) );

   if ( bPrint || bPrintTo ) {
      printFile( pDocument, pszPrinter, pszDriver, pszPort );
      throw CancelException();
   }

   // No more obstacles; we really are going to edit a file.

   HICON hicon = LoadIcon( hinst, MAKEINTRESOURCE( IDI_TEXTEDIT1 ) );
   WNDCLASSEX wc = {
   /* cbSize;        */ sizeof( WNDCLASSEX ),
   /* style;         */ 0,
   /* lpfnWndProc;   */ mainWndProc,
   /* cbClsExtra;    */ 0,
   /* cbWndExtra;    */ MAINWND_EXTRABYTES,
   /* hInstance;     */ hinst,
   /* hIcon;         */ hicon,
   /* hCursor;       */ LoadCursor( 0, IDC_ARROW ),
   /* hbrBackground; */ 0, // Don't need one; never visible.
   /* lpszMenuName;  */ MAKEINTRESOURCE( IDR_MENU ),
   /* lpszClassName; */ MAIN_CLASS,
   /* hIconSm;       */ hicon,
   };

   if ( 0 == RegisterClassEx( &wc ) ) { // Give up!
      throwException( _T( "RegisterClassEx failed" ) );
   }

   // NOTE: Do this before creating the window, otherwise 
   // the maximized state will be reset during window creation.
   if ( SW_MAXIMIZE == pDocument->getWindowState( nShow ) ) {
      nShow = SW_MAXIMIZE;
   }
   if ( bMin ) {
      nShow = SW_SHOWMINIMIZED;
   }
   if ( bMax ) {
      nShow = SW_SHOWMAXIMIZED;
   }

   // Get the window size (default is CW_USEDEFAULT):
   const int x  = pDocument->getLeft  ();
   const int y  = pDocument->getTop   ();
   const int cx = pDocument->getWidth ();
   const int cy = pDocument->getHeight();

   // Possible improvement: Initial default size based on font, 
   // contents and other windows, modified by screen size.
   // Possible improvement: Adjust pos and size to reasonableness,
   // to protect from malicious messing with the registry.

   HWND hwndMain = CreateWindowEx(
   /* dwExStyle    */ WS_EX_ACCEPTFILES | WS_EX_CONTEXTHELP, 
   /* lpClassName  */ MAIN_CLASS,
   /* lpWindowName */ _T( "" ),
   /* dwStyle      */ WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
   /* pos/size     */ x, y, cx, cy,
   /* hWndParent   */ HWND_DESKTOP,
   /* hMenu        */ 0, // Using class menu
   /* hInstance    */ hinst,
   /* lpParam      */ &pDocument );

   // Reset when assigned to Editor, to avoid autodestruction:
   assert( (Document *) 0 == pDocument );

   if ( !IsWindow( hwndMain ) ) {
      // Error handling?
      throwException( _T( "CreateWindowEx failed" ) );
   }

   ShowWindow( hwndMain, nShow );
   UpdateWindow( hwndMain );

   // The one and only Editor object is created in the main
   // window's WM_CREATE handler (onCreate in mainwnd.cpp).
   Editor *pEditor = getEditor( hwndMain );
   assert( isGoodPtr( pEditor ) );

   return pEditor;
}

// end of file
