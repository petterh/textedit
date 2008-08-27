/*
 * $Header: /Cleaner/Dialog.h 15    22.03.02 13:02 Oslph312 $
 */

#pragma once

#include "String.h"
#include "Window.h"

class Dialog : public Window {
private:
    HWND _hwndMouseWheelHandler;

protected:
   Dialog();
   virtual ~Dialog();

   virtual void onDestroy( void );

   virtual UINT getResourceID( void ) const = 0;
   virtual BOOL DlgProc( UINT msg, WPARAM wParam, LPARAM lParam );
   virtual BOOL onInitDialog( HWND hwndFocus, LPARAM lParam );
   virtual void onDlgCommand( int id, HWND hwndCtl, UINT codeNotify );
   virtual void onDlgNotify( int id, NMHDR *pNmHdr );

   void __cdecl setDlgItemTextF( UINT id, const String strFmt, ... );
   void __cdecl setDlgItemTextF( UINT id, UINT uiFmtID, ... );

   void setDlgItemText( UINT id, const String& str );
   void setDlgItemText( UINT id, UINT idString );

   String getDlgItemText( const UINT id );
   void enableDlgItem( const UINT id, const bool bEnable );
   void showDlgItem( const UINT id );
   void hideDlgItem( const UINT id );
   void gotoDlgItem( const UINT id );
   void gotoNextDlgItem( void );
   void toggleIcon( UINT idOn, UINT idOff, bool bChecked );
   HWND getDlgItem( UINT id ) const;

   void setDefaultMouseWheelHandler( HWND hwndCtrl ) {
       assert( IsWindow( hwndCtrl ) );
       _hwndMouseWheelHandler = hwndCtrl;
   }

private:
   BOOL dispatchDlgMsg( UINT msg, WPARAM wParam, LPARAM lParam );
   static BOOL CALLBACK DlgProc( 
      HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );

public:
   UINT doModal( HWND hwndParent, UINT uiResource = 0 );
   UINT doModal( UINT uiResource = 0 );
};


inline void Dialog::enableDlgItem( UINT id, bool bEnable ) {
   assert( IsWindow( *this ) );
   HWND hwndCtl = getDlgItem( id );
   assert( IsWindow( hwndCtl ) );
   EnableWindow( hwndCtl, bEnable );
}


inline void Dialog::showDlgItem( const UINT id ) {
   assert( IsWindow( *this ) );
   HWND hwndCtl = getDlgItem( id );
   assert( IsWindow( hwndCtl ) );
   ShowWindow( hwndCtl, SW_SHOW );
}


inline void Dialog::hideDlgItem( const UINT id ) {
   assert( IsWindow( *this ) );
   HWND hwndCtl = getDlgItem( id );
   assert( IsWindow( hwndCtl ) );
   ShowWindow( hwndCtl, SW_HIDE );
}


inline void Dialog::gotoNextDlgItem( void ) {
   FORWARD_WM_NEXTDLGCTL( *this, 0, FALSE, SNDMSG );
}


inline void Dialog::gotoDlgItem( const UINT id ) {
   FORWARD_WM_NEXTDLGCTL( *this, getDlgItem( id ), TRUE, SNDMSG );
}


inline HWND Dialog::getDlgItem( UINT id ) const {
   return GetDlgItem( *this, id );
}


inline UINT Dialog::doModal( UINT uiResource ) {

   return doModal( HWND_DESKTOP, uiResource );
}


// end of file
