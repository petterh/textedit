/*
 * $Header: /Book/FontDlg.h 6     5.09.99 13:07 Oslph312 $
 */

#pragma once

#include "geometry.h"


bool selectFont( HWND hwndParent, LOGFONT *pLogFont, 
   const Rect *prcAvoid = 0, DWORD dwExtraFlags = 0 );

// end of file
