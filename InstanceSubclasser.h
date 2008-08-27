/*
 * $Header: /Book/InstanceSubclasser.h 6     6-09-01 11:35 Oslph312 $
 */

#pragma once

class Node;
class InstanceSubclasser {
private:
   WNDPROC m_wndProc;
   static ATOM sm_atom;

   static Node *getHead( HWND hwnd );
   static bool setHead( HWND hwnd, const Node *pNewHead );
   Node *findNode( HWND hwnd ) const;

public:
   explicit InstanceSubclasser( WNDPROC wndProc );
   virtual ~InstanceSubclasser() throw();
   bool subclass( HWND hwnd, void *pUserData = 0 );
   virtual bool unSubclass( HWND hwnd );
   void *getUserData( HWND hwnd ) const;
   LRESULT callOldProc( 
      HWND hwnd, UINT uiMsg, WPARAM wParam, LPARAM lParam );

   // Convenience for subclassings that use user data to store a HWND:
   HWND getUserDataAsHwnd( HWND hwnd );
};


inline HWND InstanceSubclasser::getUserDataAsHwnd( HWND hwnd ) {

   return reinterpret_cast< HWND >(  getUserData( hwnd ) );
}

// end of file
