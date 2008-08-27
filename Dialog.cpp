/*
 * $Header: /Book/Dialog.cpp 29    16.07.04 10:42 Oslph312 $
 */

#include "precomp.h"
#include "Dialog.h"
#include "InstanceSubclasser.h"
#include "AutoArray.h"
#include "Exception.h"
#include "formatMessage.h"
#include "winUtils.h"
#include "resource.h"
#include "help.h"
#include "utils.h"
#include "trace.h"
#include "wnd_snap.h"


#undef EXCEPTION_PROTECTION

Dialog::Dialog() : _hwndMouseWheelHandler( 0 ) {
}


Dialog::~Dialog() {
   setHelpID( 0 );
}


/**
 * This should never be called!
 */
UINT Dialog::getResourceID( void ) const {

   trace( _T( "pure virtual func Dialog::getResourceID called\n" ) );
   debugBreak(); // This method should never be called!
   return 0;
}


/**
 * NOTE: Neither WM_DESTROY nor WM_NCDESTROY 
 * get here under normal circumstances.
 */
BOOL Dialog::dispatchDlgMsg( 
   UINT msg, WPARAM wParam, LPARAM lParam ) 
{
    switch ( msg ) {
    case WM_INITDIALOG:
        setSnap( *this, true );
        restorePosition( *this, getResourceID() );
        return onInitDialog( (HWND) wParam, lParam );
        
    case WM_COMMAND:
        return onDlgCommand( 
            GET_WM_COMMAND_ID  ( wParam, lParam ) ,
            GET_WM_COMMAND_HWND( wParam, lParam ) ,
            GET_WM_COMMAND_CMD ( wParam, lParam ) ), TRUE;
        
    case WM_MOUSEWHEEL:
        if ( IsWindow( _hwndMouseWheelHandler ) ) {
            return (BOOL) ::SendMessage(
                _hwndMouseWheelHandler, WM_MOUSEWHEEL, wParam, lParam );
        }
        break;

    case WM_NOTIFY:
        return onDlgNotify( (int) wParam, (NMHDR *) lParam ), TRUE;
    }
    
    return DlgProc( msg, wParam, lParam );
}


BOOL Dialog::DlgProc( UINT msg, WPARAM, LPARAM ) {

    return FALSE;
}


/**
 * NOTE: WM_NCDESTROY normally never gets here, and
 * when WM_DESTROY arrives, weære normally unhooked already.
 */
BOOL CALLBACK Dialog::DlgProc( 
   HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    assert( ::IsWindow( hwnd ) );
    
    if ( WM_INITDIALOG == msg ) {
        Dialog *pDlg = reinterpret_cast< Dialog * >( lParam );
        assert( isGoodPtr( pDlg ) );
        pDlg->attach( hwnd ); 
    }
    
    Dialog *pDlg = static_cast< Dialog * >( windowFromHWND( hwnd ) );
    if ( 0 != pDlg ) {
        assert( isGoodPtr( pDlg ) );
        return pDlg->dispatchDlgMsg( msg, wParam, lParam );
    }
    // If pDlg is 0, we've been unhooked, and are perhaps processing WM_DESTROY.
    // Or we haven't gotten WM_INITDIALOG yet.

    return FALSE;
}


void Dialog::onDestroy( void ) {
   savePosition( *this, getResourceID() );
}


BOOL Dialog::onInitDialog( HWND hwndFocus, LPARAM lParam ) {
   return TRUE;
}


/**
 * This is not named onCommand, since that would override 
 * the Window method rather than the Dialog method, and, 
 * among other things, screw up the drop-down handling
 * (in dlgSubClasser.cpp).
 */
void Dialog::onDlgCommand( int id, HWND hwndCtl, UINT codeNotify ) {

   switch ( id ) {
   case IDOK:
   case IDCANCEL:
      verify( EndDialog( *this, id ) );
      break;
   }
}


void Dialog::onDlgNotify( int, NMHDR * ) {
}


UINT Dialog::doModal( HWND hwndParent, UINT uiResource ) {

   assert( HWND_DESKTOP == hwndParent || IsWindowEnabled( hwndParent ) );

#ifdef ID_COMMAND_RESETSTATUSBAR

   if ( HWND_DESKTOP != hwndParent ) {
      FORWARD_WM_COMMAND( hwndParent, ID_COMMAND_RESETSTATUSBAR, 0, 0, SNDMSG );
   }

#endif // ID_COMMAND_RESETSTATUSBAR

   const HINSTANCE hinst = getModuleHandle();

#ifdef EXCEPTION_PROTECTION
   try {
#endif //  EXCEPTION_PROTECTION

      if ( 0 == uiResource ) {
         uiResource = getResourceID();
      }
      setHelpID( uiResource );
      
      assert( 0 != uiResource );
      assert( 0 != hinst );
      assert( HWND_DESKTOP == hwndParent || IsWindow( hwndParent ) );

      const INT_PTR result = DialogBoxParam( hinst,
         MAKEINTRESOURCE( uiResource ), hwndParent, DlgProc, 
         reinterpret_cast< LPARAM >( this ) );
      if ( -1 == result ) {
          const DWORD last_error = GetLastError();
          const String description = getError( last_error );
          trace( _T( "Dialog::doModal: DialogBoxParam returns %d (%#x)\n" ), result, result );
          trace( _T( "\tGetLastError() returns %d (%#x): %s" ),
               last_error, last_error, description.c_str() );
          if ( NO_ERROR == last_error ) {
            trace( _T( "\tHint: Inability to create dialog may be due to lack of InitCommonControls()\n" ) );
          }
      }
      return result;
#ifdef EXCEPTION_PROTECTION
   }
   catch ( ... ) {
      trace( _T( "Exception in Dialog::doModal %d (%#x)\n" ), uiResource, uiResource );
#if 1
      if ( IsWindow( *this ) ) {
         HWND hwndThis = *this;
         detach();
         EndDialog( hwndThis, -1 );
         //DestroyWindow( *this );
      }
#endif

      // To avoid, say, an access violation freezing the parent:
      if ( HWND_DESKTOP != hwndParent && IsWindow( hwndParent ) ) {
         EnableWindow( hwndParent, true );
      }
      throw;
   }
#endif //  EXCEPTION_PROTECTION
}


void Dialog::toggleIcon( UINT idOn, UINT idOff, bool bChecked ) {
   
   if ( bChecked ) {
      hideDlgItem( idOff );
      showDlgItem( idOn  );
   } else {
      hideDlgItem( idOn  );
      showDlgItem( idOff );
   }
}


/**
 * Formats a message using the formatMessageV function, 
 * See formatMessage.cpp for further comments.
 */
void __cdecl Dialog::setDlgItemTextF( UINT id, const String strFmt, ... ) {

   va_list vl;
   va_start( vl, strFmt );
   const String strMessage = formatMessageV( strFmt, vl );
   va_end( vl );

   setDlgItemText( id, strMessage );
}


/**
 * Formats a message using the formatMessageV function, 
 * See formatMessage.cpp for further comments.
 */
void __cdecl Dialog::setDlgItemTextF( UINT id, UINT uiFmtID, ... ) {

   const String strFmt = loadString( uiFmtID );

   va_list vl;
   va_start( vl, uiFmtID );
   const String strMessage = formatMessageV( strFmt, vl );
   va_end( vl );

   setDlgItemText( id, strMessage );
}


void Dialog::setDlgItemText( UINT id, const String& str ) {

   HWND hwndCtl = getDlgItem( id );
   assert( IsWindow( hwndCtl ) );
   //LockWindowUpdate( hwndCtl );
   SetWindowText( hwndCtl, str.c_str() );
   //LockWindowUpdate( 0 );
}


void Dialog::setDlgItemText( UINT id, UINT idString ) {

   setDlgItemText( id, loadString( idString ) );
}


/**
 * Throws a MemoryException if the allocation fails.
 */
String Dialog::getDlgItemText( const UINT id ) {

   const int nLength = 
      SendDlgItemMessage( *this, id, WM_GETTEXTLENGTH, 0, 0 );
   AutoString pszText( new TCHAR[ nLength + 1 ] );
   GetDlgItemText( *this, id, pszText, nLength + 1 );
   return (LPCTSTR) pszText;
}

// end of file
