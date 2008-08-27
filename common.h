/*
 * $Header: /Book/common.h 16    17.12.02 9:36 Oslph312 $
 * 
 * Common definitions.
 */

#pragma once


#ifndef SNDMSG
#define SNDMSG ::SendMessage
#endif


inline bool isGoodWritePtr( LPVOID addr, UINT ucb ) {
   return !IsBadWritePtr( addr, ucb );
}


inline bool isGoodReadPtr( LPCVOID addr, UINT ucb ) {
   return !IsBadReadPtr( addr, ucb );
}


inline bool isGoodStringPtr( LPCTSTR psz ) {
   return !IsBadStringPtr( psz, INT_MAX );
}


inline bool isGoodStringPtrOrAtom( LPCTSTR psz ) {
    return ( 
        0 == HIWORD( psz ) && 0 != LOWORD( psz ) ) ||
        isGoodStringPtr( psz );
}


// Use these with some care on arrays (such as strings)!
// I don't use templates, because VC gets into trouble with AutoPtr's
#define isGoodPtr( ptr )      isGoodWritePtr( ( ptr ), sizeof( *(ptr) ) )
#define isGoodConstPtr( ptr ) isGoodReadPtr ( ( ptr ), sizeof( *(ptr) ) )
#define isGoodCodePtr( func ) (!IsBadCodePtr( reinterpret_cast< FARPROC >( func ) ))
#define clearPtr( ptr )       memset( ( ptr ), 0, sizeof( *(ptr) ) )

#define dim( x ) (sizeof( x ) / sizeof( ( x )[ 0 ] ))

#ifdef _DEBUG
    #define verify( b )         assert( b )
    #define reset_pointer( p )  ((p) = 0)
    #undef  reset_pointer
    #define reset_pointer( p )  \
        ( memset( &( p ), 0xacACacAC, sizeof( p ) ), \
          assert( 4 == sizeof( p ) ) )
    #define unused( x ) ( x )
    #define debugBreak() DebugBreak()
#else
    #define verify( b )         ( b )
    #define reset_pointer( p )
    #define unused( x )
    #define debugBreak()
#endif

#define PRIVATE static

#if defined( _DEBUG ) && defined( _MSC_VER ) && (1200 <= _MSC_VER)
   #define _CRTDBG_MAP_ALLOC
   #define _MFC_OVERRIDES_NEW
   #include <crtdbg.h>
   _CRTIMP void * __cdecl operator new( 
      unsigned int, int, const char *, int );
#if 0
   inline void __cdecl operator delete( 
      void * _P, int, const char *, int ) 
   { 
      ::operator delete( _P ); 
   }
#endif
   #define new new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#endif

#if defined( _DEBUG ) 
   #if defined( _CRTDBG_REPORT_FLAG )
      #define isGoodHeapPtr( p ) \
         ( _CrtIsValidHeapPointer( p ) && isGoodConstPtr( p ) )
   #else
      #define isGoodHeapPtr isGoodPtr
   #endif
#else
   #define isGoodHeapPtr( p ) true
#endif


typedef WCHAR PATHNAMEW[ MAX_PATH + 1 ];
typedef CHAR  PATHNAMEA[ MAX_PATH + 1 ];

#ifdef UNICODE
   typedef PATHNAMEW PATHNAME;
#else
   typedef PATHNAMEA PATHNAME;
#endif


#define HANDLE_WM_COMMAND_ID( id, fn ) \
    case (id): ((fn)((hWnd), (id), (hWndCtrl), (nCode))); return 

// end of file
