/*
 * $Header: /Book/themes.c 5     16.07.04 10:42 Oslph312 $
 */

#pragma warning( disable: 4514 ) // Unreferenced inline function has been removed
#pragma warning( push, 3 )

#ifdef UNICODE
#undef _UNICODE
#define _UNICODE
#endif

#ifdef _UNICODE
#undef  UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <uxtheme.h>
#include <tchar.h>
#include <winerror.h>

#pragma warning( pop )

#include "themes.h"


// Function types for dynamic linking:

typedef DWORD (STDAPICALLTYPE* ISAPPTHEMED)( void );
typedef HTHEME (STDAPICALLTYPE* OPENTHEMEDATA)( HWND, LPCWSTR );
typedef HRESULT (STDAPICALLTYPE* CLOSETHEMEDATA)( HTHEME );
typedef HRESULT (STDAPICALLTYPE* DRAWTHEMEBACKGROUND)(
    MYHTHEME, HDC, int, int, const RECT *, const RECT * );
typedef HRESULT (STDAPICALLTYPE* GETTHEMECOLOR)(
    HTHEME, int, int, int, COLORREF * );

static BOOL isAppThemed( void ) {

    DWORD is_themed = FALSE;
    HMODULE hmodule = LoadLibrary( _T( "UXTHEME" ) );
    if ( 0 != hmodule ) {
        ISAPPTHEMED fnIsAppThemed =
            (ISAPPTHEMED) GetProcAddress( hmodule, "GetThemeAppProperties" );
        if ( 0 != fnIsAppThemed ) {
            is_themed = fnIsAppThemed(); // STAP_ALLOW_NONCLIENT
        }
        FreeLibrary( hmodule );
    }
    return is_themed;
}


// OS-version-safe wrappers for theme functions:

MYHTHEME STDAPICALLTYPE openThemeData( HWND hwnd, LPCWSTR pszClassList ) {

    MYHTHEME ht = 0;
    if ( isAppThemed() ) {
        HMODULE hmodule = LoadLibrary( _T( "UXTHEME" ) );
        if ( 0 != hmodule ) {
            OPENTHEMEDATA fnOpenThemeData =
                (OPENTHEMEDATA) GetProcAddress( hmodule, "OpenThemeData" );
            if ( 0 != fnOpenThemeData ) {
                ht = (MYHTHEME) fnOpenThemeData( hwnd, pszClassList );
            }
            FreeLibrary( hmodule );
        }
    }
    return ht;
}


HRESULT STDAPICALLTYPE drawThemeBackground(
    MYHTHEME ht, HDC hdc, int iPartId, int iStateId,
    const RECT *pRect, OPTIONAL const RECT *pClipRect )
{
    HRESULT hres = E_NOTIMPL;
    HMODULE hmodule = LoadLibrary( _T( "UXTHEME" ) );
    if ( 0 != hmodule ) {
        DRAWTHEMEBACKGROUND fnDrawThemeBackground =
            (DRAWTHEMEBACKGROUND) GetProcAddress( hmodule, "DrawThemeBackground" );
        if ( 0 != fnDrawThemeBackground ) {
            hres = fnDrawThemeBackground(
                ht, hdc, iPartId, iStateId, pRect, pClipRect );
        }
        FreeLibrary( hmodule );
    }
    return hres;
}


HRESULT STDAPICALLTYPE getThemeColor(
    HTHEME ht, int iPartId, int iStateId, int iPropId, OUT COLORREF *pColor )
{
    HRESULT hres = E_NOTIMPL;
    HMODULE hmodule = LoadLibrary( _T( "UXTHEME" ) );
    if ( 0 != hmodule ) { // TODO AutoLibrary in C++ version
        GETTHEMECOLOR fnGetThemeColor =
            (GETTHEMECOLOR) GetProcAddress( hmodule, "GetThemeColor" );
        if ( 0 != fnGetThemeColor ) {
            hres = fnGetThemeColor( ht, iPartId, iStateId, iPropId, pColor );
        }
        FreeLibrary( hmodule );
    }
    return hres;
}


HRESULT STDAPICALLTYPE closeThemeData( MYHTHEME ht ) {

    HRESULT hres = E_NOTIMPL;
    HMODULE hmodule = LoadLibrary( _T( "UXTHEME" ) );
    if ( 0 != hmodule ) {
        CLOSETHEMEDATA fnCloseThemeData =
            (CLOSETHEMEDATA) GetProcAddress( hmodule, "CloseThemeData" );
        if ( 0 != fnCloseThemeData ) {
            hres = fnCloseThemeData( ht );
        }
        FreeLibrary( hmodule );
    }
    return hres;
}

// end of file
