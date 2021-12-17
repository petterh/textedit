/*
 * $Header: /Book/SetupDlg.h 13    6-09-01 12:54 Oslph312 $
 */

#pragma once

#include "Dialog.h"
#include "AutoLibrary.h"


/**
 * SetupDlg handles the TextEdit setup dialog box.
 */
class SetupDlg : public Dialog {
private:
   enum { 
      START_SEARCH   , DONE_SEARCH   , SEARCH_FAILED   ,
      START_UNINSTALL, DONE_UNINSTALL, UNINSTALL_FAILED,
      START_INSTALL  , DONE_INSTALL  , INSTALL_FAILED  ,
   };

   HINSTANCE   m_hinst;
   AutoLibrary m_hShell32; // Contains animations
   HANDLE      m_hThread;
   bool        m_bInstall;
   String      m_strVersion;
   String      m_strInstallDir;
   String      m_strDataDir;
   String      m_strExePath;
   UINT        m_uiResult;

   bool m_isCancelled;
   bool m_hasPrevious;
   bool m_isOlderThanPrevious;
   bool m_bDelayedRemove;

   void setMessage( const String& strMessage );
   void setMessage( UINT uiString );
   void startAnimation( UINT uiAviID );
   void stopAnimation( void );
   int  getDefaultButtonID( void ) const;

   void searchPrevious( void );
   void install( void );
   void uninstall( void );
   bool deleteFile( LPCTSTR pszFile );
   void copyResource( UINT uiID, const String& strFile );
   void deleteResource( UINT uiID, DWORD dwBytes );

   void cleanupThread( void );
   String getHelpFile( void ) const;

   static UINT __stdcall searchPreviousThread( LPVOID );
   static UINT __stdcall installThread       ( LPVOID );
   static UINT __stdcall uninstallThread     ( LPVOID );

protected:
   virtual UINT getResourceID( void ) const;
   virtual BOOL DlgProc( UINT msg, WPARAM wParam, LPARAM lParam );
   virtual BOOL onInitDialog( HWND hwndFocus, LPARAM lParam );
   virtual void onDlgCommand( int id, HWND hwndCtl, UINT codeNotify );

public:
   SetupDlg( bool bInstall );
   virtual ~SetupDlg();

   const String& getExePath( void ) const;
};


inline const String& SetupDlg::getExePath( void ) const {

   assert( !m_strExePath.empty() );
   return m_strExePath;
}

// end of file
