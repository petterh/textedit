/*
 * $Header: /Book/FindDlg.h 12    20.08.99 16:33 Oslph312 $
 */

#pragma once

#include "Dialog.h"
#include "Editor.h"
#include "persistence.h"


class FindDlg : public Dialog {
private:
   Editor *m_pEditor;
   String m_strSearchPattern;
   String m_strReplacePattern;
   String m_strOriginalTip;
   int    m_nCount;
   bool   m_isReplace;
   bool   m_bReplaceInSelection;
   UINT   m_uiRetCode;

   enum { 
      MAX_STRINGS = 10,
      MAX_TEXT_LENGTH = 200, 
   };

   String getComboContents( HWND hwndCtl, bool bSelChange );
   String getComboContentsAndSetSelection( HWND hwndCtl );
   void addComboString( HWND hwndCtl, const String& str );

   void adjustButtons( void );
   void loadStrings( void );
   void saveStrings( bool bSaveReplacements );
   void positionOutsideSelection( void );
   void onComboChanged( HWND hwndCtl, UINT codeNotify );
   void onFindNext( void );
   void onReplace( void );
   void onReplaceAll( void );

   friend PRIVATE int findStringExact( HWND, LPCTSTR );

protected:
   virtual BOOL onInitDialog( HWND hwndFocus, LPARAM lParam );
   virtual void onDlgCommand( int id, HWND hwndCtl, UINT codeNotify );
   virtual UINT getResourceID( void ) const;

public:
   FindDlg( Editor *pEditor, 
      const String &strSearchPattern, bool isReplace = false );
   const String& getSearchPattern( void ) const;
};


inline const String& FindDlg::getSearchPattern( void ) const {
   return m_strSearchPattern;
}

// end of file
