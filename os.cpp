/*
 * $Header: /Book/os.cpp 14    16.07.04 10:42 Oslph312 $
 * 
 * Implements operating system-specific functions and functions to 
 * figure out which system we're running (Win9x or WinNT).
 * NOTE: The function name argument to GetProcAddress is NOT Unicode.
 * NOTE: Neither GetProcAddress nor FreeLibrary mind a null hmodule,
 * though tools such as BoundsChecker may hickup.
 */


#include "precomp.h"
#include "os.h"
#include <objbase.h>
#include "trace.h"
#include "AutoLibrary.h"


bool isWindowsNT( void ) {

   OSVERSIONINFO osvi = { sizeof osvi };
   assert( 0 == offsetof( OSVERSIONINFO, dwOSVersionInfoSize ) );
   return GetVersionEx( &osvi ) && 
      VER_PLATFORM_WIN32_NT == osvi.dwPlatformId;
}


DWORD getGoodIOBufferSize( void ) {

   SYSTEM_INFO si = { 0 };
   GetSystemInfo( &si );
   if ( 0 == si.dwAllocationGranularity ) {
      si.dwAllocationGranularity = 4096;
   }
   if ( 65536 < si.dwAllocationGranularity ) {
      si.dwAllocationGranularity = 65536;
   }
   return si.dwAllocationGranularity;
}


// Function types for dynamic linking:

typedef HRESULT (STDAPICALLTYPE* COINITIALIZEEX)(
   LPVOID pvReserved, DWORD dwCoInit);
typedef HRESULT (STDAPICALLTYPE* COINITIALIZE)(LPVOID pvReserved);
typedef DWORD (WINAPI *GETCOMPRESSEDFILESIZE)(
   LPCTSTR lpFileName, LPDWORD lpFileSizeHigh );
typedef BOOL (WINAPI *SETTHREADLOCALE)( LCID locale );
typedef BOOL (WINAPI *ISTEXTUNICODE)( 
   CONST LPVOID lpBuffer, int cb, LPINT lpi );
typedef BOOL (WINAPI *ISDEBUGGERPRESENT)( void );


/**
 * Early versions of Win95 do not support CoInitializeEx.
 * Windows CE does not support CoInitialize. 
 */
HRESULT coInitialize( void ) {

   HRESULT hres = E_UNEXPECTED;
   AutoLibrary hmodule( _T( "OLE32" ) );
   assert( 0 != (HMODULE) hmodule );

   COINITIALIZEEX fnCoInitializeEx = 
      (COINITIALIZEEX) GetProcAddress( hmodule, "CoInitializeEx" );
   if ( 0 != fnCoInitializeEx ) {
      assert( isGoodCodePtr( fnCoInitializeEx ) );
      hres = fnCoInitializeEx( 0, COINIT_APARTMENTTHREADED );
   }
   if ( !SUCCEEDED( hres ) ) {
      COINITIALIZE fnCoInitialize = 
         (COINITIALIZE) GetProcAddress( hmodule, "CoInitialize" );
      if ( 0 != fnCoInitialize ) {
         assert( isGoodCodePtr( fnCoInitialize ) );
         hres = fnCoInitialize( 0 );
      }
   }

   return hres;
}


#ifdef UNICODE
#define FN_NAME "GetCompressedFileSizeW"
#else
#define FN_NAME "GetCompressedFileSizeA"
#endif

bool getCompressedFileSize( LPCTSTR pszFile, DWORD *pdwSize ) {

   assert( isGoodStringPtr( pszFile ) );
   assert( isGoodPtr( pdwSize ) );

   bool bOK = false;
   AutoLibrary hmodule( _T( "KERNEL32" ) );
   assert( 0 != (HMODULE) hmodule );

   GETCOMPRESSEDFILESIZE fnGetCompressedFileSize = 
      (GETCOMPRESSEDFILESIZE) GetProcAddress( hmodule, FN_NAME );
   bOK = 0 != fnGetCompressedFileSize;
   if ( bOK ) {
      assert( isGoodCodePtr( fnGetCompressedFileSize ) );
      *pdwSize = fnGetCompressedFileSize( pszFile, 0 );
   }

   return bOK;
}


#undef FN_NAME

// LOCALE_USER_DEFAULT, LOCALE_SYSTEM_DEFAULT
bool setThreadLocale( LCID locale ) {

#if 1 // TODO
    bool bOK = 0 != SetThreadLocale( locale );
#ifdef _DEBUG
    const LCID real_locale = GetThreadLocale();
    assert( real_locale == locale );
#endif
#else
   bool bOK = false;
   AutoLibrary hmodule( _T( "KERNEL32" ) );
   assert( 0 != (HMODULE) hmodule );

   SETTHREADLOCALE fnSetThreadLocale = 
      (SETTHREADLOCALE) GetProcAddress( hmodule, "SetThreadLocale" );
   bOK = 0 != fnSetThreadLocale;
   if ( bOK ) {
      assert( isGoodCodePtr( fnSetThreadLocale ) );
      bOK = 0 != fnSetThreadLocale( locale );
   }
#endif

   return bOK;
}


bool isTextUnicode( CONST LPVOID lpBuffer, int cb, LPINT lpi ) {

   if ( 0 == cb ) {
      return false; // LATER: getDefaultUnicodeSetting.
   }

   AutoLibrary hmodule( _T( "ADVAPI32" ) );
   ISTEXTUNICODE fnIsTextUnicode = 
      (ISTEXTUNICODE) GetProcAddress( hmodule, "IsTextUnicode" );
   bool bOK = 0 != fnIsTextUnicode;
   if ( bOK ) {
      assert( isGoodCodePtr( fnIsTextUnicode ) );
      bOK = 0 != fnIsTextUnicode( lpBuffer, cb, lpi );
   }

   return bOK;
}


#ifdef _DEBUG

bool isDebuggerPresent( void ) {
   
   bool bDebuggerPresent = false;
   
   AutoLibrary hmodule( _T( "KERNEL32" ) );
   assert( 0 != (HMODULE) hmodule );

   ISDEBUGGERPRESENT fnIsDebuggerPresent = 
      GetProcAddress( hmodule, "IsDebuggerPresent" ); // NOT Unicode!
   if ( 0 != fnIsDebuggerPresent ) {
      assert( isGoodCodePtr( fnIsDebuggerPresent ) );
      bDebuggerPresent = 0 != fnIsDebuggerPresent();
   }

   return bDebuggerPresent;
}

#else

#define isDebuggerPresent() false

#endif // _DEBUG

// end of file
