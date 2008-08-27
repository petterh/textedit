/*
 * $Header: /Book/Window.cpp 21    16.07.04 10:42 Oslph312 $
 */

#include "precomp.h"
#include "Window.h"
#include "AutoArray.h"
#include "Exception.h"


InstanceSubclasser Window::sm_windowSubclasser( Window::wndProc );


Window::~Window() {

   assert( isGoodPtr( this ) );
   if ( IsWindow( m_hwnd ) ) {
      verify( DestroyWindow( *this ) );
      assert( 0 == m_hwnd ); // See XXX comment below.
   }
}


// TODO: throw on invalid hwnd?
void Window::attach( HWND hwnd ) {

   assert( isGoodPtr( this ) );
   assert( IsWindow( hwnd ) );
   m_hwnd = hwnd;
   sm_windowSubclasser.subclass( m_hwnd, this );
   assertValid();
}


void Window::detach( void ) {

   assertValid();
   sm_windowSubclasser.unSubclass( m_hwnd );
   m_hwnd = 0;
}


/**
 * May throw MemoryException.
 */
String Window::getWindowText( void ) const {

   assertValid();
   const int nLength = GetWindowTextLength( *this );
   AutoString pszWindowText( new TCHAR[ nLength + 1 ] );
   GetWindowText( *this, pszWindowText, nLength + 1 );
   return String( pszWindowText );
}


void Window::onPaint( HDC hdc ) {

   assertValid();
}


void Window::onDestroy( void ) {

   assertValid();
}


void Window::onCommand( int id, HWND hwndCtl, UINT codeNotify ) {

   assertValid();
}


LRESULT Window::onNotify( const int id, const LPNMHDR pHdr ) {

   assertValid();
   return sm_windowSubclasser.callOldProc(
      m_hwnd, WM_NOTIFY,
      static_cast< WPARAM >( id ),
      reinterpret_cast< LPARAM >( pHdr ) );
}


void Window::onLButtonDown(
   BOOL fDoubleClick, int x, int y, UINT keyFlags )
{
   assertValid();
}


void Window::onSettingChange( LPCTSTR pszSection ) {

   assertValid();
}


void Window::onSysColorChange( void ) {

   assertValid();
}


void Window::onTimer( UINT id ) {

   assertValid();
}


/**
 * The dispatcher.
 * TODO: Use return values to stop further processing?
 */
LRESULT Window::dispatch( UINT msg, WPARAM wParam, LPARAM lParam ) {

   assertValid();

   switch ( msg ) {
   case WM_PAINT:
      onPaint( reinterpret_cast< HDC >( wParam ) );
      break;

   case WM_DESTROY:
      onDestroy();
      break;

   case WM_COMMAND:
      onCommand(
         GET_WM_COMMAND_ID  ( wParam, lParam ),
         GET_WM_COMMAND_HWND( wParam, lParam ),
         GET_WM_COMMAND_CMD ( wParam, lParam ) );
      break;

   case WM_NOTIFY:
      return
         onNotify( wParam, reinterpret_cast< NMHDR * >( lParam ) );

   case WM_LBUTTONDOWN:
   case WM_LBUTTONDBLCLK:
      onLButtonDown( WM_LBUTTONDBLCLK == msg,
         (int)(short)LOWORD( lParam ),
         (int)(short)HIWORD( lParam ), (UINT) wParam );
      break;

   case WM_SYSCOLORCHANGE:
      onSysColorChange();
      break;

   case WM_TIMER:
      onTimer( (UINT) wParam );
      break;

   case WM_SETTINGCHANGE:
      onSettingChange( reinterpret_cast< LPCTSTR >( lParam ) );
      break;
   }

   const LRESULT lResult =
      sm_windowSubclasser.callOldProc( m_hwnd, msg, wParam, lParam );

#ifdef _DEBUG
   const Window *pWindow = windowFromHWND( m_hwnd );
   if ( this != pWindow ) { // Disconnected.
      assert( 0 == pWindow );
      m_hwnd = 0;           // See XXX comment above.
   }
#endif // _DEBUG

   return lResult;
}


LRESULT CALLBACK Window::wndProc(
   HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
   assert( IsWindow( hwnd ) );

   Window *pWindow = windowFromHWND( hwnd );
   assert( 0 != pWindow );
   if ( 0 == pWindow ) {
      FatalAppExit(
         0, _T( "Window not found in Window::wndProc\n" ) );
   }

   assert( hwnd == pWindow->m_hwnd );
   return pWindow->dispatch( msg, wParam, lParam );
}

// end of file
