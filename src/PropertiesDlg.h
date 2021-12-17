/*
 * $Header: /Book/PropertiesDlg.h 10    3.07.99 17:46 Oslph312 $
 */

#pragma once

#include "Dialog.h"
#include "Document.h"

class PropertiesDlg : public Dialog {
private:
   Document *m_pDocument;
   String   m_strFormat;

   String formatBytes( DWORD dwBytes, bool bAddUsed );
   void setInfo( const WIN32_FIND_DATA& fd );
   bool applyChanges( void );
   bool onBrowse( void );
   void setFileName( const String& strPath );
   String getFileName( void );

protected:
   virtual BOOL onInitDialog( HWND hwndFocus, LPARAM lParam );
   virtual void onDlgCommand( int id, HWND hwndCtl, UINT codeNotify );

   virtual UINT getResourceID( void ) const;

public:
   PropertiesDlg( HWND hwndParent, Document *pDocument );
};

// end of file
