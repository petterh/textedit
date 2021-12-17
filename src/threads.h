/*
 * $Header: /Book/threads.h 4     3.07.99 17:46 Oslph312 $
 */

#pragma once


#include "Exception.h"


#ifndef _MT
   #error "No threads in single-threaded app"
#endif


typedef UINT (__stdcall *THREADFUNC) (void *);


inline HANDLE beginThread( THREADFUNC pfnThreadFunc, LPVOID pData ) 
   throw( WinException )
{
   UINT uiID = 0;
   const HANDLE hThread = reinterpret_cast< HANDLE >(
      _beginthreadex( 0, 0, pfnThreadFunc, pData, 0, &uiID ) );
   if ( 0 == hThread ) {
      throwException( _T( "Unable to start thread" ) );
   }
   return hThread;
}


inline void endThread( UINT uiRetCode ) {
   _endthreadex( uiRetCode );
}

// end of file
