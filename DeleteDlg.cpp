/*
 * $Header: /Book/DeleteDlg.cpp 10    3.07.99 17:46 Oslph312 $
 */

#include "precomp.h"
#include "resource.h"
#include "formatMessage.h"
#include "winUtils.h"
#include "DeleteDlg.h"
#include "HTML.h"
#include "persistence.h"


BOOL DeleteDlg::onInitDialog( HWND hwndFocus, LPARAM lParam ) {

   const String strFmt = getDlgItemText( IDC_INFO );
   const String strInfo = formatMessage( strFmt, m_strFile.c_str() );
   setDlgItemText( IDC_INFO, strInfo );
   subclassHTML( getDlgItem( IDC_INFO ) );
   subclassHTML( getDlgItem( IDC_TIP ) );

   const bool bSendToWasteBasket = 0 != getSendToWasteBasket();
   Button_SetCheck( 
      getDlgItem( IDC_TRASHCAN ), bSendToWasteBasket ? 1 : 0 );
   toggleIcon( 
      IDC_WASTEBASKET, IDC_WASTEBASKETEMPTY, bSendToWasteBasket );
   
   // This is always true if we're shoing the dialog!
   Button_SetCheck( getDlgItem( IDC_SHOWDELETEDIALOG ), true);

   return TRUE; // ...since we didn't set the focus.
}


void DeleteDlg::onDlgCommand( 
   int id, HWND hwndCtl, UINT codeNotify ) 
{
   switch ( id ) {
   case IDC_TRASHCAN:
      if ( BN_CLICKED == codeNotify ) {
         toggleIcon( IDC_WASTEBASKET, IDC_WASTEBASKETEMPTY, 
            0 != Button_GetCheck( getDlgItem( IDC_TRASHCAN ) ) );
      }
      break;

   case IDOK:
      if ( BN_CLICKED == codeNotify ) {
         setSendToWasteBasket( Button_GetCheck( 
            getDlgItem( IDC_TRASHCAN ) ) ? 1 : 0 );
         setShowDeleteDialog( Button_GetCheck( 
            getDlgItem( IDC_SHOWDELETEDIALOG ) ) ? 1 : 0 );
      }

      //*** FALL THROUGH

   case IDCANCEL:
      if ( BN_CLICKED == codeNotify ) {
         verify( EndDialog( *this, id ) );
      }
      break;
   }
}


UINT DeleteDlg::getResourceID( void ) const {
   return IDD_DELETE;
}


DeleteDlg::DeleteDlg( const String& strFile ) 
   : m_strFile( strFile )
{
}

// end of file
