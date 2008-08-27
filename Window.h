/*
 * $Header: /Book/Window.h 17    17.12.02 9:39 Oslph312 $
 *
 * The Window class is a general C++ wrapper class for HWNDs.
 * The sendMessage method is a case in point of the strict vs.
 * conceptual const-ness dilemma -- the compiler does not object
 * to a const sendMessage, but SendMessage in general is decidedly
 * not const with regard to the actual HWND.
 *
 * A protected const variant of sendMessage is provided for the
 * benefit of const member functions of derived classes.
 */

#pragma once

#include "String.h"
#include "InstanceSubclasser.h"

class Window {
private:
   static LRESULT CALLBACK wndProc(
      HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );

protected:
   HWND m_hwnd;
   void attach( HWND hwnd );
   void detach( void );
   LRESULT sendMessage(
      UINT msg, WPARAM wParam = 0, LPARAM lParam = 0 ) const;

   static InstanceSubclasser sm_windowSubclasser;

   void assertValid( void );
   void assertValid( void ) const;

   // Message handlers:
   virtual void onPaint( HDC hdc );
   virtual void onDestroy( void );
   virtual void onCommand( int id, HWND hwndCtl, UINT codeNotify );
   virtual LRESULT onNotify( const int id, const LPNMHDR pHdr );
   virtual void onLButtonDown(
      BOOL fDoubleClick, int x, int y, UINT keyFlags );
   virtual void onSettingChange( LPCTSTR pszSection );
   virtual void onSysColorChange( void );
   virtual void onTimer( UINT id );

public:
   Window( HWND = 0 );
   virtual ~Window();

   operator HWND() const;

   LRESULT sendMessage(
      UINT msg, WPARAM wParam = 0, LPARAM lParam = 0 );
   virtual LRESULT dispatch(
      UINT msg, WPARAM wParam = 0, LPARAM lParam = 0 );

   String getWindowText( void ) const;
   void setWindowText( const String& strText );

   static Window *windowFromHWND( HWND hwnd );
};


// Inline Window functions:

inline Window::Window( HWND hwnd ) : m_hwnd( hwnd ) {

   if ( IsWindow( hwnd ) ) {
      attach( hwnd );
      assertValid();
   }
}


/**
 * Note that this operator is safe to call for null pointers.
 * We never expect to do so, hence the assertValid.
 */
inline Window::operator HWND() const {

   assertValid();
   return 0 != this ? m_hwnd : 0;
}


inline LRESULT Window::sendMessage(
   UINT msg, WPARAM wParam, LPARAM lParam )
{
   assertValid();
   return SNDMSG( m_hwnd, msg, wParam, lParam );
}


inline LRESULT Window::sendMessage(
   UINT msg, WPARAM wParam, LPARAM lParam ) const
{
   assertValid();
   return SNDMSG( m_hwnd, msg, wParam, lParam );
}


inline void Window::setWindowText( const String& strText ) {

   assertValid();
   SetWindowText( m_hwnd, strText.c_str() );
}


inline Window *Window::windowFromHWND( HWND hwnd ) {

   assert( IsWindow( hwnd ) );
   Window *pWindow = reinterpret_cast< Window * >(
      sm_windowSubclasser.getUserData( hwnd ) );
   assert( 0 == pWindow || hwnd == pWindow->m_hwnd );
   return pWindow;
}


inline void Window::assertValid( void ) {

   assert( isGoodPtr( this ) );
   assert( IsWindow( m_hwnd ) );
   assert( this == windowFromHWND( m_hwnd ) );
}


inline void Window::assertValid( void ) const {

   assert( isGoodConstPtr( this ) );
   assert( IsWindow( m_hwnd ) );
   assert( this == windowFromHWND( m_hwnd ) );
}

// end of file
