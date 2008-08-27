/*
 * $Header: /Book/winUtils.cpp 18    16.07.04 10:42 Oslph312 $
 */

#include "precomp.h"
#include "Exception.h"
#include "VersionInfo.h"
#include "formatMessage.h"
#include "winUtils.h"
#include "geometry.h"
#include "utils.h"


/**
 * Used by messageBoxV.
 */
PRIVATE void stripHTML( String *pstr ) {

   assert( isGoodPtr( pstr ) );
   replace( pstr, _T( "<b>"  ) );
   replace( pstr, _T( "</b>" ) );
   replace( pstr, _T( "<p>"  ), _T( "\n" ) );
}


UINT __cdecl messageBoxV( 
   HWND hwndParent, UINT uiFlags, const String& strFmt, va_list vl )
{
   LPCTSTR pszTitle = _T( "TextEdit" );
   const VersionInfo vi( getModuleHandle() );
   if ( vi.isValid() ) {
      pszTitle = vi.getStringFileInfo( _T( "FileDescription" ) );
   }
   String strMessage = formatMessageV( strFmt, vl );
   stripHTML( &strMessage );
   const UINT uiRet = MessageBox( 
      hwndParent, strMessage.c_str(), pszTitle, uiFlags );
   if ( 0 == uiRet ) {
      trace( _T( "MessageBox failed: %s\n" ), WinException().what() );
   }
   return uiRet;
}


UINT __cdecl messageBox( 
   HWND hwndParent, UINT uiFlags, const String strFmt, ... ) 
{
   va_list vl;
   va_start( vl, strFmt );
   const UINT uiResult = 
      messageBoxV( hwndParent, uiFlags, strFmt, vl );
   va_end( vl );

   return uiResult;
}


UINT __cdecl messageBox( 
   HWND hwndParent, UINT uiFlags, UINT uiFmtStringID, ... ) 
{
   const String strFmt = loadString( uiFmtStringID );

   va_list vl;
   va_start( vl, uiFmtStringID );
   const UINT uiResult = 
      messageBoxV( hwndParent, uiFlags, strFmt, vl );
   va_end( vl );

   return uiResult;
}


/**
 * Warning -- not all style bits are independent.
 */ 
void modifyStyle( HWND hwnd, DWORD dwAdd, DWORD dwRemove ) {

   assert( ::IsWindow( hwnd ) );
   const DWORD dwOldStyle = GetWindowStyle( hwnd );
   DWORD dwNewStyle = dwOldStyle;
   dwNewStyle &= ~dwRemove;
   dwNewStyle |= dwAdd;
   if ( dwOldStyle != dwNewStyle ) {
      setWindowStyle( hwnd, dwNewStyle );
      verify( SetWindowPos( hwnd, 0, 0, 0, 0, 0, 
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | 
        SWP_NOACTIVATE | SWP_FRAMECHANGED ) );
   }
}


HWND getDescendantWindow( HWND hwnd, int nID ) {

   HWND hwndChild = GetDlgItem( hwnd, nID );
   if ( 0 != hwndChild ) {
      return hwndChild;
   }

   for ( hwndChild = GetTopWindow( hwnd ); 0 != hwndChild;
      hwndChild = GetNextWindow( hwndChild, GW_HWNDNEXT ) )
   {
      HWND hwndSubChild = getDescendantWindow( hwndChild, nID );
      if ( 0 != hwndSubChild ) {
         return hwndSubChild;
      }
   }
   return 0;
}


/**
 * Search the hierarchy up and down for a default button.
 */ 
HWND getDefaultButton( HWND hwnd ) {
   const UINT nID = SNDMSG( hwnd, DM_GETDEFID, 0, 0 );
   if ( 0 != nID ) {
      while ( IsWindow( hwnd ) ) {
         HWND hwndDefault = GetDlgItem( hwnd, LOWORD( nID ) );
         if ( IsWindow( hwndDefault ) ) {
            return hwndDefault; //*** FUNCTION EXIT POINT
         }
         if ( WS_CHILD & GetWindowStyle( hwnd ) ) {
            hwnd = GetParent( hwnd );
         } else {
            hwnd = 0;
         }
      }
      return getDescendantWindow( hwnd, nID );
   }
   return 0;
}


/**
 * This function is necessary, as the DS_CENTER style bit centers
 * the dialog in relation to the working area (i.e., the part of
 * the screen not obscured by the tray) rather than in relation
 * to the application window.
 *
 * Dialogs that need more careful positioning (e.g., the replace
 * dialog) can reposition the dialog in onInitDialog.
 * Could get by without storing the size by using SetWindowPos, 
 * but it's not worth the trouble to change this.
 */
void centerDialog( HWND hwndDlg ) {

   assert( IsWindow( hwndDlg ) );

   HWND hwndParent = GetParent( hwndDlg );
   if ( !IsWindow( hwndParent ) ) {
      hwndParent = GetDesktopWindow(); // *Not* HWND_DESKTOP!
   }

   const Rect rcParent = getWindowRect( hwndParent );
   Rect rcDlg = getWindowRect( hwndDlg );

   const int cx = rcDlg.width();
   const int cy = rcDlg.height();

   rcDlg.left = rcParent.left + ( rcParent.width () - cx ) / 2;
   rcDlg.top  = rcParent.top  + ( rcParent.height() - cy ) / 2;

   // Don't cover the parent's title bar, and stay inside left edge:
   const int nTitlebarHeight = GetSystemMetrics( SM_CYSIZE ) + 
                               GetSystemMetrics( SM_CYSIZEFRAME );
   if ( rcDlg.top < rcParent.top + nTitlebarHeight ) {
      rcDlg.top = rcParent.top + nTitlebarHeight;
   }
   const int nFrameWidth = GetSystemMetrics( SM_CXSIZEFRAME );
   if ( rcDlg.left < rcParent.left + nFrameWidth ) {
      rcDlg.left = rcParent.left + nFrameWidth;
   }

   moveWindow( hwndDlg, rcDlg );
   adjustToScreen( hwndDlg );
}


/**
 * Adjust dialog box so that it is inside the screen, giving 
 * priority to the upper left-hand corner (actually, it isn't 
 * reasonable to expect the dialog box to be larger than the 
 * screen). Could get by without storing the size by using 
 * SetWindowPos, but it's not worth the trouble to change this.
 */
void adjustToScreen( HWND hwndDlg ) {

   Rect rcScreen;
   verify( SystemParametersInfo( SPI_GETWORKAREA, 0, &rcScreen, 0 ) );

   Rect rcDlg = getWindowRect( hwndDlg );

   if ( rcScreen.right < rcDlg.right ) {
      rcDlg.left -= rcDlg.right - rcScreen.right;
   }
   if ( rcDlg.left < rcScreen.left ) {
      rcDlg.left = rcScreen.left;
   }

   if ( rcScreen.bottom < rcDlg.bottom ) {
      rcDlg.top -= rcDlg.bottom - rcScreen.bottom;
   }
   if ( rcDlg.top < rcScreen.top ) {
      rcDlg.top = rcScreen.top;
   }

   moveWindow( hwndDlg, rcDlg.left, rcDlg.top );
}


typedef std::map< int, Point > PointMap;
PRIVATE PointMap thePointMap;


void savePosition( HWND hwnd, int id ) {

   Rect rc = getWindowRectInParent( hwnd );
   thePointMap[ id ] = Point( rc.left, rc.top );
}


void restorePosition( HWND hwnd, int id ) {

   assert( IsWindow( hwnd ) );
   centerDialog( hwnd );
   PointMap::iterator iter = thePointMap.find( id );
   if ( thePointMap.end() != iter) {
      MapWindowPoints( 
         GetParent( hwnd ), HWND_DESKTOP, &iter->second, 1 );
      moveWindow( hwnd, iter->second.x, iter->second.y );
   }
   adjustToScreen( hwnd );
}


#ifdef _DEBUG

String _getWindowDescription( HWND hwnd ) {

   if ( HWND_DESKTOP == hwnd ) {
      return _T( "HWND_DESKTOP" );
   }

   if ( !IsWindow( hwnd ) ) {
      return formatMessage( _T( "%1!#x!:<not a window>" ), hwnd );
   }

   TCHAR szText[ 20 ] = { 0 };

#if 0 
   // Causes WM_GETTEXT to be sent, which is not good in 
   // some of the places this function is called from.
   GetWindowText( hwnd, szText, dim (szText ) );
   if ( 15 < _tcslen( szText ) ) {
      _tcscpy( szText + 13, _T( "..." ) );
   }
#endif

   TCHAR szClassName[ 100 ] = { 0 };
   GetClassName( hwnd, szClassName, dim( szClassName ) );

   return formatMessage( _T( "%1!#x! (%2): %3" ), 
      hwnd, szClassName, szText );
}

#endif // _DEBUG
// end of file
