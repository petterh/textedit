/*
 * $Header: /scr_test/winUtils.h 13    11.07.01 14:45 Oslph312 $
 */

#pragma once

#include "String.h"
#include "geometry.h"


// The WM_WININICHANGE message has changed its name; this 
// is not reflected in the current version of windowsx.h.

#ifndef HANDLE_WM_SETTINGCHANGE
#ifndef HANDLE_WM_WININICHANGE
#error Define HANDLE_WM_SETTINGCHANGE or HANDLE_WM_WININICHANGE!
#endif
#define HANDLE_WM_SETTINGCHANGE  HANDLE_WM_WININICHANGE
#endif

#ifndef FORWARD_WM_SETTINGCHANGE
#ifndef FORWARD_WM_WININICHANGE
#error Define HANDLE_WM_SETTINGCHANGE or HANDLE_WM_WININICHANGE!
#endif
#define FORWARD_WM_SETTINGCHANGE FORWARD_WM_WININICHANGE
#endif


/**
 * This function determines if the given window is of the given class.
 */
inline bool isClass( HWND hwnd, LPCTSTR pszClassName ) {
   TCHAR szClassName[ 100 ];
   return 
      0 < GetClassName( hwnd, szClassName, dim( szClassName ) ) &&
      0 == _tcsicmp( szClassName, pszClassName );
}


inline Rect getClientRect( HWND hwnd ) {

   assert( ::IsWindow( hwnd ) );

   Rect rc;
   GetClientRect( hwnd, &rc );
   return rc;
}


inline Rect getWindowRect( HWND hwnd ) {

   assert( ::IsWindow( hwnd ) );

   Rect rc;
   GetWindowRect( hwnd, &rc );
   return rc;
}


inline Rect getWindowRectInParent( HWND hwnd ) {

   assert( ::IsWindow( hwnd ) );

   Rect rc = getWindowRect( hwnd );
   MapWindowPoints( HWND_DESKTOP, GetParent( hwnd ), 
      reinterpret_cast< LPPOINT >( &rc ), 2 );
   return rc;
}


inline void moveWindow( 
   HWND hwnd, int x, int y, bool bRepaint = true ) 
{
   assert( ::IsWindow( hwnd ) );
   UINT uiFlags = SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE;
   if ( bRepaint ) {
      uiFlags |= SWP_NOCOPYBITS;
   }
   verify( SetWindowPos( hwnd, 0, x, y, 0, 0, uiFlags ) );
}


inline void moveWindow( 
   HWND hwnd, const RECT& rc, bool bRepaint = true ) 
{
   moveWindow( hwnd, rc.left, rc.top, bRepaint );
}


inline void moveWindow( 
   HWND hwnd, const RECT *prc, bool bRepaint = true ) 
{
   assert( 0 != prc );
   moveWindow( hwnd, *prc, bRepaint );
}


inline void setWindowStyle( HWND hwnd, DWORD dwStyle ) {
   assert( IsWindow( hwnd ) );
   SetWindowLong( hwnd, GWL_STYLE, dwStyle );
}


inline void setButtonStyle( HWND hwnd, DWORD dwStyle ) {
   
   assert( IsWindow( hwnd ) );

   // NOTE: This fails if we clone the class.
   assert( isClass( hwnd, _T( "button" ) ) );
   Button_SetStyle( hwnd, dwStyle, TRUE );
}


inline void adjustDefaultButtonStyle( HWND hwnd ) {

   assert( IsWindow( hwnd ) );

   // The LOWORD is to keep VC++ 5 from complaining 
   // about truncation of long to word.
   const WORD wButtonStyle = IsWindowEnabled( hwnd ) 
      ? LOWORD( BS_DEFPUSHBUTTON ) : LOWORD( BS_PUSHBUTTON );
   setButtonStyle( hwnd, wButtonStyle );
}


#if 0 // Unused.
inline UINT getButtonStyle( HWND hwnd ) {
   assert( IsWindow( hwnd ) );
   return (UINT) GetWindowLong( hwnd, GWL_STYLE ) & 0xff;
}
#endif


#ifdef _STRING_DEFINED_
UINT __cdecl messageBoxV( 
   HWND hwndParent, UINT uiFlags, const String& strFmt, va_list vl );
UINT __cdecl messageBox( 
   HWND hwndParent, UINT uiFlags, const String strFmt, ... );
#endif // _STRING_DEFINED_

UINT __cdecl messageBox( 
   HWND hwndParent, UINT uiFlags, UINT uiFmtStringID, ... );

void modifyStyle( HWND hwnd, DWORD dwAdd, DWORD dwRemove );
HWND getDescendantWindow( HWND hwnd, int nID );
HWND getDefaultButton( HWND hwnd );

void centerDialog( HWND hwndDlg );
void adjustToScreen( HWND hwndDlg );
void savePosition( HWND hwnd, int id );
void restorePosition( HWND hwnd, int id );


#ifdef _STRING_DEFINED_
#ifdef _DEBUG
String _getWindowDescription( HWND hwnd );
#define getWindowDescription( hwnd ) \
   _getWindowDescription( hwnd ).c_str()
#else
#define getWindowDescription( hwnd ) 0
#endif
#endif // _STRING_DEFINED_


// end of file
