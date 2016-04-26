/*
 * $Header: /Book/utils.h 15    16.07.04 10:42 Oslph312 $
 */

#pragma once

#include "String.h"

#ifdef _WINDOWS
#include "MenuFont.h"
#endif // _WINDOWS

inline HMODULE getModuleHandle( void ) {

   return GetModuleHandle( 0 );
}


String getModuleFileName( void );

#ifdef _DEBUG
#define comFree( p ) (comFreeImpl( p ), reset_pointer( p ))
#else
#define comFree      comFreeImpl
#endif

void comFreeImpl( LPVOID pObject );

String getPathFromIDList( ITEMIDLIST *pidl );

String loadString( UINT uiID );
String loadToolTip( UINT uiID );

String __cdecl loadMenuDescription( UINT uiID, ... );

#ifdef _WINDOWS
SIZE measureString( LPCTSTR psz, HFONT hfont = MenuFont::getFont() );
int devPointsFromPrinterPoints( int nHeight );
int printerPointsFromDevPoints( int nHeight );
int devPointsFromPrinterPoints( int nHeight, HDC hdc );
int printerPointsFromDevPoints( int nHeight, HDC hdc );
#endif // _WINDOWS

void multiByteToWideChar( LPCSTR pszIn, LPWSTR pwszOut, int nChars );

// Returns true if the default character was used:
bool wideCharToMultiByte( 
   LPCWSTR pwszIn, LPSTR pszOut, int nChars, CHAR chDefault = 0 );


inline void multiByteToWideChar( LPCSTR pszIn, LPWSTR pwszOut ) {
   const int nChars = strlen( pszIn );
   multiByteToWideChar( pszIn, pwszOut, nChars + 1 );
}


inline bool wideCharToMultiByte( LPCWSTR pwszIn, LPSTR pszOut ) {
   const int nChars = wcslen( pwszIn );
   return wideCharToMultiByte( pwszIn, pszOut, nChars + 1, '_' );
}


inline LPTSTR charNext( LPCTSTR psz ) {

   return CharNext( psz );
}


inline LPTSTR charPrev( LPCTSTR pszStart, LPCTSTR pszCurr ) {

   return CharPrev( pszStart, pszCurr );
}


int random( int nCeiling = RAND_MAX + 1 );
void doubleAmpersands( String *pstr );
void replace( String *pstr, LPCTSTR pszPattern, LPCTSTR pszReplacement = _T( "" ) );

#ifndef shiftKeyPressed
#define shiftKeyPressed() (GetAsyncKeyState( VK_SHIFT ) < 0)
#endif

// end of file
