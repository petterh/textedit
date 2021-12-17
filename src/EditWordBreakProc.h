/*
 * $Header: /Book/EditWordBreakProc.h 1     16.07.04 10:42 Oslph312 $
 */

#pragma once

extern void installEditWordBreakProcA( HWND hwnd );
extern void installEditWordBreakProcW( HWND hwnd );

#ifdef _UNICODE
#define installEditWordBreakProc installEditWordBreakProcW
#else
#define installEditWordBreakProc installEditWordBreakProcA
#endif