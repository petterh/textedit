/*
 * $Header: /Book/mainwnd.cpp 31    16.07.04 10:42 Oslph312 $
 */

#include "precomp.h"
#include <mapi.h>
#include "mainwnd.h"
#include "Editor.h"
#include "Dialog.h"
#include "OptionsDlg.h"
#include "AboutDlg.h"
#include "FindDlg.h"
#include "DeleteDlg.h"
#include "FontDlg.h"
#include "PropertiesDlg.h"
#include "Toolbar.h"
#include "Registry.h"
#include "WaitCursor.h"
#include "Exception.h"
#include "AutoArray.h"
#include "AutoLibrary.h"
#include "Statusbar.h"
#include "MenuFont.h"
#include "MRU.h"
#include "saveFile.h"
#include "openFileDlg.h"
#include "printFile.h"
#include "setupPage.h"
#include "createNewFile.h"
#include "startInstance.h"
#include "formatMessage.h"
#include "fileUtils.h"
#include "menuUtils.h"
#include "winUtils.h"
#include "init.h"
#include "help.h"
#include "utils.h"
#include "os.h"
#include "persistence.h"
#include "timers.h"
#include "resource.h"
#include "wnd_snap.h"

#include "Help/map.hh"


#define HANDLE_WM_ENTERSIZEMOVE(  \
   hwnd, wParam, lParam, fn ) ((fn)(hwnd), 0L)
#define HANDLE_WM_EXITSIZEMOVE(   \
   hwnd, wParam, lParam, fn ) ((fn)(hwnd), 0L)
#define HANDLE_WM_POWERBROADCAST( hwnd, wParam, lParam, fn ) \
   (fn)((hwnd), (DWORD)(wParam), (DWORD)(wParam))
#define HANDLE_WM_HELP( hwnd, wParam, lParam, fn ) \
   (fn)((hwnd), reinterpret_cast< HELPINFO * >( lParam ))
#define HANDLE_WM_APP( hwnd, wParam, lParam, fn ) \
   (fn)((hwnd), static_cast< ATOM >( lParam ))


PRIVATE String s_strLast;
PRIVATE HWND s_hwndNextClipboardViewer;


PRIVATE void onSize( HWND hwnd, UINT state, int cx, int cy );


PRIVATE inline Toolbar *getToolbar( HWND hwnd ) {

   Editor *pEditor = getEditor( hwnd );
   assert( 0 != pEditor );
   Toolbar *pToolbar = pEditor->getToolbar();
   assert( 0 != pToolbar && IsWindow( *pToolbar ) );
   return pToolbar;
}


PRIVATE inline AbstractEditWnd *getEditWnd( HWND hwnd ) {

   Editor *pEditor = getEditor( hwnd );
   assert( 0 != pEditor );
   AbstractEditWnd *pEditWnd = pEditor->getEditWnd();
   assert( 0 != pEditWnd && IsWindow( *pEditWnd ) );
   return pEditWnd;
}


PRIVATE inline Document *getDocument( HWND hwnd ) {
   Editor *pEditor = getEditor( hwnd );
   assert( 0 != pEditor );
   return pEditor->getDocument();
}


PRIVATE Statusbar *getStatusbar( HWND hwnd ) {
   Editor *pEditor = getEditor( hwnd );
   assert( 0 != pEditor );
   return pEditor->getStatusbar();
}


PRIVATE void setFont( const HWND hwnd, bool bFixed ) {

   getEditor( hwnd )->setFont( bFixed );
}


PRIVATE void enablePaste( HWND hwnd ) {
   const bool bCanPaste =
      IsClipboardFormatAvailable( CF_TEXT ) &&
      !getEditWnd( hwnd )->isReadOnly();
   getToolbar( hwnd )->setEnabled( ID_EDIT_PASTE, bCanPaste );
   enableMenuItem( GetMenu( hwnd ), ID_EDIT_PASTE, bCanPaste );
}


/**
 * Called when the size of the docked windows might
 * have changed (from onSettingChange) or when one
 * of them are shown or hidden (toggleDockedWindow).
 * (Docked windows, who dat? Toolbar and Statusbar, dat who.)
 */
PRIVATE void recalcLayout( const HWND hwnd ) {

   if ( !IsIconic( hwnd ) ) {
      const Rect rc = getClientRect( hwnd );
      onSize( hwnd, SIZE_RESTORED,
         rc.right - rc.left, rc.bottom - rc.top );
   }
   InvalidateRect( *getEditor( hwnd )->getToolbar(), 0, 0 );
}


PRIVATE inline void setEditor( HWND hwnd, Editor *pEditor ) {

   assert( 0 == pEditor || IsWindow( *pEditor->getEditWnd() ) );
   if ( 0 == pEditor ) {
      getEditor( hwnd )->detach( hwnd );
   }
   SetWindowLong( hwnd, EDITOR_OFFSET,
      reinterpret_cast< LONG >( pEditor ) );
}


PRIVATE void removeSendToSubMenu( HMENU hmenu ) {

   assert( IsMenu( hmenu ) );
   HMENU hmenuFile = GetSubMenu( hmenu, 0 );
   assert( IsMenu( hmenuFile ) );
   const int nItems = GetMenuItemCount( hmenuFile );
   for ( int iItem = 0; iItem < nItems; ++iItem ) {
      HMENU hmenuSub = GetSubMenu( hmenuFile, iItem );
      if ( IsMenu( hmenuSub ) && containsItem( hmenuSub, ID_FILE_SENDTO_MAILRECIPIENT ) ) {
         verify( DeleteMenu( hmenuFile, iItem, MF_BYPOSITION ) );
         break; //*** LOOP EXIT POINT
      }
   }
}


PRIVATE void checkMAPI( HMENU hmenu ) {

   const bool hasMapiInProfile = 0 != GetProfileInt( _T( "Mail" ), _T( "MAPI" ), 0 );
   const bool foundMapiDll = 0 != SearchPath( 0, _T( "MAPI32.DLL" ), 0, 0, 0, 0 );
   const bool isMailAvailable = hasMapiInProfile && foundMapiDll;
   if ( !isMailAvailable ) {
      removeSendToSubMenu( hmenu );
   }
}


/*
 * Message handlers:
 */

PRIVATE bool onCreate( HWND hwnd, CREATESTRUCT FAR* lpCreateStruct ) {

   AutoDocument *ppDocument = reinterpret_cast< AutoDocument * >(
      lpCreateStruct->lpCreateParams );
   assert( 0 != ppDocument );

   Editor *pEditor = new Editor( hwnd, ppDocument );
   setEditor( hwnd, pEditor );

   // NOTE: From this point until the AutoEditor in WinMain has
   // been initialized, pEditor is vulnerable to leakage.
   // The risk is, however, quite low, and no user data would
   // be lost in any case.

   const Document  *pDocument  = pEditor->getDocument ();
   AbstractEditWnd *pEditWnd   = pEditor->getEditWnd  ();
   Statusbar       *pStatusbar = pEditor->getStatusbar();

   HMENU hmenu = GetMenu( hwnd );
   if ( !pEditWnd->hasRedo() ) {
      verify( DeleteMenu( hmenu, ID_EDIT_REDO, MF_BYCOMMAND ) );
   }

   checkMAPI( hmenu );

   pEditWnd->setFirstVisibleLine( pDocument->getFirstLine() );
   pEditWnd->setSel( pDocument->getSelStart(), pDocument->getSelEnd() );
   pEditor->setSettings();
   pStatusbar->update( pEditWnd->getCurPos() );
   if ( pDocument->isBinary() ) {
      pStatusbar->setHighlightMessage( IDS_BINARY );
   }
   enablePaste( hwnd );

   s_hwndNextClipboardViewer = SetClipboardViewer( hwnd );
   DragAcceptFiles( hwnd, true );

   setSnap( hwnd, true );

   return TRUE;
}


PRIVATE void onActivate(
   HWND hwnd, UINT state, HWND hwndActDeact, BOOL fMinimized )
{
   if ( WA_INACTIVE == state ) {
      getEditor( hwnd )->saveIfNecessary();
   }
}


PRIVATE void onDestroy( const HWND hwnd ) {

   getEditor( hwnd )->saveState();
   assert( getEditor( hwnd )->isClean() );

   // SetMenu results in a number of messages, including
   // WM_SIZE. onSize throws a NullPointerException if
   // there is no Editor object, so the call to SetMenu
   // *must* come before we reset the Editor.
   verify( DestroyMenu( GetMenu( hwnd ) ) );
   SetMenu( hwnd, 0 );
   setEditor( hwnd, 0 );

   WinHelp( hwnd, getHelpFile(), HELP_QUIT, 0 );
   ChangeClipboardChain( hwnd, s_hwndNextClipboardViewer );
   DragAcceptFiles( hwnd, false );
   PostQuitMessage( 0 ); // Goodbye, and thanks for all the fish!
}


PRIVATE void onClose ( HWND hwnd ) {

   getEditor( hwnd )->saveIfNecessary();
   DestroyWindow( hwnd );
}


PRIVATE void openFile( HWND hwnd, const String& str ) {

   Editor *pEditor = getEditor( hwnd );
   if ( pEditor->saveIfNecessary() ) {
      pEditor->openFile( str );
   }
   enablePaste( hwnd );
}


class AutoDrop {
private:
   HDROP m_hdrop;
public:
   AutoDrop( HDROP hdrop ) : m_hdrop( hdrop ) {
   }
   ~AutoDrop() {
      assert( 0 != m_hdrop );
      DragFinish( m_hdrop );
   }
};


PRIVATE void onDropFiles( HWND hwnd, HDROP hdrop ) {

   AutoDrop autoDrop( hdrop );

   const UINT DRAGQUERY_NUMFILES = (UINT) -1;
   const int nFiles = DragQueryFile(
      hdrop, DRAGQUERY_NUMFILES, 0, 0 );
   for ( int iFile = nFiles - 1; 0 <= iFile; --iFile ) {
      PATHNAME szDragFile = { 0 };
      verify( 0 < DragQueryFile(
         hdrop, iFile, szDragFile, dim( szDragFile ) ) );
      if ( 0 == iFile ) {
         openFile( hwnd, szDragFile );
      } else {
         const String strArg =
            formatMessage( _T( "\"%1\"" ), szDragFile );
         startInstance( strArg );
      }
   }
}


PRIVATE void onTimer( HWND hwnd, UINT id ) {

   switch ( id ) {
   case timer_save_state:
      KillTimer( hwnd, timer_save_state );
      getEditor( hwnd )->saveState();
      break;

   default:
      // This will cause any defined callback to be called.
      FORWARD_WM_TIMER( hwnd, id, DefWindowProc );
      break;
   }
}


PRIVATE void onSize( HWND hwnd, UINT state, int cx, int cy ) {

#if 0 // Both children subclass us, so no need to pass this on.
   FORWARD_WM_SIZE( *getStatusbar( hwnd ), state, cx, cy, SNDMSG );
   FORWARD_WM_SIZE(   *getToolbar( hwnd ), state, cx, cy, SNDMSG );
#endif

   if ( SIZE_MINIMIZED != state ) {
      if ( IsWindowVisible( *getStatusbar( hwnd ) ) ) {
         const Rect rc = getWindowRect( *getStatusbar( hwnd ) );
         cy -= rc.height();
      }

      int y = 0;
      if ( IsWindowVisible( *getToolbar( hwnd ) ) ) {
         const Rect rc = getWindowRect( *getToolbar( hwnd ) );
         y = rc.height();
         cy -= y;
      }

      AbstractEditWnd *pEditWnd = getEditWnd( hwnd );
      MoveWindow( *pEditWnd, 0, y, cx, cy, true );
   }

   // One might think that it would be more efficient to just
   // saveState for SIZE_MINIMIZED and SIZE_MAXIMIZED here, using
   // onExitSizeMove to save the state for SIZE_RESTORED.
   // Unfortunately, if you select a window tiling command from the
   // task bar context menu, WM_EXITSIZEMOVE is never sent.

   SetTimer( hwnd, timer_save_state, 500, 0 );
}


PRIVATE void onMove( HWND hwnd, int x, int y ) {

   SetTimer( hwnd, timer_save_state, 500, 0 );
}


PRIVATE void onEnterSizeMove( const HWND hwnd ) {
   HideCaret( 0 );
}


PRIVATE void onExitSizeMove( const HWND hwnd ) {
   ShowCaret( 0 );
}


// TODO: Retain focus while deactivated?
PRIVATE void onSetFocus( const HWND hwnd, HWND /* hwndOldFocus */ ) {

   AbstractEditWnd *pEditWnd = getEditWnd( hwnd );
   if ( IsWindowVisible( *pEditWnd ) ) {
      SetFocus( *pEditWnd );
   }
}


PRIVATE void onDrawClipboard( HWND hwnd ) {

   if ( IsWindow( s_hwndNextClipboardViewer ) ) {
      FORWARD_WM_DRAWCLIPBOARD( s_hwndNextClipboardViewer, SNDMSG );
   }
   enablePaste( hwnd );
}


PRIVATE void onChangeCBChain(
   HWND hwnd, HWND hwndRemove, HWND hwndNext )
{
   if ( s_hwndNextClipboardViewer == hwndRemove ) {
      s_hwndNextClipboardViewer = hwndNext;
   } else {
      FORWARD_WM_CHANGECBCHAIN( s_hwndNextClipboardViewer,
         hwndRemove, hwndNext, SNDMSG );
   }
}


PRIVATE BOOL onQueryEndSession( HWND hwnd ) {

   Editor *pEditor = getEditor( hwnd );;
   pEditor->saveState();
   return pEditor->saveIfNecessary();
}


PRIVATE void onEndSession( HWND hwnd, BOOL fEnding ) {

   Document::s_bEndSession = 0 != fEnding;
}


PRIVATE LRESULT onPowerBroadcast(
   HWND hwnd, DWORD dwPowerEvent, DWORD dwData )
{
   // LATER/TODO: GetDevicePowerState( hfile );

#ifdef PBT_APMBATTERYLOW

   switch ( dwPowerEvent ) {
   case PBT_APMBATTERYLOW:
      break;
   case PBT_APMPOWERSTATUSCHANGE:
      break; // LATER: increase or decrease autosave interval
   case PBT_APMRESUMECRITICAL: // Sent only to drivers, not to apps.
      break; // Are we still in good shape?
   case PBT_APMRESUMESUSPEND:
      break; // Normal resumption.
   case PBT_APMQUERYSUSPEND:
      break; // Return BROADCAST_QUERY_DENY to deny
   case PBT_APMSUSPEND:
      break;// LATER: saveIfNecessary.
   }

#endif // PBT_APMBATTERYLOW

   return 0;
}


PRIVATE inline void onSysColorChange( HWND hwnd ) {

   for ( HWND hwndChild = GetTopWindow( hwnd );
      IsWindow( hwndChild );
      hwndChild = GetNextWindow( hwndChild, GW_HWNDNEXT ) )
   {
      FORWARD_WM_SYSCOLORCHANGE( hwndChild, SNDMSG );
   }
}


PRIVATE void refreshMenu( HWND hwnd ) {

   if ( isWindowsNT() ) { // May have changed language.
      verify( DestroyMenu( GetMenu( hwnd ) ) );
      SetMenu( hwnd, LoadMenu( getModuleHandle(),
         MAKEINTRESOURCE( IDR_MENU ) ) );
      verify( DrawMenuBar( hwnd ) );
      getEditor( hwnd )->loadAcceleratorTable();
   }
   Editor *pEditor = getEditor( hwnd );
   pEditor->refreshToolbar();
   pEditor->setTitle();
}


PRIVATE inline void onSettingChange( HWND hwnd, LPCTSTR pszSection ) {

   MenuFont::refresh();
   refreshMenu( hwnd ); // May have changed language.
   for ( HWND hwndChild = GetTopWindow( hwnd );
      IsWindow( hwndChild );
      hwndChild = GetNextWindow( hwndChild, GW_HWNDNEXT ) )
   {
      FORWARD_WM_SETTINGCHANGE( hwndChild, pszSection, SNDMSG );
   }
   recalcLayout( hwnd );

   // In case the thousand separator has changed:
   getStatusbar( hwnd )->update( getEditWnd( hwnd )->getCurPos() );
}


PRIVATE inline BOOL onHelp( const HWND hwnd, HELPINFO *pHelpInfo ) {

   help( hwnd, pHelpInfo );
   return TRUE;
}


PRIVATE inline BOOL onApp( const HWND hwnd, ATOM atomDocName ) {

   PATHNAME szDocName = { 0 };
   const UINT string_length =
      GlobalGetAtomName( atomDocName, szDocName, dim( szDocName ) );
   if ( 0 == string_length ) { // May happen if message timed out...
      return FALSE;
   }

   const Document *pDocument = getDocument( hwnd );
   assert( isGoodConstPtr( pDocument ) );

#if 0 // Test code
   messageBox( hwnd, MB_OK, _T( "atom = %1!d! (%2!#x!)\nname = [%3]" ),
      atomDocName, atomDocName, szDocName );
#endif

   return 0 != pDocument && areFileNamesEqual( pDocument->getPath(), szDocName );
}

// TODO: Unit test new safe string API
PRIVATE bool isExecutable( LPCTSTR title ) {

	TCHAR szExt[ _MAX_EXT ] = { 0 };
	_tsplitpath_s( title, 0, 0, 0, 0, 0, 0, szExt, _MAX_EXT );
	return
		0 == _tcsicmp( szExt, _T( ".html" ) ) || 
		0 == _tcsicmp( szExt, _T( ".htm"  ) ) || 
		0 == _tcsicmp( szExt, _T( ".bat"  ) ) || 
		0 == _tcsicmp( szExt, _T( ".cmd"  ) ) ; 
}


/**
 * Note: This is called for *all* menus, including popups!
 */
PRIVATE void onInitMenu( HWND hwnd, HMENU hmenu ) {

   AbstractEditWnd *pEditWnd = getEditWnd( hwnd );

   String strTitle = getDocument( hwnd )->getTitle();
   doubleAmpersands( &strTitle );
   const String strFmt = loadString( IDS_FILE_COPY );
   setMenuItemText(
      hmenu, ID_FILE_COPY, strFmt.c_str(),
      compactPath( strTitle.c_str(), 120 ).c_str() );

   // The book says that the delete command should be enabled
   // if and only if there is text and the file is writable.
   // This is not quite right; if the caret is at the bottom
   // of the file, no delete is possible.
   // Note that there is an unfortunate amount of code duplication
   // between this function and Editor::updateToolbar.
   int nStart = 0;
   int nEnd   = 0;

   const bool bHasSelection = pEditWnd->getSel( &nStart, &nEnd );
   const int  nTextLength   = pEditWnd->getTextLength( true );

   const bool bDeleteOK = nStart < nTextLength; // Can't delete if we're at the end
   const bool bHasText  = 0 < nTextLength;
   const bool bCanUndo  = pEditWnd->canUndo();
   const bool bHasRedo  = pEditWnd->hasRedo();
   const bool bCanRedo  = bHasRedo && pEditWnd->canRedo();

   enableMenuItem( hmenu,
      ID_FILE_ABANDONCHANGES, !getEditor( hwnd )->isWhistleClean() );
   enableMenuItem( hmenu,
      ID_SENDTO_SYSTEM, isExecutable( strTitle.c_str() ) );

   String strDeleteFile = getMenuItemText(
      hmenu, ID_FILE_DELETE );
   int iDotPos = strDeleteFile.find( _T( '.' ) );
   if ( 0 <= iDotPos ) {
      strDeleteFile.erase( iDotPos );
   }
   if ( getShowDeleteDialog() ) {
      strDeleteFile.append( _T( "..." ) );
   }
   setMenuItemText( hmenu, ID_FILE_DELETE, _T( "%1" ),
      strDeleteFile.c_str() );

   // TODO -- use Document instead, when fixed.
   const bool bReadOnly = getEditWnd( hwnd )->isReadOnly();
   enableMenuItem( hmenu, ID_FILE_DELETE, !bReadOnly );

   enableMenuItem( hmenu, ID_EDIT_COPY         , bHasSelection );
   enableMenuItem( hmenu, ID_EDIT_CUT          , bHasSelection && !bReadOnly );
   enableMenuItem( hmenu, ID_EDIT_FINDSELECTION, bHasText      );
   enableMenuItem( hmenu, ID_EDIT_DELETE, !bReadOnly && bDeleteOK );
   enableMenuItem( hmenu, ID_EDIT_SELECTALL    , bHasText );

   setMenuItemText( hmenu, ID_EDIT_UNDO,
      loadString( IDS_UNDO ).c_str(),
      pEditWnd->getUndoName().c_str() );
   enableMenuItem( hmenu, ID_EDIT_UNDO, bCanUndo );

   if ( bHasRedo ) {
      setMenuItemText( hmenu, ID_EDIT_REDO,
         loadString( IDS_REDO ).c_str(),
         pEditWnd->getRedoName().c_str() );
      enableMenuItem( hmenu, ID_EDIT_REDO, bCanRedo );
   }

   enableMenuItem( hmenu, ID_EDIT_FIND, bHasText );
   enableMenuItem( hmenu, ID_EDIT_FINDNEXT,
      bHasText && 0 != s_strLast.size() );
   enableMenuItem( hmenu, ID_EDIT_FINDPREVIOUS,
      bHasText && 0 != s_strLast.size() );
   enableMenuItem( hmenu, ID_EDIT_REPLACE,
      bHasText && !pEditWnd->isReadOnly() );

   checkMenuItem( hmenu, ID_VIEW_TOOLBAR  ,
      0 != IsWindowVisible(   *getToolbar( hwnd ) ) );
   checkMenuItem( hmenu, ID_VIEW_STATUSBAR,
      0 != IsWindowVisible( *getStatusbar( hwnd ) ) );

   // Append MRU items to the menu. If hmenu is a popup menu,
   // we're called after a toolbar drop-down, and we use hmenu
   // directly. If hmenu is the window's menu, get the file menu,
   // which is the first submenu:
   bool bShowAccelerators = false;
   HMENU hmenuFile = hmenu;
   if ( GetMenu( hwnd ) == hmenu ) {
      hmenuFile = GetSubMenu( hmenu, 0 );
      if ( containsItem( hmenuFile, ID_FILE_OPEN ) ) {
         bShowAccelerators = true;
      } else {
         hmenuFile = 0;
      }
   }

   if ( IsMenu( hmenuFile ) ) {
      for ( int id = ID_MRU_1; id <= ID_MRU_10; ++id ) {
         DeleteMenu( hmenuFile, id, MF_BYCOMMAND );
      }

      MRU().addFilesToMenu( hmenuFile, bShowAccelerators );
   }
}


#define MENU_CLOSING 0xffff


// The macro defined in windowsx.h handles MENU_CLOSING incorrectly.
#undef HANDLE_WM_MENUSELECT
#define HANDLE_WM_MENUSELECT(hwnd, wParam, lParam, fn) \
   ((fn)((hwnd), (HMENU)(lParam),  \
   (HIWORD(wParam) & MF_POPUP) ? 0L : (int)(LOWORD(wParam)), \
   (HIWORD(wParam) == MENU_CLOSING ? 0 : \
      HIWORD(wParam) & MF_POPUP) ? \
      GetSubMenu((HMENU)lParam, LOWORD(wParam)) : 0L, \
   (UINT)(((short)HIWORD(wParam) == -1) ? \
      0xFFFFFFFF : HIWORD(wParam))), 0L)


PRIVATE void onMenuSelect( // WM_MENUSELECT
   HWND hwnd, HMENU hmenu, int nItem, HMENU hmnuPopup, UINT flags )
{
   const Editor *pEditor = getEditor( hwnd );
   assert( isGoodConstPtr( pEditor ) );

   Statusbar *pStatusbar = getStatusbar( hwnd );
   assert( isGoodPtr( pStatusbar ) );

   const AbstractEditWnd *pEditWnd = getEditWnd( hwnd );
   assert( isGoodConstPtr( pEditWnd ) );

   try {
      if ( MENU_CLOSING == LOWORD( flags ) ) {
         pStatusbar->update( pEditWnd->getCurPos() );
      } else {
         String strMessage;
         if ( IsMenu( hmnuPopup ) ) {
            strMessage = pEditor->getMenuDescription( hmnuPopup );
         } else {
            strMessage = pEditor->getMenuItemDescription(
               nItem, s_strLast );
         }
         pStatusbar->setMessage( strMessage.c_str() );
      }
   }
   catch ( ... ) {
      pStatusbar->update( pEditWnd->getCurPos() );
      throw;
   }
}


/**
 * Word Wrap function on View menu (toggle).
 */
PRIVATE void onViewWordWrap( HWND hwnd ) {

   assert( IsWindow( hwnd ) );
   HMENU hmenu = GetMenu( hwnd );
   assert( IsMenu( hmenu ) );
   UINT uiState = GetMenuState(
      hmenu, ID_VIEW_WORDWRAP, MF_BYCOMMAND );
   getEditor( hwnd )->setWordWrap( 0 == ( MF_CHECKED & uiState ) );
}


PRIVATE void toggleDockedWindow(
   const HWND hwndMain, const HWND hwndChild )
{
   assert( IsWindow( hwndMain ) );
   assert( IsWindow( hwndChild ) );

   const int nShow = IsWindowVisible( hwndChild ) ? SW_HIDE : SW_SHOW;
   ShowWindow( hwndChild, nShow );
   recalcLayout( hwndMain );
}


PRIVATE void onViewStatusbar( HWND hwnd ) {

   Statusbar *pStatusbar = getStatusbar( hwnd );
   toggleDockedWindow( hwnd, *pStatusbar );
   setStatusbarVisible( IsWindowVisible( *pStatusbar ) );
}


PRIVATE void onViewToolbar( HWND hwnd ) {

   Toolbar *pToolbar = getToolbar( hwnd );
   toggleDockedWindow( hwnd, *pToolbar );
   setToolbarVisible( IsWindowVisible( *pToolbar ) );
}


PRIVATE void onEditCut( HWND hwnd ) {
   getEditWnd( hwnd )->cutSelection();
}


PRIVATE void onEditCopy( HWND hwnd ) {
   getEditWnd( hwnd )->copySelection();
}


PRIVATE void onEditPaste( HWND hwnd ) {
   getEditWnd( hwnd )->paste();
}


PRIVATE void onEditDelete( HWND hwnd ) {
   getEditWnd( hwnd )->deleteSelection();
}


PRIVATE void onEditUndo( HWND hwnd ) {
   getEditWnd( hwnd )->undo();
}


PRIVATE void onEditRedo( HWND hwnd ) {
   getEditWnd( hwnd )->redo();
}


PRIVATE void onEditSelectAll( HWND hwnd ) {
   getEditWnd( hwnd )->setSel( 0, -1 );
}


PRIVATE inline void search( HWND hwnd ) {

   if ( !getEditor( hwnd )->searchAndSelect( s_strLast ) ) {
      Statusbar *pStatusbar = getStatusbar( hwnd );
      if ( IsWindowVisible( *pStatusbar ) ) {
         pStatusbar->setErrorMessage( MB_ICONINFORMATION,
            IDS_STRING_NOT_FOUND, s_strLast.c_str() );
      } else {
         messageBox( hwnd, MB_ICONINFORMATION | MB_OK,
            IDS_STRING_NOT_FOUND, s_strLast.c_str() );
      }
   }
}


PRIVATE void onEditFind( HWND hwnd ) {

   AbstractEditWnd *pEditWnd = getEditWnd( hwnd );
   assert( 0 != pEditWnd );

   String strSelection;
   if ( !pEditWnd->getSel( &strSelection ) ) {
      if ( !pEditWnd->getWord( &strSelection ) ) {
         strSelection = s_strLast;
      }
   }

   // In case of msgs on status bar.
   getStatusbar( hwnd )->update( pEditWnd->getCurPos() );
   FindDlg findDlg( getEditor( hwnd ), strSelection );
   const UINT uiRetCode = findDlg.doModal( hwnd, IDD_EDITFIND );
   if ( IDOK == uiRetCode ) {
      s_strLast = findDlg.getSearchPattern();
   }
}


PRIVATE void onEditReplace( HWND hwnd ) {

   AbstractEditWnd *pEditWnd = getEditWnd( hwnd );
   assert( 0 != pEditWnd );

   String strSelection;
   if ( !pEditWnd->getSel( &strSelection ) ) {
      if ( !pEditWnd->getWord( &strSelection ) ) {
         strSelection = s_strLast;
      }
   }

   // In case of msgs on status bar.
   getStatusbar( hwnd )->update( pEditWnd->getCurPos() );
   FindDlg findDlg( getEditor( hwnd ), strSelection, true );
   const UINT uiRetCode = findDlg.doModal( hwnd, IDD_EDITREPLACE );
   if ( IDOK == uiRetCode ) {
      s_strLast = findDlg.getSearchPattern();
   }
}


PRIVATE void onEditFindNext( HWND hwnd ) {

   setBackwards( false );
   if ( s_strLast.empty() ) {
      onEditFind( hwnd );
   } else {
      search( hwnd );
   }
}


PRIVATE void onEditFindPrevious( HWND hwnd ) {

   setBackwards( true );
   if ( s_strLast.empty() ) {
      onEditFind( hwnd );
   } else {
      search( hwnd );
   }
}


PRIVATE void onFindSelection( HWND hwnd, bool bBackwards ) {

   setBackwards( bBackwards );
   AbstractEditWnd *pEditWnd = getEditWnd( hwnd );
   String str;
   if ( pEditWnd->getSel( &str ) || pEditWnd->getWord( &str ) ) {
      s_strLast = str;
   }

   if ( s_strLast.empty() ) {
      MessageBeep( MB_ICONWARNING );
   } else {
      search( hwnd );
   }
}


PRIVATE void onEditFindSelection( HWND hwnd ) {

   onFindSelection( hwnd, false );
}


PRIVATE void onFindPrevSelection( HWND hwnd ) {

   onFindSelection( hwnd, true );
}


PRIVATE void onFileNew( HWND hwnd ) {

   openFile( hwnd, createNewFile( hwnd ) );
}


PRIVATE void onFileNewEditor( HWND hwnd ) {

   startInstance( _T( "" ) );
}


PRIVATE void onFileOpen( HWND hwnd ) {

   assert( IsWindow( hwnd ) );
   assert( IsWindowEnabled( hwnd ) );

   try {
      PATHNAME szPath = { 0 };
      bool bNewFile = true;
      if ( openFileDlg( hwnd, szPath, dim( szPath ), &bNewFile ) ) {
         if ( bNewFile ) {
            startInstance( szPath );
         } else {
            openFile( hwnd, szPath );
         }
      }
   }
   catch ( ... ) {
      trace( _T( "Exception in onFileOpen, enabling main wnd\n" ) );
      EnableWindow( hwnd, true );
      throw;
   }
}


PRIVATE void onFileReload( HWND hwnd ) {

	assert( IsWindow( hwnd ) );
	assert( IsWindowEnabled( hwnd ) );

	Editor *pEditor = getEditor( hwnd );
	if ( pEditor->saveIfNecessary() ) {
		pEditor->reload();
	}
	enablePaste( hwnd );
}


PRIVATE void onFileCopy( HWND hwnd ) {

   getEditor( hwnd )->copyFile();
}


PRIVATE void onFilePrint( HWND hwnd ) {

   WaitCursor waitCursor;
   getEditor( hwnd )->printFile();
}

// TODO: Unit test new safe string API
PRIVATE void onFileSendToMailRecipient( HWND hwnd ) {

	const Document* pDocument = getDocument( hwnd );
	PATHNAMEA szFilepath = { 0 };
#ifdef UNICODE
	wideCharToMultiByte( pDocument->getPath().c_str(), szFilepath );
#else
	strcpy_s( szFilepath, pDocument->getPath().c_str() );
#endif

	PATHNAMEA szFilename = { 0 };
#ifdef UNICODE
	wideCharToMultiByte( pDocument->getTitle().c_str(), szFilename );
#else
	strcpy_s( szFilename, pDocument->getTitle().c_str() );
#endif

	AutoLibrary hLib( _T( "MAPI32.DLL" ) );
	LPMAPISENDDOCUMENTS fpMAPISendDocuments =
		(LPMAPISENDDOCUMENTS) GetProcAddress( hLib, "MAPISendDocuments" );
	if ( 0 != fpMAPISendDocuments ) {
		String msg;
		const ULONG result = fpMAPISendDocuments(
			reinterpret_cast< ULONG >( hwnd ),
			";", szFilepath, szFilename, 0 );
		switch ( result ) {
		case MAPI_E_ATTACHMENT_OPEN_FAILURE:
			msg = _T( "One or more files in the lpszFilePaths parameter could not be located. No message was sent. " );
			break;
		case MAPI_E_ATTACHMENT_WRITE_FAILURE:
			msg = _T( "An attachment could not be written to a temporary file. " )
				_T( "Check directory permissions." );
			break;
		case MAPI_E_FAILURE:
			msg = _T( "One or more unspecified errors occurred while sending the message. " )
				_T( "It is not known if the message was sent." );
			break;
		case MAPI_E_INSUFFICIENT_MEMORY:
			msg = _T( "There was insufficient memory to proceed." );
			break;
		case MAPI_E_LOGIN_FAILURE:
			msg = _T( "There was no default logon, and the user failed to " )
				_T( "log on successfully when the logon dialog box was displayed. " )
				_T( "No message was sent." );
			break;
		case MAPI_E_USER_ABORT:
		case SUCCESS_SUCCESS:
			break;
		}
		if ( 0 < msg.length() ) {
			messageBox( hwnd, MB_OK | MB_ICONERROR, msg );
		}
	} else {
		messageBox( hwnd, MB_OK | MB_ICONERROR,
			_T( "Unable to load MAPI" ) );
	}
}


PRIVATE void onFileSendToSystem( HWND hwnd ) {

	const Document* pDocument = getDocument( hwnd );
	ShellExecute( hwnd, _T( "open" ), pDocument->getPath().c_str(),
		_T( "" ), _T( "" ), SW_SHOWDEFAULT );
}


PRIVATE void onFilePageSetup( HWND hwnd ) {

   WaitCursor waitCursor;
   setupPage( hwnd, getDocument( hwnd ) );
}


/*
 * This makes the UI reflect changed file properties.
 * It is called after the PropertiesDlg exits, and it's
 * called in response to the Apply button in that dialog.
 */
PRIVATE void onFilePropsChanged( HWND hwnd ) {

   getEditor( hwnd )->setTitle();
   getStatusbar( hwnd )->setFileType(
      getDocument( hwnd )->isUnicode() );
   const bool bReadOnly = getDocument( hwnd )->isReadOnly();
   getEditor( hwnd )->setReadOnly( bReadOnly );
}


PRIVATE void onFileProperties( HWND hwnd ) {

   getEditor( hwnd )->saveIfNecessary();
   WaitCursor waitCursor;
   PropertiesDlg( hwnd, getDocument( hwnd ) );
   // The dialog itself takes care of invoking onFilePropsChanged.
}


PRIVATE void onFileRename( HWND hwnd ) {

   getEditor( hwnd )->saveIfNecessary();
   WaitCursor waitCursor;
   Document *pDocument = getDocument( hwnd );
   assert( isGoodPtr( pDocument ) );

   String strFullPath = pDocument->getPath();
   const bool bRetVal = saveFile(
      hwnd, &strFullPath, IDS_MOVE, IDD_MOVE_CHILD );
   if ( bRetVal ) {
      if ( pDocument->setPath( hwnd, strFullPath ) ) {
         onFilePropsChanged( hwnd );
      } else {
         getStatusbar( hwnd )->setHighlightMessage( IDS_FAILED_MOVE );
      }
   }
}


PRIVATE void onFileDelete( HWND hwnd ) {

   UINT uiRetCode = IDOK;
   if ( getShowDeleteDialog() ) {
      DeleteDlg deleteDlg( getDocument( hwnd )->getPath() );
      uiRetCode = deleteDlg.doModal( hwnd );
   }

   if ( IDOK == uiRetCode &&
        getDocument( hwnd )->deleteFile( hwnd ) )
   {
      getEditor( hwnd )->clean();
      PostMessage( hwnd, WM_CLOSE, 0, 0 );
   }
}


PRIVATE void onFileAbandonChanges( HWND hwnd ) {

   WaitCursor waitCursor( _T( "load.ani" ) );
   getEditor( hwnd )->restoreOriginal();
}


PRIVATE void onFileClose( HWND hwnd ) {

   PostMessage( hwnd, WM_CLOSE, 0, 0 );
}


PRIVATE void onViewOptions( HWND hwnd ) {

   Editor *pEditor = getEditor( hwnd );
   assert( isGoodPtr( pEditor ) );

   OptionsDlg optionsDlg(
      pEditor->getLogFont( true  ),
      pEditor->getLogFont( false ) );
   if ( IDOK == optionsDlg.doModal( hwnd ) ) {
      pEditor->setLogFont( optionsDlg.getFixedFont       (), true  );
      pEditor->setLogFont( optionsDlg.getProportionalFont(), false );
      refreshMenu( hwnd ); // May have changed language.
   }
}


PRIVATE void onClickReadOnly( HWND hwnd ) {

   Editor *pEditor = getEditor( hwnd );
   const bool bReadOnly = pEditor->getReadOnly();
   pEditor->setReadOnly( bReadOnly );
   enablePaste( hwnd );
}


PRIVATE void onCommandReadOnly( HWND hwnd ) {

   Editor *pEditor = getEditor( hwnd );
   bool bReadOnly = pEditor->getReadOnly();
   bReadOnly = !bReadOnly;
   pEditor->setReadOnly( bReadOnly );
   enablePaste( hwnd );
}


PRIVATE void onCommandTabs( HWND hwnd ) {

   if ( getEditWnd( hwnd )->canSetTabs() ) {
      Toolbar *pToolbar = getToolbar( hwnd );
      assert( IsWindow( *pToolbar ) );

      HWND hwndTabs = pToolbar->getChild( IDC_TABEDIT );
      assert( IsWindow( hwndTabs  ) );

      SetFocus( hwndTabs );
      Edit_SetSel( hwndTabs, 0, -1 );
   }
}


PRIVATE void onSetTabs( HWND hwnd ) {

   AbstractEditWnd *pEditWnd = getEditWnd( hwnd );
   if ( pEditWnd->canSetTabs() ) {
      Toolbar *pToolbar = getToolbar( hwnd );
      const int nSpacesPerTab = pToolbar->getTabs();
      pEditWnd->setSpacesPerTab( nSpacesPerTab );
      getDocument( hwnd )->setTabs( nSpacesPerTab );
   }
}


PRIVATE void onViewFixedFont( HWND hwnd ) {

   setFont( hwnd, true );
}


PRIVATE void onViewProportionalFont( HWND hwnd ) {

   setFont( hwnd, false );
}


PRIVATE void onViewSetFixedFont( HWND hwnd ) {

   LOGFONT logFont = *getEditor( hwnd )->getLogFont( true );
   if ( selectFont( hwnd, &logFont, 0, CF_FIXEDPITCHONLY ) ) {
      getEditor( hwnd )->setLogFont( &logFont, true );
   }
}


PRIVATE void onViewSetProportionalFont( HWND hwnd ) {

   LOGFONT logFont = *getEditor( hwnd )->getLogFont( false );
   if ( selectFont( hwnd, &logFont ) ) {
      getEditor( hwnd )->setLogFont( &logFont, false );
   }
}


PRIVATE void onViewSetFont( HWND hwnd ) {

   if ( getEditor( hwnd )->hasFixedFont() ) {
      onViewSetFixedFont( hwnd );
   } else {
      onViewSetProportionalFont( hwnd );
   }
}


PRIVATE void onViewToggleFont( HWND hwnd ) {

   if ( getEditor( hwnd )->hasFixedFont() ) {
      onViewProportionalFont( hwnd );
   } else {
      onViewFixedFont( hwnd );
   }
}


PRIVATE void onCommandScroll( HWND hwnd, UINT nWhat ) {
   AbstractEditWnd *pEditWnd = getEditWnd( hwnd );
   pEditWnd->scroll( nWhat );
   pEditWnd->bringCaretToWindow( getEditor( hwnd )->getFont() );
}


PRIVATE void onCommandScrollUp( HWND hwnd ) {
   onCommandScroll( hwnd, SB_LINEUP );
}


PRIVATE void onCommandScrollDown( HWND hwnd ) {
   onCommandScroll( hwnd, SB_LINEDOWN );
}


PRIVATE void onCommandDeleteLine( HWND hwnd ) {
   trace( _T( "onCommandDeleteLine" ) ); // LATER: Implement
}


PRIVATE void onCommandSave( HWND hwnd ) {

   WaitCursor waitCursor( _T( "save.ani" ) );

   const bool bSaved = getEditor( hwnd )->save();
   if ( bSaved ) {
      getStatusbar( hwnd )->setMessage(
         IDS_SAVED_FILE, getDocument( hwnd )->getPath().c_str() );
   } else {
      getStatusbar( hwnd )->setHighlightMessage(  IDS_DIDNT_SAVE_FILE,
         getDocument( hwnd )->getPath().c_str() );
      MessageBeep( MB_ICONWARNING );
   }
}


PRIVATE void onHelpContents( HWND hwnd ) {

   WinHelp( hwnd, getHelpFile(), HELP_FINDER, 0 );
}


PRIVATE void onHelpKeyboard( HWND hwnd ) {

   WinHelp(
      hwnd, getHelpFile(), HELP_CONTEXT, IDH_USING_THE_KEYBOARD );
}


PRIVATE void onHelpHomePage( HWND hwnd ) {

   // Note that plain ShellExecute has a bug when running under
   // Windows 9x. If it initiates a DDE conversation to open
   // a file, it does not close its end of the conversation.
   // This can supposedly lead to Bad Things when you try to close
   // the app (some web browser in our case).

   SHELLEXECUTEINFO shellExecuteInfo = {
      sizeof( SHELLEXECUTEINFO ),
      0, //SEE_MASK_FLAG_DDEWAIT,
      hwnd,
      _T( "open" ),
      _T( "http://www.codeplex.com/TextEdit" ),
      0, 0, SW_SHOWNORMAL,
   };

   verify( ShellExecuteEx( &shellExecuteInfo ) );
}


PRIVATE void onHelpAbout( HWND hwnd ) {

   AboutDlg aboutDlg( hwnd );
}


/**
 * Get rid of any messages on the status bar.
 */
PRIVATE void onResetStatusbar( HWND hwnd ) {

#if 1
   getStatusbar( hwnd )->update( getEditWnd( hwnd )->getCurPos() );
#else
   try {
      getStatusbar( hwnd )->update( getEditWnd( hwnd )->getCurPos() );
   }
   catch ( ... ) {
      trace( _T( "exception in onCommand\n" ) );
   }
#endif
}


#ifdef _DEBUG // Stuff to exercise SEH and other exception handling:

#pragma warning( disable: 4723 ) // potential divide by 0

PRIVATE void onDivideByZero( HWND ) {

   int x = 1;
   x = x / 0;
}

#pragma warning( default: 4723 ) // potential divide by 0


// TestClass is testing to see if we're unwinding properly.
class TestClass {
public:
   TestClass () {
       trace( _T( "TestClass ctor\n" ) );
   }
   ~TestClass() {
       trace( _T( "TestClass dtor\n" ) );
   }
};

// TODO: Unit test new safe string API
PRIVATE void onAccessViolation( HWND ) {

   TestClass testClass;

   int *pNullPointer = 0;
   *pNullPointer = 0;            // Exception
}

PRIVATE void onOutOfMemory( HWND ) {

   new TCHAR[ (INT_MAX - 100) / 2 ];
   new TCHAR[ (INT_MAX - 100) / 2 ];
}

#pragma warning( disable: 4717 ) // Stack overflow is intentional
PRIVATE void onStackOverflow( HWND hwnd ) {
   onStackOverflow( hwnd );
}
#pragma warning( disable: 4717 )

#endif // _DEBUG


PRIVATE inline void onCommand(
   HWND hwnd, int id, HWND hwndCtl, UINT codeNotify )
{
   #define COMMAND( id, proc ) case id: proc( hwnd ); break

   switch ( id ) {

   COMMAND( ID_FILE_NEW                 , onFileNew                 );
   COMMAND( ID_FILE_NEW_EDITOR          , onFileNewEditor           );
   COMMAND( ID_FILE_OPEN                , onFileOpen                );
   COMMAND( ID_RELOAD                   , onFileReload              );
   COMMAND( ID_FILE_COPY                , onFileCopy                );
   COMMAND( ID_FILE_PRINT               , onFilePrint               );
   COMMAND( ID_FILE_SENDTO_MAILRECIPIENT, onFileSendToMailRecipient );
   COMMAND( ID_SENDTO_SYSTEM			, onFileSendToSystem        );
   COMMAND( ID_FILE_PAGESETUP           , onFilePageSetup           );
   COMMAND( ID_COMMAND_PROPSCHANGED     , onFilePropsChanged        );
   COMMAND( ID_FILE_PROPERTIES          , onFileProperties          );
   COMMAND( ID_FILE_RENAME              , onFileRename              );
   COMMAND( ID_FILE_DELETE              , onFileDelete              );
   COMMAND( ID_FILE_ABANDONCHANGES      , onFileAbandonChanges      );
   COMMAND( ID_FILE_CLOSE               , onFileClose               );
   COMMAND( ID_VIEW_WORDWRAP            , onViewWordWrap            );
   COMMAND( ID_VIEW_STATUSBAR           , onViewStatusbar           );
   COMMAND( ID_VIEW_TOOLBAR             , onViewToolbar             );
   COMMAND( ID_EDIT_CUT                 , onEditCut                 );
   COMMAND( ID_EDIT_COPY                , onEditCopy                );
   COMMAND( ID_EDIT_PASTE               , onEditPaste               );
   COMMAND( ID_EDIT_DELETE              , onEditDelete              );
   COMMAND( ID_EDIT_UNDO                , onEditUndo                );
   COMMAND( ID_EDIT_REDO                , onEditRedo                );
   COMMAND( ID_EDIT_SELECTALL           , onEditSelectAll           );
   COMMAND( ID_EDIT_FIND                , onEditFind                );
   COMMAND( ID_EDIT_FINDNEXT            , onEditFindNext            );
   COMMAND( ID_EDIT_FINDPREVIOUS        , onEditFindPrevious        );
   COMMAND( ID_ACCEL_FINDNEXT           , onEditFindNext            );
   COMMAND( ID_ACCEL_FINDPREVIOUS       , onEditFindPrevious        );
   COMMAND( ID_EDIT_FINDSELECTION       , onEditFindSelection       );
   COMMAND( ID_ACCEL_FINDPREVSELECTION  , onFindPrevSelection       );
   COMMAND( ID_EDIT_REPLACE             , onEditReplace             );
   COMMAND( ID_VIEW_FIXEDFONT           , onViewFixedFont           );
   COMMAND( ID_VIEW_PROPORTIONALFONT    , onViewProportionalFont    );
   COMMAND( ID_VIEW_SETFIXEDFONT        , onViewSetFixedFont        );
   COMMAND( ID_VIEW_SETPROPORTIONALFONT , onViewSetProportionalFont );
   COMMAND( ID_VIEW_SETFONT             , onViewSetFont             );
   COMMAND( ID_COMMAND_TOGGLEFONT       , onViewToggleFont          );
   COMMAND( ID_VIEW_OPTIONS             , onViewOptions             );
   COMMAND( ID_HELP_ABOUT               , onHelpAbout               );
   COMMAND( IDC_READONLY                , onClickReadOnly           );
   COMMAND( ID_COMMAND_READONLY         , onCommandReadOnly         );
   COMMAND( ID_COMMAND_TABS             , onCommandTabs             );
   COMMAND( ID_COMMAND_SCROLLUP         , onCommandScrollUp         );
   COMMAND( ID_COMMAND_SCROLLDOWN       , onCommandScrollDown       );
   COMMAND( ID_COMMAND_DELETELINE       , onCommandDeleteLine       );
   COMMAND( ID_COMMAND_SAVE             , onCommandSave             );
   COMMAND( ID_SET_TABS                 , onSetTabs                 );
   COMMAND( ID_HELP_CONTENTS            , onHelpContents            );
   COMMAND( ID_HELP_KEYBOARD            , onHelpKeyboard            );
   COMMAND( ID_HELP_HOMEPAGE            , onHelpHomePage            );
   COMMAND( ID_COMMAND_RESETSTATUSBAR   , onResetStatusbar          );

#ifdef _DEBUG
   COMMAND( ID_DEBUG_DIVIDEBYZERO      , onDivideByZero            );
   COMMAND( ID_DEBUG_ACCESSVIOLATION   , onAccessViolation         );
   COMMAND( ID_DEBUG_OUTOFMEMORY       , onOutOfMemory             );
   COMMAND( ID_DEBUG_STACKOVERFLOW     , onStackOverflow           );
#endif

   case ID_MRU_1: case ID_MRU_2: case ID_MRU_3:
   case ID_MRU_4: case ID_MRU_5: case ID_MRU_6:
   case ID_MRU_7: case ID_MRU_8: case ID_MRU_9:
      {
         // Note: activateOldInstance will activate us,
         // too, if we're reloading the same file.
         const String strPath = MRU().getFile( id );
         openFile( hwnd, strPath );
      }
      break;
   }

   #undef COMMAND
}


PRIVATE inline void onSysCommand(
   HWND hwnd, UINT cmd, int x, int y )
{
   if ( SC_RESTORE == cmd ) {
      SetTimer( hwnd, timer_save_state, 500, 0 );
   }
   FORWARD_WM_SYSCOMMAND( hwnd, cmd, x, y, DefWindowProc );
}


#undef HANDLE_WM_NCCALCSIZE
#undef FORWARD_WM_NCCALCSIZE
#define HANDLE_WM_NCCALCSIZE(hwnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(fn)((hwnd), (BOOL)(wParam), (NCCALCSIZE_PARAMS *)(lParam))
#define FORWARD_WM_NCCALCSIZE(hwnd, fCalcValidRects, lpcsp, fn) \
    (UINT)(DWORD)(fn)((hwnd), WM_NCCALCSIZE, (WPARAM)(fCalcValidRects), (LPARAM)(NCCALCSIZE_PARAMS *)(lpcsp))

UINT onNCCalcSize( HWND hwnd, BOOL fCalcValidRects, NCCALCSIZE_PARAMS * lpcsp ) {

   UINT uiRet = FORWARD_WM_NCCALCSIZE(
      hwnd, fCalcValidRects, lpcsp, DefWindowProc );
#if 0 // Results in blank NC area when restoring maximized window!
   if ( fCalcValidRects ) {
      uiRet |= WVR_ALIGNBOTTOM | WVR_ALIGNRIGHT;
   }
#endif
   return uiRet;
}

/**
 * This is the window function of TextEdit's main window.
 * It's a central switching point for the whole application.
 */
LRESULT CALLBACK mainWndProc(
   HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
   switch ( msg ) {

   HANDLE_MSG( hwnd, WM_CREATE         , onCreate          );
   HANDLE_MSG( hwnd, WM_ACTIVATE       , onActivate        );
   HANDLE_MSG( hwnd, WM_DESTROY        , onDestroy         );
   HANDLE_MSG( hwnd, WM_CLOSE          , onClose           );
   HANDLE_MSG( hwnd, WM_DROPFILES      , onDropFiles       );
   HANDLE_MSG( hwnd, WM_TIMER          , onTimer           );
   HANDLE_MSG( hwnd, WM_SIZE           , onSize            );
   HANDLE_MSG( hwnd, WM_MOVE           , onMove            );
   HANDLE_MSG( hwnd, WM_ENTERSIZEMOVE  , onEnterSizeMove   );
   HANDLE_MSG( hwnd, WM_EXITSIZEMOVE   , onExitSizeMove    );
   HANDLE_MSG( hwnd, WM_SETFOCUS       , onSetFocus        );
   HANDLE_MSG( hwnd, WM_INITMENU       , onInitMenu        );
   HANDLE_MSG( hwnd, WM_MENUSELECT     , onMenuSelect      );
   HANDLE_MSG( hwnd, WM_COMMAND        , onCommand         );
   HANDLE_MSG( hwnd, WM_SYSCOMMAND     , onSysCommand      );
   HANDLE_MSG( hwnd, WM_DRAWCLIPBOARD  , onDrawClipboard   );
   HANDLE_MSG( hwnd, WM_CHANGECBCHAIN  , onChangeCBChain   );
   HANDLE_MSG( hwnd, WM_QUERYENDSESSION, onQueryEndSession );
   HANDLE_MSG( hwnd, WM_ENDSESSION     , onEndSession      );
   HANDLE_MSG( hwnd, WM_POWERBROADCAST , onPowerBroadcast  );
   HANDLE_MSG( hwnd, WM_SYSCOLORCHANGE , onSysColorChange  );
   HANDLE_MSG( hwnd, WM_SETTINGCHANGE  , onSettingChange   );
   HANDLE_MSG( hwnd, WM_HELP           , onHelp            );
   HANDLE_MSG( hwnd, WM_APP            , onApp             );
   HANDLE_MSG( hwnd, WM_NCCALCSIZE     , onNCCalcSize      );

   }

   return DefWindowProc( hwnd, msg, wParam, lParam );
}

// end of file
