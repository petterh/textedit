/*
 * $Header: /Book/openDlgCommon.h 3     3.07.99 17:46 Oslph312 $
 */

#pragma once

void subclassOpenDlgCommon( HWND hwndChildDlg, UINT id );
bool getOpenFileName( HWND hwndParent, UINT uiTitleString,
   LPOFNHOOKPROC fnHook, LPTSTR pszFileName, UINT cb, 
   UINT uiChildDlg, DWORD dwFlags = 0 );
bool getSaveFileName(  HWND hwndParent, UINT uiTitleString,
   LPOFNHOOKPROC fnHook, LPTSTR pszFileName, UINT cb, 
   UINT uiChildDlg, DWORD dwFlags = 0 );

// end of file
