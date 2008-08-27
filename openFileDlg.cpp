/*
 * $Header: /Book/openFileDlg.cpp 6     16.07.04 10:42 Oslph312 $
 *
 * This module handles the "Open File" dialog box.
 * Would it be appropriate to reset file time stamps after preview?
 */

#include "precomp.h"
#include "openFileDlg.h"
#include "os.h"
#include "Exception.h"
#include "AutoArray.h"
#include "AutoHandle.h"
#include "InstanceSubclasser.h"
#include "openDlgCommon.h"
#include "resolveName.h"
#include "fileUtils.h"
#include "winUtils.h"
#include "utils.h"
#include "persistence.h"
#include "resource.h"


PRIVATE bool s_bValid = false;
PRIVATE bool s_bReadOnly = false;
PRIVATE bool s_bNewWindow = false;

/**
 * Replaces each single \n in the string with \r\n.
 * This simpleminded function is hardly a paragon of efficiency,
 * but then it operates on small amounts of data in user time.
 * An alternative would be to employ a rich edit control.
 */
template< class T >
void fixUnixLineFeeds( T *buf, int buf_size ) {

    for ( int i = 0; i < buf_size || 0 == buf[ i ]; ++i ) {
        if ( '\n' == buf[ i ] ) {
            if ( 0 == i || '\r' != buf[ i - 1 ] ) {
                size_t bytes_to_move = (buf_size - i - 1) * sizeof( T );
                memmove( buf + i + 1, buf + i, bytes_to_move );
                buf[ i ] = '\r'; // TODO i++
            }
        }
    }
    buf[ buf_size - 1 ] = 0;
}


PRIVATE void showPreview( HANDLE hIn, HWND hwndPreview ) {

   CHAR szContents[ 4096 ] = { 0 };
   DWORD dwBytesRead = 0;
   const BOOL bOK = ReadFile( hIn, szContents,
      sizeof szContents - sizeof( WCHAR ), &dwBytesRead, 0 );
   if ( bOK ) {
      assert( dwBytesRead <= sizeof szContents - sizeof( WCHAR ) );
      if ( isTextUnicode( szContents, dwBytesRead, 0 ) ) {
         // NOTE: isTextUnicode always returns false under Win95.
         assert( isWindowsNT() );
         LPWSTR pszwContents =
            reinterpret_cast< LPWSTR >( szContents );
         pszwContents[ dwBytesRead / sizeof( WCHAR ) ] = 0;
         const size_t wchars_per_char = sizeof( WCHAR ) / sizeof( CHAR );
         fixUnixLineFeeds( pszwContents, sizeof szContents / wchars_per_char );
         SetWindowTextW( hwndPreview, pszwContents );
      } else {
         szContents[ dwBytesRead ] = 0;
         fixUnixLineFeeds( szContents, sizeof szContents );
         SetWindowTextA( hwndPreview, szContents );
      }
   }
}


PRIVATE void updatePreview( HWND hwndOpenDlg, HWND hwndPreview ) {

   trace( _T( "updatePreview\n" ) );

   assert( IsWindow( hwndOpenDlg ) );
   assert( IsWindow( hwndPreview ) );

   SetWindowText( hwndPreview, _T( "" )  );
   PATHNAME szFile = { 0 };
   s_bValid = false;
   const LRESULT lResult = SNDMSG(
      hwndOpenDlg, CDM_GETFILEPATH, dim( szFile ), (LPARAM) szFile );
   HICON hicon = 0;
   if ( 0 < lResult ) {
      const bool isShortcut = resolveName( szFile, szFile );
      AutoHandle hIn( CreateFile( szFile, GENERIC_READ,
         FILE_SHARE_READ_WRITE, 0, OPEN_EXISTING, 0, 0 ) );
      s_bValid = INVALID_HANDLE_VALUE != hIn;
      if ( s_bValid ) {
         showPreview( hIn, hwndPreview );

         BY_HANDLE_FILE_INFORMATION bhFileInfo = { 0 };
         s_bReadOnly =
            !GetFileInformationByHandle( hIn, &bhFileInfo ) ||
            ( bhFileInfo.dwFileAttributes & FILE_ATTRIBUTE_READONLY );

         SHFILEINFO fileInfo = { 0 };
         UINT uiFlags = SHGFI_ICON | SHGFI_SMALLICON;
         if ( isShortcut ) {
            uiFlags |= SHGFI_LINKOVERLAY;
         }
         SHGetFileInfo( szFile, 0,
            &fileInfo, sizeof fileInfo, uiFlags );
         hicon = fileInfo.hIcon;
      }
   }

   HWND hwndIcon =
      GetDlgItem( GetParent( hwndPreview ), IDC_FILEICON );
   Static_SetIcon( hwndIcon, hicon );
}


PRIVATE const UINT uiPreviewTimer = 1;

PRIVATE void setPreviewTimer( HWND hwndSubDlg ) {

   assert( IsWindow( hwndSubDlg ) );

   int nDelay = 0;
   SystemParametersInfo( SPI_GETKEYBOARDDELAY, 0, &nDelay, 0 );
   assert( 0 <= nDelay && nDelay <= 3 );
   nDelay &= 0x3;
   const int nPreviewTimeout = 250 * (1 + nDelay) + 100;
   SetTimer( hwndSubDlg, uiPreviewTimer, nPreviewTimeout, 0 );
}


PRIVATE LRESULT CALLBACK openDlgSubclassWndProc(
   HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );
PRIVATE LRESULT CALLBACK PreviewSubclassWndProc(
   HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );


PRIVATE InstanceSubclasser s_openDlgSub( openDlgSubclassWndProc );
PRIVATE InstanceSubclasser s_previewSub( PreviewSubclassWndProc );


/**
 * This is used to subclass the preview edit in order to catch
 * the enter key. Default buttons and subdialogs don't mesh well.
 */
PRIVATE LRESULT CALLBACK PreviewSubclassWndProc(
   HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
   const LRESULT lResult =
      s_previewSub.callOldProc( hwnd, msg, wParam, lParam );
   if ( WM_KEYDOWN == msg && VK_RETURN == wParam ) {
      HWND hwndOpenDlg =
         s_previewSub.getUserDataAsHwnd( hwnd );
      assert( hwndOpenDlg == GetParent( GetParent( hwnd ) ) );
      FORWARD_WM_COMMAND( hwndOpenDlg,
         IDOK, 0, BN_CLICKED, PostMessage );
   }
   return lResult;
}


/**
 * This is used to subclass the common open dialog in order to
 * catch notifications from the file name edit control.
 * When it changes, we want a chance to update the preview.
 */
PRIVATE LRESULT CALLBACK openDlgSubclassWndProc(
   HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
   const LRESULT lResult =
      s_openDlgSub.callOldProc( hwnd, msg, wParam, lParam );
   if ( WM_COMMAND == msg                               &&
      edt1      == GET_WM_COMMAND_ID ( wParam, lParam ) &&
      EN_CHANGE == GET_WM_COMMAND_CMD( wParam, lParam ) )
   {
      trace( _T( "EN_CHANGE\n" ) );
      HWND hwndSubDlg = s_openDlgSub.getUserDataAsHwnd( hwnd );
      assert( IsWindow( hwndSubDlg ) );
      setPreviewTimer( hwndSubDlg );
   }
   return lResult;
}


PRIVATE void positionPreview( HWND hwndChildDlg ) {

   HWND hwndOpenDlg = GetParent( hwndChildDlg );
   assert( IsWindow( hwndOpenDlg ) );
   assert( IsWindow( hwndChildDlg ) );

   HWND hwndPreview = GetDlgItem( hwndChildDlg, IDC_PREVIEW );
   assert( IsWindow( hwndChildDlg ) );

   HWND hwndNewWindow = GetDlgItem( hwndChildDlg, IDC_NEW_WINDOW );
   assert( IsWindow( hwndNewWindow ) );

   HWND hwndCancel = GetDlgItem( hwndOpenDlg, IDCANCEL );
   assert( IsWindow( hwndCancel ) );

   const Rect rcCancel = getWindowRectInParent( hwndCancel );
   Rect rcNewWindow = getWindowRectInParent( hwndNewWindow );
   const int nDelta = rcCancel.bottom - rcNewWindow.bottom;
   OffsetRect( &rcNewWindow, 0, nDelta );
   moveWindow( hwndNewWindow, &rcNewWindow );

   Rect rcPreview = getWindowRectInParent( hwndPreview );
   rcPreview.bottom = rcNewWindow.top - 4;
   moveWindow( hwndPreview, &rcPreview );

   HWND hwndIcon = GetDlgItem( hwndChildDlg, IDC_FILEICON );
   const int cx = GetSystemMetrics( SM_CXSMICON );
   const int cy = GetSystemMetrics( SM_CYSMICON );
   rcPreview.left = rcPreview.right - cx;
   rcPreview.bottom = rcPreview.top - 2;
   rcPreview.top -= cy + 2;
   moveWindow( hwndIcon, &rcPreview );
}


PRIVATE inline HBRUSH ofnHook_OnCtlColor(
   HWND hwnd, HDC hdc, HWND hwndChild, int type )
{
   if ( GetDlgItem( hwnd, IDC_PREVIEW ) == hwndChild ) {
      const bool bNormal = s_bValid && !s_bReadOnly;
      SetTextColor( hdc, GetSysColor( COLOR_WINDOWTEXT ) );
      SetBkColor( hdc, GetSysColor(
         bNormal ? COLOR_WINDOW : COLOR_3DFACE ) );
      return
         GetSysColorBrush( bNormal ? COLOR_WINDOW : COLOR_3DFACE );
   }
   return 0;
}


PRIVATE inline void ofnHook_OnTimer( HWND hwnd, UINT id ) {
   trace( _T( "WM_TIMER %d\n" ), id );
   KillTimer( hwnd, id );
   HWND hwndPreview = GetDlgItem( hwnd, IDC_PREVIEW );
   assert( IsWindow( hwndPreview ) );
   updatePreview( GetParent( hwnd ), hwndPreview );
}


PRIVATE inline void onInitDone( HWND hwndChildDlg ) {

   subclassOpenDlgCommon( hwndChildDlg, IDD_PREVIEW_CHILD );
   positionPreview( hwndChildDlg );

   // If these subclassings fail, we can live with the consequences:
   try {
      HWND hwndOpenDlg = GetParent( hwndChildDlg );
      HWND hwndPreview = GetDlgItem( hwndChildDlg, IDC_PREVIEW );
      verify( s_openDlgSub.subclass(
         hwndOpenDlg, hwndChildDlg ) );
      verify( s_previewSub.subclass(
         hwndPreview, hwndOpenDlg ) );
   }
   catch ( const SubclassException& x ) {
      trace( _T( "%s\n" ), x.what() );
   }

   if ( !s_bNewWindow )  {
      HWND hwndNewWindow = GetDlgItem( hwndChildDlg, IDC_NEW_WINDOW );
      assert( IsWindow( hwndNewWindow ) );
      Button_SetCheck( hwndNewWindow, true );
      EnableWindow( hwndNewWindow, false );
   }
}


PRIVATE inline LRESULT ofnHook_OnNotify(
   HWND hwnd, int id, LPNMHDR pNMHDR )
{
   //const OFNOTIFY *pofn = reinterpret_cast< OFNOTIFY * >( lParam );
   assert( 0 != pNMHDR );
   trace( _T( "notify %d %d\n" ), pNMHDR->idFrom, pNMHDR->code );

   switch ( pNMHDR->code ) {

   case CDN_INITDONE:
      onInitDone( hwnd );
      break;

   case CDN_FILEOK:
      s_bNewWindow = 0 != Button_GetCheck(
         GetDlgItem( hwnd, IDC_NEW_WINDOW ) );
      //*** FALL THROUGH

   case CDN_SELCHANGE:
   case CDN_FOLDERCHANGE:
   case CDN_TYPECHANGE:
      trace( _T( "starting timer\n" ) );
      setPreviewTimer( hwnd );
      break;

   default:
      break;
   }

   return 0;
}


PRIVATE inline void ofnHook_OnCommand(
   HWND hwnd, int id, HWND hwndCtl, UINT codeNotify )
{
   if ( ( IDOK == id || IDCANCEL == id ) &&
        BN_CLICKED == codeNotify )
   {
      FORWARD_WM_COMMAND(
         GetParent( hwnd ), id, hwndCtl, codeNotify, PostMessage );
   }
}


PRIVATE UINT CALLBACK openFileHookProc(
   HWND hwndSubDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
   switch ( msg ) {
   HANDLE_MSG( hwndSubDlg, WM_CTLCOLORSTATIC, ofnHook_OnCtlColor );
   HANDLE_MSG( hwndSubDlg, WM_TIMER         , ofnHook_OnTimer    );
   HANDLE_MSG( hwndSubDlg, WM_NOTIFY        , ofnHook_OnNotify   );
   HANDLE_MSG( hwndSubDlg, WM_COMMAND       , ofnHook_OnCommand  );
   }

   return FALSE;
}


bool openFileDlg(
   const HWND hwndParent, LPTSTR pszName, int cb,
   bool *pbNewWindow,
   bool bMustExist )
   throw( CommonDialogException )
{
   s_bNewWindow = false;
   if ( 0 != pbNewWindow ) {
      assert( isGoodPtr( pbNewWindow ) );
      s_bNewWindow = *pbNewWindow;
   }

   const bool bOK = getOpenFileName(
      hwndParent,
      bMustExist ? IDS_OPENFILE : IDS_OPENORCREATEFILE,
      openFileHookProc, pszName, cb, IDD_PREVIEW_CHILD,
      bMustExist ? OFN_FILEMUSTEXIST : 0 );
   if ( 0 != pbNewWindow && *pbNewWindow ) {
      *pbNewWindow = s_bNewWindow;
   }
   return bOK;
}

// end of file
