/*
 * $Header: /Book/DeleteDlg.h 6     3.07.99 17:46 Oslph312 $
 */

#pragma once

#include "String.h"
#include "Dialog.h"
#include "persistence.h"


class DeleteDlg : public Dialog {
private:
   String m_strFile;

protected:
   virtual BOOL onInitDialog( HWND hwndFocus, LPARAM lParam );
   virtual void onDlgCommand( int id, HWND hwndCtl, UINT codeNotify );
   virtual UINT getResourceID( void ) const;

public:
   DeleteDlg( const String& strFile );
};

// end of file
