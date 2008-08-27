/*
 * $Header: /Book/saveFile.cpp 9     6-09-01 12:53 Oslph312 $
 *
 * Handles the "Emergency Save As" dialog box and the 
 * Move/Rename dialog box.
 */

#include "precomp.h"
#include "saveFile.h"
#include "os.h"
#include "Exception.h"
#include "AutoArray.h"
#include "AutoHandle.h"
#include "HTML.h"
#include "resolveName.h"
#include "winUtils.h"
#include "formatMessage.h"
#include "openDlgCommon.h"
#include "utils.h"
#include "persistence.h"
#include "resource.h"


PRIVATE PATHNAME s_szFileName = { 0 };
PRIVATE DWORD s_dwErr = 0;


PRIVATE inline void onInitDone( HWND hwndChildDlg ) {

   subclassOpenDlgCommon( hwndChildDlg, IDD_SAVE_CHILD );
   const int nLength = SendDlgItemMessage( 
      hwndChildDlg, IDC_MESSAGE, WM_GETTEXTLENGTH, 0, 0 );
   AutoString pszFmt( new TCHAR[ nLength + 1 ] );
   GetDlgItemText( 
      hwndChildDlg, IDC_MESSAGE, pszFmt, nLength + 1 );
   const String strMessage = formatMessage( 
      String( pszFmt ), s_szFileName, getError( s_dwErr ).c_str() );
   SetDlgItemText( hwndChildDlg, IDC_MESSAGE, strMessage.c_str() );
   subclassHTML( GetDlgItem( hwndChildDlg, IDC_MESSAGE ) );
}


PRIVATE inline LRESULT ofnHook_OnNotify( 
   HWND hwnd, int id, LPNMHDR pNMHDR ) 
{
   assert( 0 != pNMHDR );
   trace( _T( "notify %d %d\n" ), pNMHDR->idFrom, pNMHDR->code );

   switch ( pNMHDR->code ) {

   case CDN_INITDONE:
      onInitDone( hwnd );
      break;

   default:
      break;
   }

   return 0;
}


PRIVATE UINT CALLBACK saveFileHookProc( 
   HWND hwndSubDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
   switch ( msg ) {
   HANDLE_MSG( hwndSubDlg, WM_NOTIFY, ofnHook_OnNotify );
   }

   return FALSE;
}


bool saveFile( const HWND hwndParent, String *pstrName, 
   UINT uiTitleString, UINT uiChildDlg )
   throw( CommonDialogException )
{
   assert( isGoodPtr( pstrName ) );

   _tcscpy( s_szFileName, pstrName->c_str() );
   s_dwErr = GetLastError();

   const bool bOK = getSaveFileName( 
      hwndParent, uiTitleString, saveFileHookProc, 
      s_szFileName, dim( s_szFileName ), uiChildDlg );
   if ( bOK ) {
      pstrName->assign( s_szFileName );
   }

   return bOK;
}

// end of file
