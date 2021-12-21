/*
 * $Header: /Book/AboutDlg.h 8     3.07.99 17:46 Oslph312 $
 */

#pragma once

#ifndef _ABOUTDLG_H_
#define _ABOUTDLG_H_


#include "Dialog.h"


/**
 * AboutDlg handles the TextEdit About dialog box.
 */
class AboutDlg : public Dialog {
private:
   HFONT m_hfontBig;
   HFONT m_hfontBold;

   void setFonts( void );
   void setInfo( void );
   
protected:
   virtual BOOL onInitDialog( HWND hwndFocus, LPARAM lParam );
   virtual UINT getResourceID( void ) const;

public:
   AboutDlg( HWND hwndParent );
   virtual ~AboutDlg();
};

#endif // _ABOUTDLG_H_

// end of file
