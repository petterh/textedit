/*
 * $Header: /Book/openFileDlg.h 2     14.10.00 7:37 Oslph312 $
 */

#pragma once

#include "String.h"

bool openFileDlg( 
   const HWND hwndParent, LPTSTR pszName, int cb, 
   bool *pbNewWindow = 0,
   bool bMustExist = true );

// end of file
