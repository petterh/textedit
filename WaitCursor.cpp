/*
 * $Header: /Cleaner/WaitCursor.cpp 15    22.03.02 12:58 Oslph312 $
 *
 * Thread challenges:
 * 1. SetCursor from a different thread has no effect unless thread 
 *    input has been attached.
 * 2. Animation always works under debugger. With no debugger, requires
 *    a running thread. Which is why there are *two* calls to 
 *    WaitForSingleObject in _threadFunc.
 *
 * Since we don't need CRT in the thread, plain Win thread function is OK.
 */

#include "precomp.h"
#include "String.h"
#include "WaitCursor.h"
#include "trace.h"


#define CURSOR_PATH _T( "Cursors\\" )


PRIVATE inline HCURSOR loadCursor( LPCTSTR pszName ) {
   return static_cast< HCURSOR >( LoadImage( 0, pszName, 
      IMAGE_CURSOR, 0, 0, 
      LR_DEFAULTSIZE | LR_DEFAULTCOLOR | LR_LOADFROMFILE ) );
}


/*
 * If we don't call AttachThreadInput, SetCursor has 
 * no effect when called from the worker thread.
 */
inline void WaitCursor::_attachThreadInput( bool bAttach ) const {

   verify( AttachThreadInput( 
      GetCurrentThreadId(), _dwParentThreadId, bAttach ) );
}


void WaitCursor::_threadFunc( void ) const {

   const DWORD dwRet = WaitForSingleObject( _hEvent, _nTimeIn );
   if ( WAIT_TIMEOUT == dwRet ) {
      _attachThreadInput( true );
      _restore();
      _attachThreadInput( false ); // Works OK withouth this...
      // ...but without this, no animation unless in debugger.
      verify( WAIT_OBJECT_0 == WaitForSingleObject( _hEvent, INFINITE ) );
   }
   assert( (DWORD) -1 != dwRet );
}


DWORD WINAPI WaitCursor::_threadFunc( void *pData ) {

   const WaitCursor *pThis = 
      reinterpret_cast< WaitCursor * >( pData );
   assert( isGoodConstPtr( pThis ) );
   pThis->_threadFunc();
   return 0;
}


void WaitCursor::_finishThread( void ) {

   if ( 0 != _hEvent ) {
      // Signal event, if necessary, to wake the thread.
      // If the thread has finished already, no matter.
      verify( SetEvent( _hEvent ) );
   }

   if ( 0 != _hThread ) {
      verify( WAIT_OBJECT_0 == WaitForSingleObject( _hThread, INFINITE ) );
      verify( CloseHandle( _hThread ) );
      _hThread = 0;
   }

   if ( 0 != _hEvent ) {
      verify( CloseHandle( _hEvent ) );
      _hEvent  = 0;
   }
}


void WaitCursor::_restore( void ) const {

   // Actually, 0 is an acceptable value for SetCursor,
   // but _hcur is nevertheless not *supposed* to be 0 here:
   assert( 0 != _hcur );
   SetCursor( _hcur );
}


HCURSOR WaitCursor::_loadCursor( LPCTSTR pszName ) {

   if ( 0 != pszName ) {
      assert( isGoodStringPtr( pszName ) );

      String strCursor( CURSOR_PATH );
      strCursor += pszName;

      PATHNAME szWindowsDirectory = { 0 };
      const int nChars = GetWindowsDirectory( 
         szWindowsDirectory, dim( szWindowsDirectory ) );
      if ( 0 < nChars && nChars < dim( szWindowsDirectory ) ) {
         // We now have something like C:\\WINNT or C:\\WINDOWS. 
         // There's a terminating \\ if it is a root directory.
         PATHNAME szCursorPath = { 0 };
		 _tmakepath_s( szCursorPath, 0, szWindowsDirectory, strCursor.c_str() , 0 );
         
         // Sample szCursorPath = "C:\\WINNT\\Cursors\\load.ani"
         return loadCursor( szCursorPath );
      }
   }

   return 0;
}


/**
 * The constructor attempts to load the named cursor.
 * The pszName parameter should be relative to the Windows directory.
 */
WaitCursor::WaitCursor( LPCTSTR pszName, int nTimeIn )
   : _nTimeIn( nTimeIn )
   , _hThread( 0 )
   , _hEvent( 0 )
   , _dwParentThreadId( GetCurrentThreadId() )
   , _hcur( _loadCursor( pszName ) )
   , _isFromFile( 0 != _hcur ) 
{
   if ( !_isFromFile ) {
      _hcur = LoadCursor( 0, IDC_WAIT ); // NOTE: LR_SHARED is undoc!
   }
   if ( 0 < _nTimeIn ) {
      _hEvent = CreateEvent( 0, true, false, 0 );
      DWORD dwThreadId = 0; // Required under Win9x, not under NT.
      _hThread = CreateThread( 0, 0, _threadFunc, this, 0, &dwThreadId );
   }
   if ( 0 == _hThread ) {
      _restore();
   }
}


WaitCursor::~WaitCursor() throw() {

   _finishThread();
   assert( 0 != _hcur );
   if ( _isFromFile ) {
      //verify( DestroyCursor( _hcur ) ); // NOTE: This fails.
      DestroyCursor( _hcur );
   }
   reset_pointer( _hcur );

   POINT pt; // Screen coordinates.

   // SetCursorPos forces a WM_SETCURSOR message.
   if ( !GetCursorPos( &pt ) ) {
      trace( _T( "GetCursorPos failed!" ) );
      assert( false );
   } else if ( !SetCursorPos( pt.x, pt.y ) ) {
      trace( _T( "SetCursorPos( %d, %d ) failed!" ), pt.x, pt.y );
      assert( false );
   }
}


void WaitCursor::restore( void ) {

   _finishThread();
   _restore();
}

// end of file
