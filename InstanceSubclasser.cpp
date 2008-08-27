/*
 * $Header: /Book/InstanceSubclasser.cpp 7     16.07.04 10:42 Oslph312 $
 * TODO: Base this on the C code instead, to avoid double maintenance.
 */

#include "precomp.h"
#include "InstanceSubclasser.h"
#include "Exception.h"
#include "addAtom.h"
#include "winUtils.h"


#define getWndProc( hwnd ) \
   ( (WNDPROC) GetWindowLong( hwnd, GWL_WNDPROC ) )
#define PROP_NAME MAKEINTRESOURCE( sm_atom )
#define ATOM_NAME _T( "subclass_list_rootPtr" )


class Node {
public:
   WNDPROC m_wndProc;
   WNDPROC m_wndProcSaved;
   void    *m_pUserData;
   Node    *m_pNext;

   Node( HWND hwnd, WNDPROC wndProc, void *pUserData, Node *pNext );
};


ATOM InstanceSubclasser::sm_atom = 0;


Node::Node( HWND hwnd, WNDPROC wndProc, void *pUserData, Node *pNext )
   : m_wndProc     ( wndProc )
   , m_pUserData   ( pUserData )
   , m_pNext       ( pNext )
   , m_wndProcSaved( SubclassWindow( hwnd, wndProc ) )
{
   assert( 0 != m_wndProcSaved );
   trace( _T( "creating node for [%s]: %#x (saving %#x)\n" ), 
      getWindowDescription( hwnd ), m_wndProc, m_wndProcSaved ); 
   if ( 0 == m_wndProcSaved ) {
      throw SubclassException();
   }
}


InstanceSubclasser::InstanceSubclasser( WNDPROC wndProc ) 
   : m_wndProc( wndProc )
{
   assert( isGoodCodePtr( m_wndProc ) );
   trace( _T( "Creating instance subclasser %#x\n" ), wndProc );
   sm_atom = globalAddAtom( ATOM_NAME );
   assert( 0 != sm_atom );
   if ( 0 == sm_atom ) {
      throw SubclassException();
   }
}


InstanceSubclasser::~InstanceSubclasser() throw() {

   trace( _T( "Destroying instance subclasser %#x\n" ), m_wndProc );
   assert( 0 != sm_atom );
   verify( 0 == GlobalDeleteAtom( sm_atom ) );
}


inline Node *InstanceSubclasser::getHead( HWND hwnd ) {

   assert( IsWindow( hwnd ) );
   return reinterpret_cast< Node * >( GetProp( hwnd, PROP_NAME ) );
}


bool InstanceSubclasser::setHead( HWND hwnd, const Node *pNewHead ) {

   assert( IsWindow( hwnd ) );

   if( 0 != GetProp( hwnd, PROP_NAME ) ) {
      verify( RemoveProp( hwnd, PROP_NAME ) );
   }
   if ( 0 != pNewHead ) {
      verify( SetProp( hwnd, PROP_NAME, (HANDLE) pNewHead ) );
   }
   return (HANDLE) pNewHead == GetProp( hwnd, PROP_NAME );
}


inline Node *InstanceSubclasser::findNode( HWND hwnd ) const {
   
   Node *pNode = getHead( hwnd );
   int iCounter = 1000;

   while ( 0 != pNode && m_wndProc != pNode->m_wndProc ) {
      if ( --iCounter < 0 ) {
         assert( false );
         return 0;
      }
      pNode = pNode->m_pNext;
   }
   return pNode;
}


bool InstanceSubclasser::subclass( HWND hwnd, void *pUserData ) {

   assert( IsWindow( hwnd ) );
   WNDPROC const curr = getWndProc( hwnd );
   trace( _T( "subclassing [%s] with %#x (saving %#x)\n" ), 
      getWindowDescription( hwnd ), m_wndProc, curr ); 

   Node *pNode = findNode( hwnd );
   if ( 0 != pNode ) {
      trace( _T( "subclass: %s is already hooked with %#x. " )
         _T( "Replacing user data %#x with %#x\r\n" ), 
         getWindowDescription( hwnd ), m_wndProc,
         pNode->m_pUserData, pUserData );
      pNode->m_pUserData = pUserData;
      return false; //*** METHOD EXIT POINT
   }

   if ( curr == m_wndProc ) {
      trace( _T( "subclass: %#x is already hooked\r\n" ), m_wndProc );
      return false; //*** METHOD EXIT POINT
   }

   Node *pHead = getHead( hwnd );
   pNode = new Node( hwnd, m_wndProc, pUserData, pHead );
   assert( 0 != pNode );
   if ( !setHead( hwnd, pNode ) ) {
      trace( _T( "failed to set head node\n" ) );
      SubclassWindow( hwnd, pNode->m_wndProcSaved );
      delete pNode;
      throw SubclassException();
   }
   return true;
}


bool InstanceSubclasser::unSubclass( HWND hwnd ) {

   trace( _T( "unsubclass %s (%#x)\r\n" ), 
      getWindowDescription( hwnd ), m_wndProc );
   assert( IsWindow( hwnd ) );

   Node *pNode = findNode( hwnd );
   if ( 0 == pNode ) {
      trace( _T( "subclassing %#x does not exist\r\n" ), m_wndProc );
      return FALSE;
   }

   Node *pHead = getHead( hwnd );
   if ( pHead == pNode ) {
      WNDPROC const curr = getWndProc( hwnd );
      if ( curr != m_wndProc ) {
         trace( _T( "unsubclass found unexpected " )
            _T( "wndproc %#x instead of %#x at top of stack\r\n" ),
            curr, pHead->m_wndProc );
         trace( _T( "unsubclass NOT unhooking %#x, leaving %#x\r\n" ),
            m_wndProc, curr );
         return FALSE; //*** METHOD EXIT POINT
      }
      SubclassWindow( hwnd, pNode->m_wndProcSaved );
      setHead( hwnd, pNode->m_pNext );
   } else {
      Node *pCurr = pHead;
      while ( pCurr->m_pNext != pNode ) {
         pCurr = pCurr->m_pNext;
      }
      assert( pCurr->m_pNext == pNode );
      if ( pCurr->m_wndProcSaved != m_wndProc ) {
         trace( _T( "unsubclass: %#x is between %#x and %#x\r\n" )
            _T( "NOT unhooking\r\n" ),
            pCurr->m_wndProcSaved, pCurr->m_wndProc, m_wndProc );
         return FALSE; //*** METHOD EXIT POINT
      }
      pCurr->m_pNext        = pNode->m_pNext;
      pCurr->m_wndProcSaved = pNode->m_wndProcSaved;
   }

   delete pNode;
   return TRUE;
}


void *InstanceSubclasser::getUserData( HWND hwnd ) const {

   const Node *pNode = findNode( hwnd );
   assert( IsWindow( hwnd ) );
   return 0 != pNode ? pNode->m_pUserData : 0;
}


LRESULT InstanceSubclasser::callOldProc( 
   HWND hwnd, UINT uiMsg, WPARAM w, LPARAM l ) 
{
   assert( IsWindow( hwnd ) );

   const Node *pNode = findNode( hwnd );
   assert( 0 != pNode );
   if ( 0 == pNode ) {
      throw SubclassException();
   }
   WNDPROC const oldProc = pNode->m_wndProcSaved;
   assert( 0 != oldProc );

   if ( WM_DESTROY == uiMsg || WM_NCDESTROY == uiMsg ) {
      unSubclass( hwnd );
   }

   return CallWindowProc( oldProc, hwnd, uiMsg, w, l );
}

// end of file
