/*
 * $Header: /FAST/pspscan.root/pspscan/AutoHandle.h 10    9.02.05 17:34 Oslph312 $
 *
 * HINT: To see the handle wrapped by AutoHandle during Visual C++
 * debugging, add the following line to the AUTOEXP.DAT file:
 * AutoHandle<*>=<_handle>
 *
 * See:
 * C:\Program Files\Microsoft Visual Studio\Common\MSDev98\Bin\AUTOEXP.DAT
 */

#pragma once


class AutoHandle {
private:
   HANDLE _handle;

public:
   explicit AutoHandle( HANDLE handle = 0 );
   ~AutoHandle();
   HANDLE operator=( HANDLE handle ); // TODO -- is this ever used? And should we ref count?
   operator HANDLE();
};


inline AutoHandle::AutoHandle( HANDLE handle ) : _handle( handle ) {
}


inline AutoHandle::~AutoHandle() {
   if ( 0 != _handle && INVALID_HANDLE_VALUE != _handle ) {
      verify( CloseHandle( _handle ) );
   }
}

inline HANDLE AutoHandle::operator=( HANDLE handle ) {

#ifdef _DEBUG
   if ( INVALID_HANDLE_VALUE != _handle && 0 != _handle ) {
#ifdef trace
      trace( _T( "Suspect AutoHandle assignment -- will this ever be closed?" ) );
#endif
#ifdef debugBreak
      debugBreak();
#endif
   }
#endif

   _handle = handle;
   return _handle;
}

inline AutoHandle::operator HANDLE() {
   return _handle;
}

// end of file
