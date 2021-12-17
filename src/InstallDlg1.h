/*
 * $Header: /Book/InstallDlg1.h 8     5.09.99 13:07 Oslph312 $
 */

#pragma once

#include "Dialog.h"


class InstallDlg1 : public Dialog {
private:
   String m_strInstallDir;
   String m_strDataDir;

   HINSTANCE m_hinst;
   void setupList( void );
   void getList( void );
   void onBrowse( UINT id, LPCTSTR pszTitle );
   
protected:
   virtual UINT getResourceID( void ) const;
   virtual BOOL DlgProc( UINT msg, WPARAM wParam, LPARAM lParam );
   virtual BOOL onInitDialog( HWND hwndFocus, LPARAM lParam );
   virtual void onDlgCommand( int id, HWND hwndCtl, UINT codeNotify );

public:
   InstallDlg1( HINSTANCE hinst, const String& strInstallDir, 
      const String& strDataDir );
   const String& getInstallDir( void );
   const String& getDataDir( void );
};


inline const String& InstallDlg1::getInstallDir( void ) {

   return m_strInstallDir;
}


inline const String& InstallDlg1::getDataDir( void ) {

   return m_strDataDir;
}

// end of file
