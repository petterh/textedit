/*
 * $Header: /Book/Toolbar.cpp 25    16.07.04 10:42 Oslph312 $
 *
 * The toolbar automatically uses large icons when the user's
 * selected menu font gets above a suitable treshold. This treshold
 * is controlled by MenuFont::isLarge.
 */

#include "precomp.h"
#include "String.h"
#include "Exception.h"
#include "Editor.h" // TODO: This is dirty. reorg!
#include "Toolbar.h"
#include "Registry.h"
#include "MenuFont.h"
#include "InstanceSubclasser.h"
#include "mainwnd.h"
#include "resource.h"
#include "winUtils.h"
#include "utils.h"
#include "geometry.h"
#include "themes.h"
#include <uxtheme.h>
//#include <Tmschema.h>


PRIVATE LRESULT CALLBACK tabSubclassWndProc(
   HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );
PRIVATE LRESULT CALLBACK toolbarParentSpy(
   HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );


PRIVATE InstanceSubclasser s_tabSubclasser( tabSubclassWndProc );
PRIVATE InstanceSubclasser s_parentSubclasser( toolbarParentSpy );

void Toolbar::onCommand( int id, HWND hwndCtl, UINT codeNotify ) {

   switch ( id ) {
   case IDOK:
      FORWARD_WM_COMMAND(
         GetParent( *this ), ID_SET_TABS, 0, 0, PostMessage );

      //*** FALL THROUGH

   case IDCANCEL:
      if ( BN_CLICKED == codeNotify ) {
         SetFocus( GetParent( *this ) );
      }
      break;

   case IDC_READONLY:
      if ( BN_CLICKED == codeNotify ) {
         FORWARD_WM_COMMAND(
            GetParent( *this ), id, 0, 0, PostMessage );
      }
      break;
   }
}


// LATER: If the toolbar doesn't support drowdown, don't drop down!
bool onDropdown( HWND hwnd, NMTOOLBAR *pNmToolbar ) {

   assert( IsWindow( hwnd ) );
   assert( isGoodPtr( pNmToolbar ) );

   if ( ID_FILE_OPEN == pNmToolbar->iItem ) {
      HWND hwndToolbar = s_parentSubclasser.getUserDataAsHwnd( hwnd );
      assert( IsWindow( hwndToolbar ) );
      HMENU hmenu = CreatePopupMenu();
      if ( 0 != hmenu ) {
         RECT rc;
         SNDMSG( hwndToolbar, TB_GETRECT,
            pNmToolbar->iItem, reinterpret_cast< LPARAM >( &rc ) );

         POINT pt= { rc.left, rc.bottom };
         ClientToScreen( pNmToolbar->hdr.hwndFrom, &pt );

         // The main window's onInitMenu will add files.
         TrackPopupMenu( hmenu,
            TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON,
            pt.x, pt.y, 0, hwnd, 0 );
         DestroyMenu( hmenu );
      } else {
         trace( _T( "onDropdown.CreatePopupMenu failed: %s\n" ),
            WinException().what() );
      }
   }
   return TBDDRET_DEFAULT;
}


/**
 * This is called when the spin buttons attached to the
 * tab field are spun (or if the up/down arrows are used).
 */
void Toolbar::onDeltaPos( NMUPDOWN *pUpDown ) {

   assert( isGoodPtr( pUpDown ) );
   assert( IDC_TABUPDOWN == pUpDown->hdr.idFrom );

   if ( pUpDown->iPos + pUpDown->iDelta <= 4 ) {
      ;
   } else if ( pUpDown->iPos - (pUpDown->iDelta < 0 ) < 8 ) {
      if ( pUpDown->iDelta < 0 ) {
         pUpDown->iDelta = 4 - pUpDown->iPos;
      } else {
         pUpDown->iDelta = 8 - pUpDown->iPos;
      }
   } else if ( pUpDown->iPos - (pUpDown->iDelta < 0 ) < 16 ) {
      if ( pUpDown->iDelta < 0 ) {
         pUpDown->iDelta = 8 - pUpDown->iPos;
      } else {
         pUpDown->iDelta = 16 - pUpDown->iPos;
      }
   } else {
      pUpDown->iDelta = 16 - pUpDown->iPos;
   }
   PostMessage( GetDlgItem( *this, IDC_TABEDIT ), EM_SETSEL, 0, -1 );
}


/**
 * Get tooltip text.
 */
// TODO: Unit test new safe string API
bool Toolbar::onGetDispInfo( NMTTDISPINFO *pDispInfo ) {

   assertValid();
   assert( isGoodPtr( pDispInfo ) );

   // NOTE: The hwndFrom member refers to the tooltip control.
   const HWND hwndCtl = reinterpret_cast< HWND >( pDispInfo->hdr.idFrom );
   if ( GetDlgItem( *this, IDC_TABEDIT ) == hwndCtl ) {
      pDispInfo->lpszText = MAKEINTRESOURCE( IDS_TABS_TIP );
   } else if ( GetDlgItem( *this, IDC_TABUPDOWN ) == hwndCtl ) {
      pDispInfo->lpszText = MAKEINTRESOURCE( IDS_TABS_UPDOWN_TIP );
   } else if ( GetDlgItem( *this, IDC_READONLY ) == hwndCtl ) {
      pDispInfo->lpszText = MAKEINTRESOURCE( IDS_READ_ONLY_TIP );
   } else {
      String strToolTip = loadToolTip( pDispInfo->hdr.idFrom );
      pDispInfo->lpszText = pDispInfo->szText;
      _tcsncpy_s(pDispInfo->szText, strToolTip.c_str(), dim(pDispInfo->szText));
      pDispInfo->hinst = 0;
   }

   return true;
}


#ifndef UNICODE
bool Toolbar::onGetDispInfoW( NMTTDISPINFOW *pDispInfo ) {

   assertValid();
   assert( isGoodPtr( pDispInfo ) );

   // NOTE: The hwndFrom member refers to the tooltip control.
   const HWND hwndCtl =
      reinterpret_cast< HWND >( pDispInfo->hdr.idFrom );
   if ( GetDlgItem( *this, IDC_TABEDIT ) == hwndCtl ) {
      pDispInfo->lpszText = MAKEINTRESOURCEW( IDS_TABS_TIP );
   } else if ( GetDlgItem( *this, IDC_TABUPDOWN ) == hwndCtl ) {
      pDispInfo->lpszText = MAKEINTRESOURCEW( IDS_TABS_UPDOWN_TIP );
   } else if ( GetDlgItem( *this, IDC_READONLY ) == hwndCtl ) {
      pDispInfo->lpszText = MAKEINTRESOURCEW( IDS_READ_ONLY_TIP );
   } else {
      StringA strToolTip = loadToolTip( pDispInfo->hdr.idFrom );
      pDispInfo->lpszText = pDispInfo->szText;
	  multiByteToWideChar( strToolTip.c_str(), pDispInfo->szText );
      pDispInfo->hinst = 0;
   }

   return true;
}
#endif


LRESULT Toolbar::onNotify( const int id, const LPNMHDR pHdr ) {

#ifdef _DEBUG

#ifdef UNICODE
	trace( L"UNICODE\n" );
#else
	trace( "NOT UNICODE\n" );
#endif

#ifdef _UNICODE
	trace( L"_UNICODE\n" );
#else
	trace( "NOT _UNICODE\n" );
#endif

	trace( _T( "TTN_GETDISPINFO  = %x\n" ), TTN_GETDISPINFO );
	trace( _T( "TTN_GETDISPINFOA = %x\n" ), TTN_GETDISPINFOA );
	trace( _T( "TTN_GETDISPINFOW = %x\n" ), TTN_GETDISPINFOW );
#endif

	assert( isGoodPtr( pHdr ) );

	switch ( pHdr->code ) {
	case UDN_DELTAPOS:
		onDeltaPos( reinterpret_cast< NMUPDOWN * >( pHdr ) );
		return 0;

// TODO: Document: If running with a manifest for Common controls 6, 
// we only get the unicode message.
#ifndef UNICODE
	case TTN_GETDISPINFOW:
		return onGetDispInfoW(
			reinterpret_cast< NMTTDISPINFOW * >( pHdr ) );
#endif

	case TTN_GETDISPINFO:
		return onGetDispInfo(
			reinterpret_cast< NMTTDISPINFO * >( pHdr ) );

	case TBN_DROPDOWN:  // Never gets here; sent to parent,
		assert( false ); // intercepted in toolbarParentSpy.
	}

	return Window::onNotify( id, pHdr );
}


int Toolbar::commandFromIndex( int index ) {

   assertValid();
   TBBUTTON button = { 0 };
   sendMessage(
      TB_GETBUTTON, index, reinterpret_cast< LPARAM >( &button ) );
   return button.idCommand;
}


#ifndef TB_HITTEST
#define TB_HITTEST (WM_USER + 69)
#endif


void Toolbar::onLButtonDown(
   BOOL fDoubleClick, int x, int y, UINT keyFlags )
{
   assertValid();
   Window::onLButtonDown( fDoubleClick, x, y, keyFlags );

   if ( fDoubleClick ) {
      POINT pt = { x, y };
      const int index = sendMessage(
         TB_HITTEST, 0, reinterpret_cast< LPARAM >( &pt ) );
      switch ( commandFromIndex( index ) ) {
      case ID_VIEW_PROPORTIONALFONT:
         FORWARD_WM_COMMAND( GetParent( *this ),
            ID_VIEW_SETPROPORTIONALFONT, 0, 0, PostMessage );
         break;
      case ID_VIEW_FIXEDFONT:
         FORWARD_WM_COMMAND( GetParent( *this ),
            ID_VIEW_SETFIXEDFONT, 0, 0, PostMessage );
         break;
      }
   }
}


PRIVATE LRESULT CALLBACK tabSubclassWndProc(
   HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
   Editor *pEditor = reinterpret_cast< Editor * >(
      s_tabSubclasser.getUserData( hwnd ) );
   assert( isGoodPtr( pEditor ) );

   // TODO: Should reorg this as a filter, or subclass both controls!
   if ( WM_KEYDOWN == msg || WM_KEYUP == msg ) {
      if ( VK_NEXT == wParam || VK_PRIOR == wParam ) {
         pEditor->getEditWnd()->sendMessage( msg, wParam, lParam );
      }
   }

   LRESULT lResult =
      s_tabSubclasser.callOldProc( hwnd, msg, wParam, lParam );
   if ( WM_GETDLGCODE == msg ) {
      lResult &= ~(DLGC_WANTTAB | DLGC_WANTALLKEYS);
   } else if ( WM_SETFOCUS == msg ) {
      pEditor->getStatusbar()->setMessage( IDS_TAB_PROMPT );
   } else if ( WM_KILLFOCUS == msg ) {
      pEditor->getStatusbar()->update(
         pEditor->getEditWnd()->getCurPos() );
   }
   return lResult;
}
// TODO: On leaving the field, set contents to actual tab


PRIVATE LRESULT CALLBACK toolbarParentSpy(
   HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
   if ( WM_SIZE == msg ) {
      HWND hwndToolbar = s_parentSubclasser.getUserDataAsHwnd( hwnd );
      assert( IsWindow( hwndToolbar ) );
      SNDMSG( hwndToolbar, WM_SIZE, wParam, lParam );
   } else if ( WM_NOTIFY == msg ) {
      NMHDR *pHdr = reinterpret_cast< NMHDR * >( lParam );
      if ( TBN_DROPDOWN == pHdr->code ) {
         return onDropdown( hwnd,
            reinterpret_cast< NMTOOLBAR * >( pHdr ) );
      }
   }
   return s_parentSubclasser.callOldProc( hwnd, msg, wParam, lParam );
}


Toolbar::Toolbar(
   Editor *pEditor, UINT uiID, bool hasRedo, bool canSetTabs )
{
   assert( isGoodPtr( this ) );

   const HINSTANCE hinst = getModuleHandle();

   attach( CreateWindowEx(
      0, TOOLBARCLASSNAME, 0, WS_CHILD | WS_VISIBLE | WS_TOOLBAR,
      0, 0, 0, 0, pEditor->getMainWnd(), (HMENU) uiID, hinst, 0 ) );
   assert( IsWindow( *this ) );
   if ( !IsWindow( *this ) ) {
      throwException( _T( "Failed to create toolbar" ) );
   }

   sendMessage( TB_BUTTONSTRUCTSIZE, sizeof( TBBUTTON ) );
   sendMessage( TB_SETINDENT, GetSystemMetrics( SM_CXEDGE ) );

#if ( 0x0400 <= _WIN32_IE ) && defined( TBSTYLE_EX_DRAWDDARROWS )
   sendMessage( TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_DRAWDDARROWS );
#endif

   HWND hwndTabs      = 0;
   HWND hwndUpDown    = 0;
   if ( canSetTabs ) {
#ifdef _DEBUG
      HWND hwndTabsLabel =
#endif
         CreateWindowEx(
             0, _T( "STATIC" ), loadString( IDS_TABS_LABEL ).c_str(),
             WS_CHILD | WS_VISIBLE | SS_LEFT,
             0, 0, 0, 0, m_hwnd, (HMENU) IDC_TABLABEL, hinst, 0 );
      assert( IsWindow( hwndTabsLabel ) );

      hwndTabs = CreateWindowEx(
         WS_EX_CLIENTEDGE, _T( "EDIT" ), 0,
         WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_GROUP |
         ES_AUTOHSCROLL | ES_WANTRETURN | ES_MULTILINE |
         ES_RIGHT | ES_NUMBER,
         0, 0, 0, 0, m_hwnd, (HMENU) IDC_TABEDIT, hinst, 0 );
      assert( IsWindow( hwndTabs ) );

      hwndUpDown = CreateWindowEx(
         0, UPDOWN_CLASS, 0,
         WS_CHILD | WS_VISIBLE | WS_GROUP |
         UDS_AUTOBUDDY | UDS_SETBUDDYINT |
         UDS_ALIGNRIGHT | UDS_ARROWKEYS | UDS_HOTTRACK,
         0, 0, 0, 0, m_hwnd, (HMENU) IDC_TABUPDOWN, hinst, 0 );
      assert( IsWindow( hwndUpDown ) );

      SNDMSG( hwndUpDown,
         UDM_SETRANGE, 0, (LPARAM) MAKELONG( 16, 1 ) );
      SNDMSG( hwndUpDown,
         UDM_SETPOS  , 0, (LPARAM) MAKELONG(  4, 0 ) );
   }

   HWND hwndReadOnly = CreateWindowEx(
      0, _T( "BUTTON" ), loadString( IDS_READ_ONLY ).c_str(),
      WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_GROUP | BS_AUTOCHECKBOX,
      0, 0, 0, 0, m_hwnd, (HMENU) IDC_READONLY, hinst, 0 );
   assert( IsWindow( hwndReadOnly ) );

   // Add tool tips for the child controls:
   HWND hwndToolTip =
      reinterpret_cast< HWND >( sendMessage( TB_GETTOOLTIPS ) );
   if ( IsWindow( hwndToolTip ) ) {
      TOOLINFO toolInfo = {
         sizeof( TOOLINFO ),
         TTF_CENTERTIP | TTF_IDISHWND |
         TTF_SUBCLASS | TTF_TRANSPARENT,
         *this, reinterpret_cast< UINT >( hwndReadOnly ), { 0 },
         hinst, LPSTR_TEXTCALLBACK ,
      };
      SNDMSG( hwndToolTip,
         TTM_ADDTOOL, 0, reinterpret_cast< WPARAM >( &toolInfo ) );
      if ( canSetTabs ) {
         toolInfo.uId = reinterpret_cast< UINT >( hwndTabs );
         SNDMSG( hwndToolTip,
            TTM_ADDTOOL, 0, reinterpret_cast< WPARAM >( &toolInfo ) );
         toolInfo.uId = reinterpret_cast< UINT >( hwndUpDown );
         SNDMSG( hwndToolTip,
            TTM_ADDTOOL, 0, reinterpret_cast< WPARAM >( &toolInfo ) );
      }

#ifdef _DEBUG
      const LONG lToolTipWndProc =
         GetWindowLong( hwndToolTip, GWL_WNDPROC );
      trace( _T( "lToolTipWndProc = %#x\n" ), lToolTipWndProc );
#endif
   }

   adjust( false, hasRedo, canSetTabs );

   if ( canSetTabs ) {
      // This should be done *after* we've created the tool
      // tips. The tool tip window subclasses hwndTabs, but
      // does not detach on window destruction. It's better
      // if we're on top of the food chain.
      verify( s_tabSubclasser.subclass( hwndTabs, pEditor ) );
   }

   // This may already be subclassed if we're recreating the toolbar,
   // which happens on WM_SETTINGCHANGE. Doesn't matter.
   s_parentSubclasser.subclass( pEditor->getMainWnd(), m_hwnd );
   assertValid();
}


void Toolbar::setButtons( bool hasRedo ) {

   assertValid();

   TBADDBITMAP tbabmpStd = {
      HINST_COMMCTRL,
      (UINT_PTR) (MenuFont::isLarge() ? IDB_STD_LARGE_COLOR : IDB_STD_SMALL_COLOR),
   };
   TBADDBITMAP tbabmpCustom = {
      getModuleHandle(),
      (UINT_PTR) (MenuFont::isLarge() ? IDR_TOOLBAR_LARGE : IDR_TOOLBAR),
   };

   static const TBBUTTON tb[] = {
     { STD_FILENEW , ID_FILE_NEW   , TBSTATE_ENABLED, TBSTYLE_BUTTON   },
     { 0           , 0             , 0              , TBSTYLE_SEP      },
     { STD_FILEOPEN, ID_FILE_OPEN  , TBSTATE_ENABLED, TBSTYLE_DROPDOWN },
     { 0           , 0             , 0              , TBSTYLE_SEP      },
     { STD_PRINT   , ID_FILE_PRINT , TBSTATE_ENABLED                   },
     { 0           , 0             , 0              , TBSTYLE_SEP      },
     { STD_DELETE  , ID_EDIT_DELETE, TBSTATE_ENABLED                   },
     { STD_CUT     , ID_EDIT_CUT   , TBSTATE_ENABLED, TBSTYLE_BUTTON   },
     { STD_COPY    , ID_EDIT_COPY  , TBSTATE_ENABLED, TBSTYLE_BUTTON   },
     { STD_PASTE   , ID_EDIT_PASTE , TBSTATE_ENABLED, TBSTYLE_BUTTON   },
     { 0           , 0             , 0              , TBSTYLE_SEP      },
     { find_icon   , ID_EDIT_FIND  , TBSTATE_ENABLED                   },
     { 0           , 0             , 0              , TBSTYLE_SEP      },
     { STD_UNDO    , ID_EDIT_UNDO  , 0              , TBSTYLE_BUTTON   },
     { STD_REDOW   , ID_EDIT_REDO  , 0              , TBSTYLE_BUTTON   },
     { 0           , 0             , 0              , TBSTYLE_SEP      },

     { prop_icon, ID_VIEW_PROPORTIONALFONT,
                                   TBSTATE_ENABLED, TBSTYLE_CHECKGROUP },
     { fixed_icon  , ID_VIEW_FIXEDFONT,
                                   TBSTATE_ENABLED, TBSTYLE_CHECKGROUP },
     { 0           , 0             , 0              , TBSTYLE_SEP      },
     { wordwrap_icon,ID_VIEW_WORDWRAP, TBSTATE_ENABLED, TBSTYLE_CHECK  },

     { 0       , ID_TABPLACEHOLDER   , 0              , TBSTYLE_SEP    },
   };

   sendMessage(
      TB_ADDBITMAP, 15, reinterpret_cast< LPARAM >( &tbabmpStd ) );
   sendMessage(
      TB_ADDBITMAP,  4, reinterpret_cast< LPARAM >( &tbabmpCustom ) );
   sendMessage(
      TB_ADDBUTTONS, dim( tb ), reinterpret_cast< LPARAM >( tb ) );
   if ( !hasRedo ) {
      const int index =
         sendMessage( TB_COMMANDTOINDEX, ID_EDIT_REDO );
      assert( 0 <= index );
      verify( sendMessage( TB_DELETEBUTTON, index ) );
   }
   sendMessage( TB_AUTOSIZE );
}


Toolbar::~Toolbar() {
}


SIZE measureDlgItem( HWND hwndCtl ) {

   assert( IsWindow( hwndCtl ) );
   TCHAR szText[ 100 ] = { 0 };
   GetWindowText( hwndCtl, szText, dim( szText ) );
   return measureString( szText );
}


SIZE getEditSize( HWND hwndEdit, LPCTSTR pszStipulatedContents ) {

   assert( IsWindow( hwndEdit ) );
   assert( isGoodStringPtr( pszStipulatedContents ) );
   SIZE size = measureString( pszStipulatedContents );

   const int nXEdge2 = 2 * GetSystemMetrics( SM_CXEDGE );
   const int nYEdge2 = 2 * GetSystemMetrics( SM_CYEDGE );

   size.cx += 2 + nXEdge2;
   size.cy += 2 + nYEdge2;

   return size;
}


void Toolbar::adjust( bool bRepaint, bool hasRedo, bool canSetTabs ) {

   assertValid();
   setButtons( hasRedo );
   setFonts();

   const int nPad = GetSystemMetrics( SM_CXDLGFRAME );

   Rect rc = getWindowRect( *this );
   const int nHeight = rc.height();
   sendMessage( TB_GETRECT, ID_TABPLACEHOLDER, (LPARAM) &rc );
   OffsetRect( &rc, nPad, 0 );

   if ( canSetTabs ) {
      HWND hwndTabsLabel  = GetDlgItem( *this, IDC_TABLABEL  );
      HWND hwndTabs       = GetDlgItem( *this, IDC_TABEDIT   );
      HWND hwndUpDown     = GetDlgItem( *this, IDC_TABUPDOWN );
      const SIZE sizeTabsLabel  = measureDlgItem( hwndTabsLabel );
      const SIZE sizeTabs = getEditSize( hwndTabs, _T( "99999" ) );
      Rect rcUpDown = getWindowRect( hwndUpDown );
      const SIZE sizeUpDown = {
         rcUpDown.right  - rcUpDown.left,
         rcUpDown.bottom - rcUpDown.top };

      MoveWindow(
         hwndTabsLabel, rc.right, (nHeight - sizeTabsLabel.cy) / 2,
         sizeTabsLabel.cx, sizeTabsLabel.cy, bRepaint );
      rc.left = rc.right;
      OffsetRect( &rc, sizeTabsLabel.cx + nPad, 0 );

      const int nTabExtra = 4;
      MoveWindow( hwndTabs, rc.left, (nHeight - sizeTabs.cy) / 2,
         sizeTabs.cx + sizeUpDown.cx + nTabExtra, sizeTabs.cy,
         bRepaint );
      SNDMSG( hwndUpDown, UDM_SETBUDDY,
         reinterpret_cast< WPARAM >( hwndTabs ), 0 );
      OffsetRect( &rc, sizeTabs.cx + sizeUpDown.cx + 4 + nPad, 0 );

      rcUpDown = getWindowRect( hwndUpDown );
      const RECT rcEditRect = {
         0, 0, sizeTabs.cx - rcUpDown.width(), sizeTabs.cy };
      SNDMSG( hwndTabs,
         EM_SETRECT, 0, reinterpret_cast< LPARAM >( &rcEditRect ) );
   }

   HWND hwndReadOnly = GetDlgItem( *this, IDC_READONLY  );
   SIZE sizeReadOnly = measureDlgItem( hwndReadOnly );
   MoveWindow( hwndReadOnly,
      rc.left + 10, (nHeight - sizeReadOnly.cy) / 2,
      sizeReadOnly.cx + 50, sizeReadOnly.cy, bRepaint );
}


int Toolbar::getTabs( void ) {

   assertValid();
   BOOL bOK = false;
   int nTabs = GetDlgItemInt( *this, IDC_TABEDIT, &bOK, false );
   return nTabs;
}


void Toolbar::setSpacesPerTab( int nSpacesPerTab ) {
   HWND hwndTabsUpDown = getChild( IDC_TABUPDOWN );
   assert( IsWindow( hwndTabsUpDown ) );
   SNDMSG( hwndTabsUpDown,
      UDM_SETPOS, 0, (LPARAM) MAKELONG( nSpacesPerTab, 0 ) );
}


void Toolbar::setReadOnly( bool bReadOnly, bool bAccessDenied ) {

   HWND hwndReadOnly = getChild( IDC_READONLY );
   assert( IsWindow( hwndReadOnly ) );

   // If you really want to be paranoid, verify that it is,
   // indeed, a checkbox.

   Button_SetCheck( hwndReadOnly, bReadOnly ? 1 : 0 );
   Button_Enable( hwndReadOnly, !bAccessDenied );
}


bool Toolbar::getReadOnly( void ) const {
   HWND hwndReadOnly = getChild( IDC_READONLY );
   return 0 != Button_GetCheck( hwndReadOnly );
}


void Toolbar::setFonts( void ) {

   assertValid();
   HFONT hfont = MenuFont::getFont();
   assert( 0 != hfont );
   //messageBox( HWND_DESKTOP, MB_OK | MB_TASKMODAL | MB_SETFOREGROUND, _T( "Toolbar::setFonts" ) );
   //assert( FALSE );

#if 0
   static int count = 0;
   ++count;
   if ( 1 < count ) {
       debugBreak();
   }
#endif

   for ( HWND hwndChild = GetTopWindow( *this );
      IsWindow( hwndChild );
      hwndChild = GetNextWindow( hwndChild, GW_HWNDNEXT ) )
   {
      SetWindowFont( hwndChild, hfont, true );
   }
}


void Toolbar::onSettingChange( LPCTSTR pszSection ) {

   assertValid();
   //FORWARD_WM_SETTINGCHANGE( hwndChild, pszSection, SNDMSG );
   if ( isGoodStringPtr( pszSection ) && 0 == _tcsicmp( _T( "WindowMetrics" ), pszSection ) ) {
      //adjust( true );
   }
}


LRESULT Toolbar::dispatch( UINT msg, WPARAM wParam, LPARAM lParam ) {

   /*const*/ LRESULT result = Window::dispatch( msg, wParam, lParam );
#if 0
   if ( WM_CTLCOLORSTATIC == msg ) {
      HTHEME ht = openThemeData( *this, L"Toolbar" ); // TOOLBARCLASSNAMEW
      if ( 0 != ht ) {
         COLORREF cr = 0;
         const HRESULT hres = getThemeColor( ht,
            TP_BUTTON, TS_NORMAL, TMT_COLOR, &cr );
         if ( SUCCEEDED( hres ) ) {
            result = (LRESULT) CreateSolidBrush( cr );
         }
         closeThemeData( ht );
      }
   }
#endif
   return result;
}

// end of file
