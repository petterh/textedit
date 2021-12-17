/*
 * $Header: /Book/combobug.c 2     5.03.02 10:00 Oslph312 $
 *
 * This code was written by Petter Hesselberg and published as part
 * of the User Interface Programming column in Windows Developer's
 * Journal. There are no restrictions on the use of this code,
 * nor are there any guarantees for fitness of purpose or anything
 * else.  Smile and carry your own risk :-)
 *
 * Changelog:
 *
 * Jan 2002     UNICODE/_UNICODE synchronization.
 */

#define WIN32_LEAN_AND_MEAN

#ifdef UNICODE
#undef _UNICODE
#define _UNICODE
#endif

#ifdef _UNICODE
#undef  UNICODE
#define UNICODE
#endif


#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <stdlib.h>
#include "combobug.h"

#define dim( x ) (sizeof( x ) / sizeof( ( x )[ 0 ] ))


static WNDPROC s_wpOldDlg  = 0;
static WNDPROC s_wpOldEdit = 0;

static BOOL s_skipSetText = FALSE;


static BOOL isClass( HWND hwnd, LPCTSTR pszClass ) {

    TCHAR szClassName[ 100 ] = { 0 };
    GetClassName( hwnd, szClassName, dim( szClassName ) );
    return 0 == _tcsicmp( szClassName, pszClass );
}


static BOOL isDropDownCombo( HWND hwnd ) {

    if ( isClass( hwnd, _T( "ComboBox" ) ) ) {
        return 0 != (CBS_DROPDOWN & GetWindowStyle( hwnd ));
    }
    return FALSE;
}


static LRESULT CALLBACK myEditWndProc(
    HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    if ( WM_SETTEXT == msg && s_skipSetText && 
        isDropDownCombo( GetParent( hwnd ) ) )
    {
        LPCTSTR psz = (LPCTSTR) lParam;
        if ( 0 != psz && 0 == *psz ) {
            s_skipSetText = FALSE;
            return 0; //*** FUNCTION EXIT POINT
        }
    }
    return CallWindowProc( s_wpOldEdit, hwnd, msg, wParam, lParam );
}


static LRESULT CALLBACK myDlgWndProc(
    HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    const int  codeNotify = GET_WM_COMMAND_CMD ( wParam, lParam );
    const HWND hwndCtl    = GET_WM_COMMAND_HWND( wParam, lParam );
    
    s_skipSetText = WM_COMMAND == msg && 
        CBN_SELCHANGE == codeNotify && 
        isDropDownCombo( hwndCtl ) &&
        -1 == ComboBox_GetCurSel( hwndCtl );

    return CallWindowProc( s_wpOldDlg, hwnd, msg, wParam, lParam );
}


static void __cdecl unFixComboBug( void ) {

    if ( 0 != s_wpOldDlg ) {
        HWND hwnd = CreateWindow( WC_DIALOG, _T( "" ), WS_POPUP,
            0, 0, 0, 0, HWND_DESKTOP, 0, GetModuleHandle( 0 ), 0 );
        SetClassLong( hwnd, GCL_WNDPROC, (LONG) s_wpOldDlg );
        DestroyWindow( hwnd );
    }
    if ( 0 != s_wpOldEdit ) {
        HWND hwnd = CreateWindow( 
            _T( "Edit" ), _T( "" ), WS_POPUP,
            0, 0, 0, 0, HWND_DESKTOP, 0, GetModuleHandle( 0 ), 0 );
        SetClassLong( hwnd, GCL_WNDPROC, (LONG) s_wpOldEdit );
        DestroyWindow( hwnd );
    }
}


void WINAPI fixComboBug( void ) { // TODO -- reuse setClassLong()

    HWND hwnd = CreateWindow( WC_DIALOG, _T( "" ), WS_POPUP, 
        0, 0, 0, 0, HWND_DESKTOP, 0, GetModuleHandle( 0 ), 0 );
    if ( IsWindow( hwnd ) ) {
        s_wpOldDlg = (WNDPROC) SetClassLong(
            hwnd, GCL_WNDPROC, (LONG) myDlgWndProc );
        DestroyWindow( hwnd );
    }
    hwnd = CreateWindow( _T( "Edit" ), _T( "" ), WS_POPUP, 
        0, 0, 0, 0, HWND_DESKTOP, 0, GetModuleHandle( 0 ), 0 );
    if ( IsWindow( hwnd ) ) {
        s_wpOldEdit = (WNDPROC) SetClassLong(
            hwnd, GCL_WNDPROC, (LONG) myEditWndProc );
        DestroyWindow( hwnd );
    }
    
    atexit( unFixComboBug );
}

// end of file
