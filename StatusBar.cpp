/*
 * $Header: /Book/StatusBar.cpp 14    16.07.04 10:42 Oslph312 $
 *
 * NOTE: Both Toolbar and Statusbar are tailored to TextEdit.
 * A more general approach would be to derive both from base classes.
 */

#include "precomp.h"
#include "Statusbar.h"
#include "MenuFont.h"
#include "HTML.h"
#include "InstanceSubclasser.h"
#include "formatMessage.h"
#include "formatNumber.h"
#include "graphics.h"
#include "utils.h"
#include "resource.h"


#ifndef SB_SETICON
#define SB_SETICON (WM_USER+15)
#endif


PRIVATE bool sm_bHighlight = false;


PRIVATE LRESULT CALLBACK statusbarParentSpy( 
   HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );


PRIVATE InstanceSubclasser s_parentSubclasser( statusbarParentSpy );


/**
 * Recalculates the sizes of the various status bar parts.
 */
PRIVATE void recalcParts( HWND hwndStatusbar, int nTotalWidth = -1 ) {

   assert( IsWindow( hwndStatusbar ) );

   if ( -1 == nTotalWidth ) {
      const Rect rc = getWindowRect( hwndStatusbar );
      nTotalWidth = rc.width();
   }

   const int nBorder = GetSystemMetrics( SM_CXEDGE );
   const int nWidth = 
      nTotalWidth - GetSystemMetrics( SM_CXVSCROLL ) - nBorder - 1;
   const int nExtra = 4 * nBorder;

   const int nWidthUnicode = 
      measureString( _T( "Unicode" ) ).cx + nExtra;
   const int nWidthPosition = 
      measureString( _T( "Ln 99,999 Col 999  100%" ) ).cx + nExtra;
   const int nWidthToolbarButton = GetSystemMetrics( 
      MenuFont::isLarge() ? SM_CXICON : SM_CXSMICON );
   const int nWidthIcon = nWidthToolbarButton + nExtra;
   
   const int aParts[] = {
      nWidth - nWidthPosition - nWidthUnicode - nWidthIcon, 
      nWidth - nWidthUnicode - nWidthIcon, 
      nWidth - nWidthIcon, 
      nWidth - 0, // If we use -1, we overflow into the sizing grip.
   };

   SNDMSG( hwndStatusbar, SB_SETPARTS, 
      dim( aParts ), reinterpret_cast< LPARAM >( aParts ) );
}


PRIVATE void drawItem( DRAWITEMSTRUCT *pDIS ) {

   // This returns a pointer to the static 
   // szMessageBuffer used in setMessageV:
   LPCTSTR pszText = reinterpret_cast< LPCTSTR >(
      SNDMSG( pDIS->hwndItem, SB_GETTEXT, 0, 0 ) );

   const int nSavedDC = SaveDC( pDIS->hDC );
   if ( sm_bHighlight ) {
      SetTextColor( pDIS->hDC, GetSysColor( COLOR_HIGHLIGHTTEXT ) );
      SetBkColor  ( pDIS->hDC, GetSysColor( COLOR_HIGHLIGHT     ) );
      fillSysColorSolidRect( pDIS->hDC, 
         &pDIS->rcItem, COLOR_HIGHLIGHT );
      pDIS->rcItem.left += GetSystemMetrics( SM_CXEDGE );
   } else {
      SetTextColor( pDIS->hDC, GetSysColor( COLOR_BTNTEXT ) );
      SetBkColor  ( pDIS->hDC, GetSysColor( COLOR_BTNFACE ) );
   }
   const int nHeight = Rect( pDIS->rcItem ).height();
   const int nExtra = nHeight - MenuFont::getHeight();
   pDIS->rcItem.top += nExtra / 2 - 1;
   pDIS->rcItem.left += GetSystemMetrics( SM_CXEDGE );
   paintHTML( pDIS->hDC, pszText, &pDIS->rcItem, 
      GetWindowFont( pDIS->hwndItem ), PHTML_SINGLE_LINE );
   verify( RestoreDC( pDIS->hDC, nSavedDC ) );
}


PRIVATE LRESULT CALLBACK statusbarParentSpy( 
   HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
   const LRESULT lResult = 
      s_parentSubclasser.callOldProc( hwnd, msg, wParam, lParam );
   if ( WM_SIZE == msg ) {
      HWND hwndStatusbar = 
         s_parentSubclasser.getUserDataAsHwnd( hwnd );
      assert( IsWindow( hwndStatusbar ) );
      const int cx = LOWORD( lParam );
      recalcParts( hwndStatusbar, cx );
      SNDMSG( hwndStatusbar, WM_SIZE, wParam, lParam );
   } else if ( WM_DRAWITEM == msg ) {
      drawItem( reinterpret_cast< DRAWITEMSTRUCT * >( lParam ) );
   }
   return lResult;
}


Statusbar::Statusbar( HWND hwndParent, UINT uiID ) 
   : m_hicon( 0 )
   , m_nIndex( 0 )
   , zoomPercentage( 100 )
 {

   assert( 0 != this );

   const HINSTANCE hinst = getModuleHandle();

   attach( CreateWindowEx( 
      0, STATUSCLASSNAME, _T( "" ), 
      SBARS_SIZEGRIP |  WS_CHILD | WS_VISIBLE, 
      0, 0, 0, 0,  hwndParent, (HMENU) uiID, hinst, 0 ) );
   assert( IsWindow( *this ) );
   if ( !IsWindow( *this ) ) {
      throwException( _T( "Unable to create status bar" ) );
   }

   onSettingChange( 0 ); // Sets the font and the parts.
   verify( s_parentSubclasser.subclass( hwndParent, m_hwnd ) );
   setMessage( IDS_READY );
}


Statusbar::~Statusbar() {

   if ( 0 != m_hicon ) {
      DestroyIcon( m_hicon );
   }
}

void __cdecl Statusbar::setMessageV( LPCTSTR pszFmt, va_list vl ) {

   // This *must* be a static buffer. Since the first pane is 
   // SBT_OWNERDRAW, the status bar control doesn't know that 
   // this is text; the lParam is merely 32 arbitrary bits of 
   // application data, and the status bar doesn't retain the 
   // text, just the pointer.
   static TCHAR szMessageBuffer[ 512 ];

   assert( isGoodStringPtr( pszFmt ) );
   if ( isGoodStringPtr( pszFmt ) ) {
      const String strMessage = formatMessageV( pszFmt, vl );
      _tcsncpy_s( szMessageBuffer, strMessage.c_str(), _TRUNCATE );
   } else {
      _tcscpy_s( szMessageBuffer, _T( "Internal Error" ) );
   }

   int nPart = message_part | SBT_OWNERDRAW;
   if ( sm_bHighlight ) {
      nPart |= SBT_NOBORDERS; // SBT_POPOUT
      // This invalidation is necessary for SBT_NOBORDERS.
      // With SBT_POPOUT, it is not necessary.
      Rect rc;
      sendMessage( SB_GETRECT, message_part, reinterpret_cast< LPARAM >( &rc ) );
      InvalidateRect( *this, &rc, TRUE );
   }
   setText( nPart, szMessageBuffer );
}


void __cdecl Statusbar::setMessage( LPCTSTR pszFmt, ... ) {
   
   sm_bHighlight = false;

   va_list vl;
   va_start( vl, pszFmt );
   setMessageV( pszFmt, vl );
   va_end( vl );
}


void __cdecl Statusbar::setMessage( UINT idFmt, ... ) {
   
   sm_bHighlight = false;

   const String strFmt = loadString( idFmt );

   va_list vl;
   va_start( vl, idFmt );
   setMessageV( strFmt.c_str(), vl );
   va_end( vl );
}


void __cdecl Statusbar::setHighlightMessage( UINT idFmt, ... ) {
   
   sm_bHighlight = true;

   const String strFmt = loadString( idFmt );

   va_list vl;
   va_start( vl, idFmt );
   setMessageV( strFmt.c_str(), vl );
   va_end( vl );
}


void __cdecl Statusbar::setErrorMessage( 
   UINT idFlags, UINT idFmt, ... ) 
{
   va_list vl;
   va_start( vl, idFmt );

   MessageBeep( idFlags );

   const String strFmt = loadString( idFmt );
   if ( IsWindowVisible( *this ) ) {
      sm_bHighlight = true;
      setMessageV( strFmt.c_str(), vl );
   } else {
      messageBoxV( GetParent( *this ), MB_OK | idFlags, strFmt.c_str(), vl );
   }
   
   va_end( vl );
}


void Statusbar::update( void ) {

#if 0 // TODO: Unit test of formatNumber
   trace( _T( "testing formatNumber: %d = %s\n" ), 0, formatNumber( 0 ).c_str() );
   trace( _T( "testing formatNumber: %d = %s\n" ), 123, formatNumber( 123 ).c_str() );
   trace( _T( "testing formatNumber: %d = %s\n" ), 12345, formatNumber( 12345 ).c_str() );
   trace( _T( "testing formatNumber: %d = %s\n" ), 1234567890, formatNumber( 1234567890 ).c_str() );
#endif

   const String strLine = formatNumber( position.y + 1 );
   const String strColumn = formatNumber( position.x + 1 );
   const String strPos = formatMessage( IDS_POSITION, strLine.c_str(), strColumn.c_str() );
   const String strZoom = formatMessage( _T( "  %1!d!%%\t" ), this->zoomPercentage );
   const String str = strPos + strZoom;
   setText( position_part, str.c_str() );

   const HWND hwndEditWnd = GetDlgItem( GetParent( *this ), IDC_EDIT );
   if ( GetFocus() == hwndEditWnd ) {
      setMessage( IDS_READY );
   }
}


void Statusbar::update( const Point& updatedPosition ) {

	this->position = updatedPosition;
	update();
}


void Statusbar::update( const int updatedZoomPercentage ) {
	this->zoomPercentage = updatedZoomPercentage;
	update();
}


void Statusbar::setFileType( const bool isUnicode ) {
   
   setText( filetype_part, isUnicode ? _T( "\tUnicode\t" ) : _T( "\tANSI\t" ) );
}


void Statusbar::setIcon( int nIndex ) {

   const int nResource = MenuFont::isLarge() ? 121: 120;
   const HINSTANCE hinst = GetModuleHandle( _T( "COMCTL32" ) );
   const HIMAGELIST hImageList = ImageList_LoadImage( hinst, 
      MAKEINTRESOURCE( nResource ), 
      0, 0, CLR_DEFAULT, IMAGE_BITMAP, 
      LR_DEFAULTSIZE | LR_LOADMAP3DCOLORS );
   setIcon( hImageList, nIndex );
   verify( ImageList_Destroy( hImageList ) );
}


void Statusbar::setIcon( HIMAGELIST hImageList, int nIndex ) {

   if ( 0 != m_hicon ) {
      DestroyIcon( m_hicon );
      m_hicon = 0;
   }

   m_nIndex = nIndex;
   if ( 0 != m_nIndex ) {
      m_hicon = ImageList_GetIcon( hImageList, m_nIndex, ILD_NORMAL );
   }

   sendMessage( SB_SETICON, action_part, reinterpret_cast< LPARAM >( m_hicon ) );
   UpdateWindow( *this );
}


void Statusbar::onSettingChange( LPCTSTR pszSection ) {

   const HFONT hfont = MenuFont::getFont();
   assert( 0 != hfont );
   SetWindowFont( *this, hfont, true );
   setIcon( m_nIndex );
   recalcParts( *this );
}

// end of file
