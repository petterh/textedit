/*
 * $Header: /Book/OptionsDlg.h 7     3.07.99 17:46 Oslph312 $
 */

#pragma once

#include "Dialog.h"


/**
 * OptionsDlg handles the TextEdit Options dialog box.
 */
class OptionsDlg : public Dialog {
private:
   LOGFONT m_logFontFixed;
   LOGFONT m_logFontProportional;
   HFONT   m_hfontFixed;
   HFONT   m_hfontProportional;

   void changeFont(
      HWND hwndSample, LOGFONT *pLogFont, 
      HFONT *phfont, DWORD dwExtraFlags = 0 );

protected:
   virtual BOOL onInitDialog( HWND hwndFocus, LPARAM lParam );
   virtual void onDlgCommand( int id, HWND hwndCtl, UINT codeNotify );
   virtual UINT getResourceID( void ) const;

public:
   OptionsDlg( const LOGFONT *pLogFontFixed, 
               const LOGFONT *pLogFontProportional );
   virtual ~OptionsDlg();

   const LOGFONT *getFixedFont( void ) const;
   const LOGFONT *getProportionalFont( void ) const;
};


inline const LOGFONT *OptionsDlg::getFixedFont( void ) const {

   return &m_logFontFixed;
}


inline const LOGFONT *OptionsDlg::getProportionalFont( void ) const {

   return &m_logFontProportional;
}

// end of file