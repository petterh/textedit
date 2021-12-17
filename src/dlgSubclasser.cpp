/*
 * $Header: /Book/dlgSubclasser.cpp 13    3.07.99 17:46 Oslph312 $
 * 
 * Subclassing of dialogs to handle default button 
 * "defaultness" on combo box drop-down.
 * No include file required; static initialization does it all.
 */

#include "precomp.h"
#include "winUtils.h"
#include "GlobalSubclasser.h"


PRIVATE LRESULT CALLBACK 
   dlgSubclassWndProc( HWND, UINT, WPARAM, LPARAM );


PRIVATE GlobalSubclasser 
   dlgSubclasser( WC_DIALOG, dlgSubclassWndProc );


inline bool isCombo( HWND hwnd ) {
    TCHAR szClassName[ 100 ] = { 0 };
    GetClassName( hwnd, szClassName, dim( szClassName ) );
    return 0 == _tcsicmp( szClassName, _T( "ComboBox" ) ) ||
           0 == _tcsicmp( szClassName, WC_COMBOBOXEX    ) ;
}


PRIVATE inline void onCommand(
   HWND hwnd, int id, HWND hwndCtl, UINT codeNotify )
{
   if ( CBN_DROPDOWN == codeNotify || CBN_CLOSEUP == codeNotify ) {
      if ( isCombo( hwndCtl ) ) {
         HWND hwndDefault = getDefaultButton( hwnd );
         if ( IsWindow( hwndDefault ) ) {
            if ( CBN_DROPDOWN == codeNotify ) {
               setButtonStyle( hwndDefault, BS_PUSHBUTTON );
            } else if ( IsWindowEnabled( hwndDefault ) ) {
               setButtonStyle( hwndDefault, BS_DEFPUSHBUTTON );
            }
         }
      }
   }
}


PRIVATE inline LRESULT internal_dlgSubclassWndProc( 
   HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) 
{
   switch ( msg ) {
   HANDLE_MSG( hwnd, WM_COMMAND, onCommand );
   }
   return 0;
}


PRIVATE LRESULT CALLBACK dlgSubclassWndProc( 
   HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) 
{
   const LRESULT lResult = 
      dlgSubclasser.callOldProc( hwnd, msg, wParam, lParam );
   internal_dlgSubclassWndProc( hwnd, msg, wParam, lParam );
   return lResult;
}

// end of file
