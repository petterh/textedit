/*
 * $Header: /Book/printFile.cpp 14    20.08.99 16:33 Oslph312 $
 */

#include "precomp.h"
#include "AutoArray.h"
#include "Exception.h"
#include "Editor.h"
#include "AutoGlobalMemoryHandle.h"
#include "printFile.h"
#include "winUtils.h"
#include "devMode.h"
#include "devNames.h"
#include "formatMessage.h"
#include "resource.h"
#include <dlgs.h>


#define PD_COMMON_TEXTEDIT (PD_ENABLEPRINTHOOK | PD_RETURNDC)


PRIVATE inline BOOL onInitDialog( 
   HWND hwnd, HWND hwndFocus, LPARAM lParam ) 
{
   PRINTDLG *pPrintDlg = reinterpret_cast< PRINTDLG * >( lParam );
   assert( isGoodPtr( pPrintDlg ) );
   Document *pDocument = reinterpret_cast< Document * >( 
      pPrintDlg->lCustData );
   if ( 0 != pDocument ) {
      assert( isGoodPtr( pDocument ) );
      const String strTitle = formatMessage( IDS_PRINT_HEADER,
         pDocument->getTitle().c_str() );
      SetWindowText( hwnd, strTitle.c_str() );
   }

   restorePosition( hwnd, IDD_PRINT );
   if ( 1 < pPrintDlg->nCopies ) {
      HWND hwndCopies = GetDlgItem( hwnd, edt3 );
      if ( IsWindow( hwndCopies ) ) {
         SetFocus( hwndCopies );
         Edit_SetSel( hwndCopies, 0, -1 );
         return FALSE;
      }
   }
   return TRUE;
}


PRIVATE inline void onDestroy( HWND hwnd ) {

   savePosition( hwnd, IDD_PRINT );
}


UINT CALLBACK PrintHookProc( 
   HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) 
{
   switch ( msg ) {
   HANDLE_MSG( hwnd, WM_INITDIALOG, onInitDialog );
   HANDLE_MSG( hwnd, WM_DESTROY   , onDestroy    );
   }

   return 0;
}


class AutoDC {
private:
   HDC m_hdc;

public:
   AutoDC( HDC hdc ) : m_hdc( hdc ) {
   }
   ~AutoDC() { 
      verify( DeleteDC( m_hdc ) ); 
   }
};


/**
 * This printFile is used with the /p and /pt command line switches.
 */
void printFile( Document *pDocument, 
   LPCTSTR pszPrinter, LPCTSTR pszDriver, LPCTSTR pszPort )
{
   PRINTDLG printDlg = {
      sizeof( PRINTDLG ),
      HWND_DESKTOP, getDevMode(), 
      getDevNames( pszPrinter, pszDriver, pszPort ),
      0, PD_COMMON_TEXTEDIT | PD_NOSELECTION,
      0, 0, 0, 0, 1, 0, reinterpret_cast< DWORD >( pDocument ),
      PrintHookProc,
   };

   const AutoGlobalMemoryHandle a1( printDlg.hDevMode  );
   const AutoGlobalMemoryHandle a2( printDlg.hDevNames );
   
   if ( 0 != pszPrinter ) {
      const DEVMODE *pDevMode = 0;
      if ( 0 != printDlg.hDevMode ) {
         pDevMode = reinterpret_cast< DEVMODE * >( 
            GlobalLock( printDlg.hDevMode ) );
      }
      printDlg.hDC = CreateDC( pszDriver, pszPrinter, 0, pDevMode );
      if ( 0 != printDlg.hDevMode ) {
         GlobalUnlock( printDlg.hDevMode );
      }
   } else {
      const BOOL bOK = PrintDlg( &printDlg );
      if ( bOK ) {
         setDevMode ( printDlg.hDevMode  );
         setDevNames( printDlg.hDevNames );
      } else {
         const DWORD dwErr = CommDlgExtendedError();
         if ( ERROR_SUCCESS == dwErr ) {
            return; // The user hit cancel; OK.
         } else {
            throw CommonDialogException( dwErr );
         }
      }

   }

   if ( 0 == printDlg.hDC ) {
      throwException( _T( "Can't get a printer device context" ) );
   }
   const AutoDC autoDC( printDlg.hDC );
   const AutoString pszText( pDocument->getContents() );
   pDocument->print( printDlg.hDC, pszText );
}


/**
 * This printFile is used for normal editing sessions.
 * # of copies is persistent only within session.
 */
void Editor::printFile( void ) {

   assert( isGoodConstPtr( this ) );
   assert( IsWindow( m_hwndMain ) );

   static int s_nCopies = 1; // Retain per session.

   PRINTDLG printDlg = {
      sizeof( PRINTDLG ),
      GetLastActivePopup( m_hwndMain ), 
      getDevMode(), getDevNames(), 0, 
      PD_COMMON_TEXTEDIT, 0, 0, 0, 0, s_nCopies, 0, 0, PrintHookProc,
   };

   const AutoGlobalMemoryHandle a1( printDlg.hDevMode  );
   const AutoGlobalMemoryHandle a2( printDlg.hDevNames );
   
   if ( m_pEditWnd->getSel() ) {
      printDlg.Flags |= PD_SELECTION;
   } else {
      printDlg.Flags |= PD_NOSELECTION;
   }

   FORWARD_WM_COMMAND( 
      m_hwndMain, ID_COMMAND_RESETSTATUSBAR, 0, 0, SNDMSG );

   const BOOL bOK = PrintDlg( &printDlg );
   if ( bOK ) {
      s_nCopies = printDlg.nCopies;
      setDevMode ( printDlg.hDevMode  );
      setDevNames( printDlg.hDevNames );
      if ( 0 == printDlg.hDC ) {
         throwException( _T( "Can't get a printer device context" ) );
      }
      const AutoDC autoDC( printDlg.hDC );
      TemporaryStatusIcon statusIcon( m_pStatusbar, STD_PRINT );
      if ( printDlg.Flags & PD_SELECTION ) {
         String strText;
         assert( m_pEditWnd->getSel( &strText ) );
         m_pDocument->print( printDlg.hDC, strText.c_str() );
      } else {
         AutoString pszText( m_pDocument->getContents() );
         m_pDocument->print( printDlg.hDC, pszText, s_nCopies );
      }
   }
}

// end of file
