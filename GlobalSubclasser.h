/*
 * $Header: /Book/GlobalSubclasser.h 3     5.09.99 13:07 Oslph312 $
 */

#pragma once


class GlobalSubclasser {
private:
   LPCTSTR m_pszWndClass;
   WNDPROC m_wndProc;
   WNDPROC m_wndProcSaved;

public:
   GlobalSubclasser( LPCTSTR pszWndClass, WNDPROC wndProcNew );
   virtual ~GlobalSubclasser() throw();
   LRESULT callOldProc( 
      HWND hwnd, UINT uiMsg, WPARAM wParam, LPARAM lParam );
};


inline LRESULT GlobalSubclasser::callOldProc( 
   HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) 
{
   assert( IsWindow( hwnd ) );
   assert( 0 != m_wndProcSaved );
   return CallWindowProc( m_wndProcSaved, hwnd, msg, wParam, lParam );
}

// end of file
