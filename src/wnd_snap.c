/*
 * $Header: /Book/wnd_snap.c 4     16.07.04 10:42 Oslph312 $
 *
 * Snap-window-to-edge subclassing.
 */

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
#include "wdjsub.h"
#include "wnd_snap.h"

#pragma warning( disable: 4100 ) // unref'd formal parameter

static LRESULT CALLBACK snapWndProc(
    HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );

#define HANDLE_WM_MOVING( hwnd, wParam, lParam, fn ) \
    (fn)( hwnd, (RECT *) lParam )
#define HANDLE_WM_ENTERSIZEMOVE( hwnd, wParam, lParam, fn ) \
    ((fn)(hwnd), 0L)

#ifndef shiftKeyPressed
#define shiftKeyPressed() (GetAsyncKeyState( VK_SHIFT ) < 0)
#endif

static SIZE offset;

static void onEnterSizeMove( HWND hwnd ) {

    RECT rcWindow = { 0 };
    GetWindowRect( hwnd, &rcWindow );

    GetCursorPos( (LPPOINT) &offset );
    
    offset.cx -= rcWindow.left;
    offset.cy -= rcWindow.top;
}

static BOOL isClose( int a, int b ) {

    NONCLIENTMETRICS ncm = { 0 };
    ncm.cbSize = sizeof ncm;
    SystemParametersInfo( SPI_GETNONCLIENTMETRICS, 0, &ncm, 0 );
    return abs( a - b ) < ncm.iCaptionHeight;
}

static BOOL onMoving( HWND hwnd, RECT *prc ) {

    POINT cur_pos = { 0 };
    RECT rcWorkArea = { 0 };

    if ( shiftKeyPressed() ) {
        return FALSE;
    }

    GetCursorPos( &cur_pos );
    OffsetRect( prc,
        cur_pos.x - (prc->left + offset.cx) ,
        cur_pos.y - (prc->top  + offset.cy) );

    SystemParametersInfo( SPI_GETWORKAREA, 0, &rcWorkArea, 0 );

    if ( isClose( prc->left, rcWorkArea.left ) ) {
        OffsetRect( prc, rcWorkArea.left - prc->left, 0 );
    } else if ( isClose( rcWorkArea.right, prc->right ) ) {
        OffsetRect( prc, rcWorkArea.right - prc->right, 0 );
    }
    if ( isClose( prc->top, rcWorkArea.top ) ) {
        OffsetRect( prc, 0, rcWorkArea.top - prc->top );
    } else if ( isClose( rcWorkArea.bottom, prc->bottom ) ) {
        OffsetRect( prc, 0, rcWorkArea.bottom - prc->bottom );
    }
    return TRUE;
}

static LRESULT CALLBACK snapWndProc(
    HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch ( msg ) {
    HANDLE_MSG( hwnd, WM_ENTERSIZEMOVE, onEnterSizeMove );
    HANDLE_MSG( hwnd, WM_MOVING       , onMoving        );
    }
    return wdjCallOldProc( snapWndProc, hwnd, msg, wParam, lParam );
}

BOOL WINAPI setSnap( HWND hwnd, BOOL enable ) {

    return enable ? wdjSubclass( snapWndProc, hwnd, 0 )
                  : wdjUnhook  ( snapWndProc, hwnd    );
}
