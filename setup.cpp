/*
 * $Header: /Book/setup.cpp 11    5.09.99 13:07 Oslph312 $
 */

#include "precomp.h"
#include "SetupDlg.h"
#include "InstallDlg1.h"
#include "Registry.h"
#include "VersionInfo.h"
#include "setup.h"
#include "utils.h"
#include "resource.h"
#include "fileUtils.h"
#include "winUtils.h"


PRIVATE void registerUninstall( const String& strCommand ) {

   setInstallPath( strCommand );

   TCHAR szDisplayName[ 100 ] = _T( "TextEdit" );
   VersionInfo versionInfo( strCommand.c_str() );
   assert( versionInfo.isValid() );
   if ( versionInfo.isValid() ) {
      const LPCTSTR pszTitle = 
         versionInfo.getStringFileInfo( _T( "FileDescription" ) );
      const LPCTSTR pszCompany = 
         versionInfo.getStringFileInfo( _T( "CompanyName" ) );
      const LPCTSTR pszVersion = 
         versionInfo.getStringFileInfo( _T( "FileVersion" ) );
      wsprintf( szDisplayName, 
         _T( "%s %s [%s]" ), pszTitle, pszVersion, pszCompany );
   }

   Registry::setString( HKEY_LOCAL_MACHINE, 
      UNINSTALL_PATH, _T( "DisplayName" ), szDisplayName );

   const String strSetupCommand = strCommand + _T( " /setup" );
   Registry::setString( HKEY_LOCAL_MACHINE, 
      UNINSTALL_PATH, _T( "UninstallString" ), 
      _T( "\"%1\"" ), strSetupCommand.c_str());
}


void appendProgramName( String *pstrPath ) {

   assert( isGoodPtr( pstrPath ) );

   LPCTSTR pszTitle = _T( "TextEdit" );
   VersionInfo versionInfo( getModuleFileName().c_str() );
   if ( versionInfo.isValid() ) {
      pszTitle = 
         versionInfo.getStringFileInfo( _T( "FileDescription" ) );
   }

   String strExe( pszTitle );
   strExe += _T( ".exe" );

   const int nExeOffset = pstrPath->find( strExe );
   const bool bExeAtEnd = 
      nExeOffset == (int)(pstrPath->length() - strExe.length());
   if ( bExeAtEnd ) {
      pstrPath->erase( nExeOffset - 1 );
   }
   
   const int nProgOffset = pstrPath->find( pszTitle );
   const bool bHasProgramNameAtEnd = 0 <= nProgOffset &&
      nProgOffset == (int)(pstrPath->length() - _tcslen( pszTitle ));
   
   if ( !bHasProgramNameAtEnd ) {
      addPathSeparator( pstrPath );
      *pstrPath += pszTitle;
   }
}


void setup( bool bSilent ) {

   SetupDlg setupDlg( false );
   UINT uiRetCode = dynamic_cast< Dialog * >( &setupDlg )->doModal();
   assert( 
      IDC_INSTALL   == uiRetCode || 
      IDC_UNINSTALL == uiRetCode || 
      IDCANCEL      == uiRetCode );

   if ( IDC_INSTALL == uiRetCode ) {
      registerUninstall( setupDlg.getExePath() );
   }
}

// end of file
