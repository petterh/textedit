/*
 * $Header: /Book/SetupDlg.cpp 18    6.11.01 11:15 Oslph312 $
 * 
 * Handles the setup dialog box, as well as actual install/uninstall.
 * TODO: Check free disk space.
 */

#include "precomp.h"
#include <winver.h>
#include "Help/map.hh"
#include "formatMessage.h"
#include "SetupDlg.h"
#include "AutoHandle.h"
#include "AutoComReference.h"
#include "VersionInfo.h"
#include "Exception.h"
#include "Registry.h"
#include "AboutDlg.h"
#include "InstallDlg1.h"
#include "HTML.h"
#include "FileType.h"
#include "language.h"
#include "createNewFile.h"
#include "menuUtils.h"
#include "winUtils.h"
#include "fileUtils.h"
#include "resource.h"
#include "setup.h"
#include "persistence.h"
#include "threads.h"
#include "utils.h"
#include "os.h"


// Resource IDs in SHELL32.DLL:

#define IDAVI_SEARCH   150
#define IDAVI_FILECOPY 161
#define IDAVI_FILENUKE 164

#define SHOW_PROGRESS( step ) \
   SendMessage( hwndProgress, PBM_SETPOS, step, 0 )


PRIVATE void registerFileForDeletion( const String& strFile ) {
   
   Registry::setString( 
      HKEY_LOCAL_MACHINE, _T( "Uninstall" ), strFile.c_str() );
}


void SetupDlg::deleteResource( UINT uiID, DWORD dwBytes ) {

   /*
    * LATER:
    * This does nothing at the moment. Might consider using the
    * BeginUpdateResource/UpdateResource/EndUpdateResource API
    * to trim the size of the installed executable; the help
    * files are already extracted when this is called.
    * These functions only work under Windows NT, though.
    */
}


void SetupDlg::copyResource( UINT uiID, const String& strFile ) {

   HRSRC hrsrc = 
      FindResource( 0, MAKEINTRESOURCE( uiID ), _T( "FILE" ) );
   DWORD dwBytes = SizeofResource( 0, hrsrc );
   HGLOBAL hRes = LoadResource( 0, hrsrc );
   LPVOID pData = LockResource( hRes );

   AutoHandle hFile( CreateFile( strFile.c_str(),
      GENERIC_WRITE, FILE_SHARE_NONE, 0, CREATE_ALWAYS, 0, 0 ) );
   if ( INVALID_HANDLE_VALUE == hFile ) {
      throwException( _T( "Unable to copy help files" ) );
   }
   DWORD dwBytesWritten = 0;
   verify( WriteFile( hFile, pData, dwBytes, &dwBytesWritten, 0 ) );
   assert( dwBytes == dwBytesWritten );

   UnlockResource( hrsrc ); // Obsolete!
   FreeResource  ( hrsrc ); // Obsolete?
   deleteResource( uiID, dwBytes );

   Sleep( 150 );
   registerFileForDeletion( strFile );
}


void SetupDlg::startAnimation( UINT uiAviId ) {

#ifndef Animate_OpenEx
   #define Animate_OpenEx( hwnd, hInst, szName ) \
      (BOOL)SNDMSG( hwnd, ACM_OPEN,              \
         (WPARAM) hInst, (LPARAM)(LPTSTR)( szName ) )
#endif // Animate_OpenEx

   hideDlgItem( IDC_MESSAGE2 );
   showDlgItem( IDC_ANIMATE );

   HWND hwndAnimate = getDlgItem( IDC_ANIMATE );
   Animate_OpenEx( hwndAnimate, (HMODULE) m_hShell32, MAKEINTRESOURCE( uiAviId ) );
   Animate_Seek( hwndAnimate, 0 );
   Animate_Play( hwndAnimate, 0, -1, -1 );
}


void SetupDlg::stopAnimation( void ) {

   hideDlgItem( IDC_ANIMATE );

   HWND hwndAnimate = getDlgItem( IDC_ANIMATE );
   Animate_Stop( hwndAnimate );
   Animate_Seek( hwndAnimate, 0 );

   showDlgItem( IDC_MESSAGE2 );
}


void SetupDlg::cleanupThread( void ) {
   if ( 0 != m_hThread ) {
      const DWORD dwResult = WaitForSingleObject( m_hThread, 5000 );
      if ( WAIT_OBJECT_0 != dwResult ) {
         trace( _T( "WaitForSingleObject( %d ) failed: %s" ),
            m_hThread, WinException().what() );
      }
      verify( CloseHandle( m_hThread ) );
      m_hThread = 0;
   }
}


String SetupDlg::getHelpFile( void ) const {
   return formatMessage( _T( "%1\\TextEdit.hlp" ), 
      m_strInstallDir.c_str() );
}

UINT __stdcall SetupDlg::searchPreviousThread( LPVOID p ) {

   SetupDlg *pSetupDlg = reinterpret_cast< SetupDlg * >( p );
   assert( isGoodPtr( pSetupDlg ) );
   
   try {
      pSetupDlg->searchPrevious();
   }
   catch ( const Exception& x ) {
      pSetupDlg->sendMessage( WM_APP, SEARCH_FAILED, 
         reinterpret_cast< LPARAM >( x.what() ) );
   }
   return 0;
}


UINT __stdcall SetupDlg::installThread( LPVOID p ) {

   SetupDlg *pSetupDlg = reinterpret_cast< SetupDlg * >( p );
   assert( 0 != pSetupDlg );

   HRESULT hres = coInitialize();
   if ( SUCCEEDED( hres ) ) {
      try {
         pSetupDlg->install();
      }
      catch ( const Exception& x ) {
         pSetupDlg->sendMessage( WM_APP, INSTALL_FAILED, 
            reinterpret_cast< LPARAM >( x.what() ) );
      }
      
      CoUninitialize();
   } else {
      const LPCTSTR pszMessage = 
         _T( "Cannot initialize COM.\nSorry, I give up." );
      pSetupDlg->sendMessage( WM_APP, INSTALL_FAILED, 
         reinterpret_cast< LPARAM >( pszMessage ) );
   }
   return 0;
}


UINT __stdcall SetupDlg::uninstallThread( LPVOID p ) {

   SetupDlg *pSetupDlg = reinterpret_cast< SetupDlg * >( p );
   assert( 0 != pSetupDlg );

   try {
      pSetupDlg->uninstall();
   }
   catch ( const Exception& x ) {
      pSetupDlg->sendMessage( WM_APP, UNINSTALL_FAILED, 
         reinterpret_cast< LPARAM >( x.what() ) );
   }
   return 0;
}


void SetupDlg::searchPrevious( void ) {
   
   sendMessage( WM_APP, START_SEARCH, 0 );

   HWND hwndProgress = getDlgItem( IDC_PROGRESS );
   SendMessage( hwndProgress, PBM_SETRANGE, 0, MAKELPARAM( 0, 2 ) );
   SHOW_PROGRESS( 0 );
   Sleep( 150 );

   m_strExePath = getInstallPath();
   WIN32_FIND_DATA fd = { 0 };
   HANDLE hFind = FindFirstFile( m_strExePath.c_str(), &fd );
   m_hasPrevious = INVALID_HANDLE_VALUE != hFind;
   if ( m_hasPrevious ) {
      verify( FindClose( hFind ) );
      const VersionInfo viOld( m_strExePath.c_str() );
      assert( viOld.isValid() );
      m_hasPrevious = viOld.isValid();
      if ( m_hasPrevious ) {
         m_strVersion = 
            viOld.getStringFileInfo( _T( "FileVersion" ) );
         DWORD dwVersionLoOld = 0;
         DWORD dwVersionHiOld = 0;
         verify( viOld.getFileVersion( 
            &dwVersionLoOld, &dwVersionHiOld ) );
         const VersionInfo viNew( getModuleHandle() );
         assert( viNew.isValid() );
         DWORD dwVersionLoNew = 0;
         DWORD dwVersionHiNew = 0;
         verify( viNew.getFileVersion( 
            &dwVersionLoNew, &dwVersionHiNew ) );
         m_isOlderThanPrevious = 
            dwVersionHiNew < dwVersionHiOld ? true  :
            dwVersionHiOld < dwVersionHiNew ? false :
            dwVersionLoNew < dwVersionLoOld ;
      }
   }
   Sleep( 150 );
   SHOW_PROGRESS( 1 );
   Sleep( 150 );
   SHOW_PROGRESS( 2 );

   if ( !m_isCancelled ) {
      sendMessage( WM_APP, DONE_SEARCH, 0 );
   }
}


PRIVATE void addShortcut( int nFolder, const String &strProgPath ) {

   String strLinkPath = getSpecialFolderLocation( nFolder );
   if ( !strLinkPath.empty() ) {
      
      // Program name: Get title from version resource
      LPCTSTR pszTitle = _T( "TextEdit" );
      const VersionInfo vi( getModuleHandle() );
      if ( vi.isValid() ) {
         pszTitle = vi.getStringFileInfo( _T( "FileDescription" ) );
         assert( 0 != pszTitle );
         if ( 0 == _tcscmp( pszTitle, _T( "?" ) ) ) {
            trace( _T( "addShortcuts: Unable to retrieve " )
               _T( "program title; using default\n" ) );
            pszTitle = _T( "TextEdit" );
         }
      } else {
         trace( _T( "Unable to retrieve version info\n" ) );
      }

      addPathSeparator( &strLinkPath );
      strLinkPath += formatMessage(
         _T( "%1.lnk" ), pszTitle );
      
      AutoComReference< IShellLink > 
         psl( CLSID_ShellLink, IID_IShellLink );
      AutoComReference< IPersistFile > ppf( IID_IPersistFile, psl );
      PATHNAMEW wszLink = { 0 };
#ifdef UNICODE
      _tcscpy_s( wszLink, strLinkPath.c_str() );
#else
      multiByteToWideChar( strLinkPath.c_str(), wszLink );
#endif
      trace( _T( "Creating link: %ws\n" ), wszLink );
      verify( SUCCEEDED( psl->SetPath( strProgPath.c_str() ) ) );
      verify( SUCCEEDED( psl->SetIconLocation( 
         strProgPath.c_str(), 0 ) ) );
      verify( SUCCEEDED( ppf->Save( wszLink, true ) ) );
      registerFileForDeletion( strLinkPath );
   }
}


void SetupDlg::install( void ) {

   sendMessage( WM_APP, START_INSTALL, 0 );
   setNewLanguage( getLanguage() );
   
   HWND hwndProgress = getDlgItem( IDC_PROGRESS );
   SendMessage( hwndProgress, PBM_SETRANGE, 0, MAKELPARAM( 0, 4 ) );
   SHOW_PROGRESS( 1 );

   CreateDirectory( m_strInstallDir.c_str(), 0 );
   DWORD dwErr = GetLastError();
   if ( NOERROR != dwErr && ERROR_ALREADY_EXISTS != dwErr ) {
      throwException( 
         _T( "Unable to create program directory.<p>" ) );
   }
   if ( NOERROR == dwErr ) {
      SHChangeNotify( 
         SHCNE_MKDIR, SHCNF_PATH, m_strInstallDir.c_str(), 0 );
   }

   const String strSetupPath = getModuleFileName();

   // NOTE: Check version! VerInstallFile, VerFindFile?
   m_strExePath = formatMessage( 
      _T( "%1\\TextEdit.exe" ), m_strInstallDir.c_str() );
   
   // Don't care if modifyAttribs() fails, but if a read-only 
   // target file exists, CopyFile() will fail with access denied.
   modifyAttribs( m_strExePath, 0, FILE_ATTRIBUTE_READONLY );

   // NOTE: CopyFileEx not supported on Windows 95
   const BOOL bOK = CopyFile( strSetupPath.c_str(), 
      m_strExePath.c_str(), /* fail if exists */ false );
   if ( !bOK ) {
      throwException( _T( "Unable to copy program file.<p>" ) );
   }

   SHOW_PROGRESS( 2 );
   copyResource( IDR_HLP_FILE, 
      formatMessage( _T( "%1\\TextEdit.hlp" ), 
      m_strInstallDir.c_str() ) );
   SHOW_PROGRESS( 3 );
   copyResource( IDR_CNT_FILE, 
      formatMessage( _T( "%1\\TextEdit.cnt" ), 
      m_strInstallDir.c_str() ) );
   SHOW_PROGRESS( 4 );

   SHChangeNotify( 
      SHCNE_UPDATEDIR, SHCNF_PATH, strSetupPath.c_str(), 0 );

   FileType::setCommand( m_strExePath );
   const int nTypes = FileType::getNumFileTypes();
   for ( int iType = 0; iType < nTypes; ++iType ) {
      FileType *pFileType = FileType::getFileType( iType );
      pFileType->registerType();
   }

   try {
      addShortcut( CSIDL_PROGRAMS, m_strExePath );
      addShortcut( CSIDL_SENDTO  , m_strExePath );
   }
   catch ( const ComException& x ) {
      trace( _T( "ComException in addShortcuts" ), x.what() );
      messageBox( *this, MB_OK | MB_ICONWARNING,
         _T( "Couldn't create start menu shortcut:\n%1" ), x.what() );
   }

   SHChangeNotify( SHCNE_FREESPACE, SHCNF_PATH, 
      getRootDir( strSetupPath ).c_str(), 0 );
   if ( !m_isCancelled ) {
      sendMessage( WM_APP, DONE_INSTALL, 0 );
   }

   setInstallPath( m_strExePath );
}


bool SetupDlg::deleteFile( LPCTSTR pszFile ) {

   const String strPath = formatMessage( _T( "%1\\%2" ), 
      m_strInstallDir.c_str(), pszFile );
   return 0 != DeleteFile( strPath.c_str() );
}


void SetupDlg::uninstall( void ) {

   sendMessage( WM_APP, START_UNINSTALL, 0 );

   HWND hwndProgress = getDlgItem( IDC_PROGRESS );

   // Remove program and help files

   WinHelp( *this, getHelpFile().c_str(), HELP_QUIT, 0 );

   SendMessage( hwndProgress, PBM_SETRANGE, 0, MAKELPARAM( 0, 8 ) );
   SHOW_PROGRESS( 0 );

   const bool bDeleted = deleteFile( _T( "TextEdit.exe" ) );
   DWORD dwErr = GetLastError();
   assert( bDeleted || ERROR_ACCESS_DENIED == dwErr );
   Sleep( 150 );
   SHOW_PROGRESS( 1 );
   
   deleteFile( _T( "TextEdit.hlp" ) );
   Sleep( 150 );
   SHOW_PROGRESS( 2 );
   
   deleteFile( _T( "TextEdit.cnt" ) );
   Sleep( 150 );
   SHOW_PROGRESS( 3 );
   
   deleteFile( _T( "TextEdit.gid" ) );
   Sleep( 150 );
   SHOW_PROGRESS( 4 );
   
   deleteFile( _T( "TextEdit.fts" ) );
   Sleep( 150 );
   SHOW_PROGRESS( 5 );
   
   // Remove all shortcuts on desktop and on start menu.
   // Some files in the uninstall list have already been deleted.
   String strFileToDelete;
   DWORD dwIndex = 0;
   while ( Registry::enumValues( HKEY_LOCAL_MACHINE, 
      _T( "Uninstall" ), dwIndex++, &strFileToDelete ) )
   {
      DeleteFile( strFileToDelete.c_str() );
   }
   Registry::deleteEntry( HKEY_LOCAL_MACHINE, _T( "Uninstall" ) );

   Sleep( 150 );
   SHOW_PROGRESS( 6 );

   bool bDirRemoved = 0 != RemoveDirectory( m_strInstallDir.c_str() );
   dwErr = GetLastError();
   assert( bDirRemoved || 
      ERROR_ACCESS_DENIED == dwErr || 
      ERROR_DIR_NOT_EMPTY == dwErr );
   Sleep( 150 );
   SHOW_PROGRESS( 7 );

   // Remove uninstall strings in registry:
   Registry::deleteEntry( HKEY_LOCAL_MACHINE, UNINSTALL_PATH );
   Sleep( 150 );
   SHOW_PROGRESS( 8 );

   // Remove RunOnce strings in registry:
   Registry::deleteEntry( HKEY_CURRENT_USER,
      RUNONCE_PATH, _T( "TextEdit Restart" ) );

   // Remove user data in registry -- for all users!
   Registry::deleteEntry( HKEY_CURRENT_USER, _T( "" ) );

   // Remove application data in registry.
   Registry::deleteEntry( HKEY_LOCAL_MACHINE, _T( "" ) );

   // Remove all registry commands:
   FileType::setCommand( m_strExePath );
   const int nTypes = FileType::getNumFileTypes();
   for ( int iType = 0; iType < nTypes; ++iType ) {
      FileType *pFileType = FileType::getFileType( iType );
      pFileType->unregisterType();
   }

   bool bDelayedDelete = false;
   bool bDelayedDirRemove = false;
   if ( !bDeleted ) {
      bDelayedDelete = delayedRemove( m_strInstallDir );
   }
   if ( !bDirRemoved ) {
      bDelayedDirRemove = delayedRemove( m_strInstallDir.c_str() );
   }
   m_bDelayedRemove = bDelayedDirRemove || bDelayedDelete;

   SHChangeNotify( SHCNE_UPDATEDIR, SHCNF_PATH, m_strInstallDir.c_str(), 0 );
   SHChangeNotify( SHCNE_FREESPACE, SHCNF_PATH, m_strInstallDir.c_str(), 0 );

   if ( !m_isCancelled ) {
      sendMessage( WM_APP, DONE_UNINSTALL );
   }
}


inline void SetupDlg::setMessage( const String& strMessage ) {
   setDlgItemText( IDC_MESSAGE, strMessage.c_str() );
}


inline void SetupDlg::setMessage( UINT uiString ) {
   setMessage( loadString( uiString ) );
}


UINT SetupDlg::getResourceID( void ) const {
   return IDD_SETUP;
}


int SetupDlg::getDefaultButtonID( void ) const {
   if ( m_hasPrevious ) {
      return m_isOlderThanPrevious ? IDCANCEL : IDC_INSTALL;
   }
   return IDC_INSTALL;
}


BOOL SetupDlg::DlgProc( UINT msg, WPARAM wParam, LPARAM lParam ) {
   assert( (ID_HELP_ABOUT & 0xF) == 0 );
   if ( WM_SYSCOMMAND == msg && (wParam & 0xFFF0) == ID_HELP_ABOUT ) {
      AboutDlg( *this );
      return TRUE;
   }

   if ( WM_APP != msg ) {
      // Putting this here results in its getting called
      // more often than necessary. But my, how convenient!
      HMENU hmenu = GetSystemMenu( *this, false );
      enableMenuItem( hmenu, SC_RESTORE, 0 != IsIconic( *this ) );
      return Dialog::DlgProc( msg, wParam, lParam );
   }

   switch ( wParam ) {
   case START_SEARCH:
      startAnimation( IDAVI_SEARCH );
      setMessage( IDS_INSTALL_SEARCH );
      break;
      
   case DONE_SEARCH:
      ReplyMessage( 0 );
      stopAnimation();
      if ( m_hasPrevious ) {
         setMessage( formatMessage( 
            IDS_INSTALL_FOUND, m_strVersion.c_str() ) );
         setDlgItemText( IDC_MESSAGE2, IDS_INSTALL_UNINSTALL );
         enableDlgItem( IDC_UNINSTALL, true );
      } else {
         setMessage( IDS_INSTALL_NOT_FOUND );
         setDlgItemText( IDC_MESSAGE2, IDS_INSTALL );
      }
      enableDlgItem( IDC_INSTALL, true );
      sendMessage( DM_SETDEFID, getDefaultButtonID() );
      gotoDlgItem( getDefaultButtonID() );
      cleanupThread();
      break;

   case START_UNINSTALL:
      ReplyMessage( 0 );
      startAnimation( IDAVI_FILENUKE );
      sendMessage( DM_SETDEFID, IDCANCEL );
      gotoDlgItem( IDCANCEL );
      DestroyWindow( getDlgItem( IDC_UNINSTALL ) );
      DestroyWindow( getDlgItem( IDC_INSTALL   ) );
      setMessage( IDS_DELETING );
      setDlgItemText( IDC_MESSAGE2, _T( "" ) );
      break;

   case DONE_UNINSTALL:
      ReplyMessage( 0 );
      stopAnimation();
      cleanupThread();
      m_uiResult = IDC_UNINSTALL;
      setMessage( IDS_DELETED );
      if ( m_bDelayedRemove ) {
         setDlgItemText( IDC_MESSAGE2, IDS_DELAYED_REMOVE );
      } else {
         setDlgItemText( IDC_MESSAGE2, _T( "" ) );
      }
      setDlgItemText( IDCANCEL, IDS_CLOSE );
      break;

   case START_INSTALL:
      ReplyMessage( 0 );
      startAnimation( IDAVI_FILECOPY );
      sendMessage( DM_SETDEFID, IDCANCEL );
      gotoDlgItem( IDCANCEL );
      DestroyWindow( getDlgItem( IDC_UNINSTALL ) );
      DestroyWindow( getDlgItem( IDC_INSTALL   ) );
      setMessage( IDS_INSTALL_COPYING );
      setDlgItemText( IDC_MESSAGE2, _T( "" ) );
      break;

   case DONE_INSTALL:
      ReplyMessage( 0 );
      stopAnimation();
      cleanupThread();
      m_uiResult = IDC_INSTALL;
      setMessage( IDS_INSTALL_SUCCESS );
      setDlgItemText( IDC_MESSAGE2, IDS_UNINSTALL_HINT );
      setDlgItemText( IDCANCEL, IDS_CLOSE );
      break;

   case SEARCH_FAILED:
   case UNINSTALL_FAILED:
   case INSTALL_FAILED:
      stopAnimation();
      setMessage( IDS_INSTALL_FAILED );
      setDlgItemText( 
         IDC_MESSAGE2, reinterpret_cast< LPCTSTR >( lParam ) );
      ReplyMessage( 0 );
      setDlgItemText( IDCANCEL, IDS_CLOSE );
      cleanupThread();
      break;
   }

   return TRUE;
}

PRIVATE inline void fixSystemMenu( HMENU hmenu ) {
   DeleteMenu( hmenu, SC_MAXIMIZE, MF_BYCOMMAND );
   DeleteMenu( hmenu, SC_SIZE    , MF_BYCOMMAND );
   appendSeparator( hmenu );
   appendMenuItem( hmenu, ID_HELP_ABOUT, loadString( IDS_ABOUT ).c_str() );
}

BOOL SetupDlg::onInitDialog( HWND hwndFocus, LPARAM lParam ) {
   fixSystemMenu( GetSystemMenu( *this, false ) );

   const HICON hicon = LoadIcon( getModuleHandle(), MAKEINTRESOURCE( IDI_SETUP ) );
   sendMessage( WM_SETICON, ICON_SMALL, reinterpret_cast< LPARAM >( hicon ) );
   sendMessage( WM_SETICON, ICON_BIG  , reinterpret_cast< LPARAM >( hicon ) );

   const VersionInfo vi( getModuleHandle() );
   const LPCTSTR pszVersion = vi.getStringFileInfo( _T( "FileVersion" ) );
   assert( 0 != pszVersion );
   const String strFmt = getDlgItemText( IDC_SETUP_TITLE );
   setDlgItemText( IDC_SETUP_TITLE, formatMessage( strFmt, pszVersion ) );

   subclassHTML( getDlgItem( IDC_SETUP_TITLE ) );
   subclassHTML( getDlgItem( IDC_MESSAGE     ) );
   subclassHTML( getDlgItem( IDC_MESSAGE2    ) );

   m_hThread = beginThread( searchPreviousThread, this );

   sendMessage( DM_SETDEFID, IDCANCEL );
   SetFocus( getDlgItem( IDCANCEL ) );
   return FALSE; // Don't let dialog manager set the focus; we did it.
}

void SetupDlg::onDlgCommand( int id, HWND hwndCtl, UINT codeNotify ) {

   switch ( id ) {
   case IDC_UNINSTALL:
      {
         const UINT uiResult = messageBox( *this, 
            MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2,
            IDS_UNINSTALL_WARNING );
         if ( IDYES == uiResult ) {
            m_hThread = beginThread( uninstallThread, this );
         }
      }
      break;

   case IDC_INSTALL:
      {
         if ( m_isOlderThanPrevious ) {
            const UINT uiAnswer = messageBox( *this,
               MB_OKCANCEL | MB_DEFBUTTON2 | MB_ICONQUESTION,
               IDS_VERSION_WARNING );
            if ( IDOK != uiAnswer ) {
               gotoDlgItem( IDCANCEL );
               break; //*** BREAK POINT
            }
         }
         InstallDlg1 installDlg1( getModuleHandle(), m_strInstallDir, m_strDataDir );
         const UINT uiRetCode = dynamic_cast< Dialog * >(  &installDlg1 )->doModal( *this );
         if ( IDC_INSTALL == uiRetCode ) {
            m_strInstallDir = installDlg1.getInstallDir();
            m_strDataDir    = installDlg1.getDataDir   ();
            m_strExePath = formatMessage( 
               _T( "%1\\TextEdit.exe" ), m_strInstallDir.c_str() );
            setInstallPath ( m_strExePath.c_str() );
            setDocumentPath( m_strDataDir.c_str() );
            m_hThread = beginThread( installThread, this );
         }
      }
      break;

   case IDCANCEL:
      m_isCancelled = true;
      Sleep( 0 );
      cleanupThread();
      verify( EndDialog( *this, m_uiResult ) );
      break;
   }
}


SetupDlg::SetupDlg( bool bInstall ) 
   : m_hThread( 0 )
   , m_bInstall( bInstall ) 
   , m_strVersion( _T( "" ) )
   , m_isCancelled( false )
   , m_hasPrevious( false )
   , m_isOlderThanPrevious( false )
   , m_uiResult( IDCANCEL )
   , m_hShell32( _T( "shell32" ) )
{
   m_isCancelled = false;

   m_strInstallDir = getInstallPath();
   if ( m_strInstallDir.empty() ) {
      m_strInstallDir = Registry::getString(
         HKEY_LOCAL_MACHINE,
         _T( "SOFTWARE\\Microsoft\\Windows\\CurrentVersion" ),
         _T( "ProgramFilesDir" ), _T( "C:\\" ) );
   }
   appendProgramName( &m_strInstallDir );

   m_strDataDir = getDocumentPath();
   if ( m_strDataDir.empty() ) {
      m_strDataDir = getDefaultPath();
   }
   if ( m_strDataDir.empty() ) {
      m_strDataDir = m_strInstallDir; // Not too brilliant.
   }
}


SetupDlg::~SetupDlg() {
}

// end of file
