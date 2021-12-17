/*
 * $Header: /Book/FileNotFoundDlg.h 6     7.06.99 0:37 Oslph312 $
 */

#pragma once

#include "dialog.h"

/**
 * Called when a file is not found.
 * The file name may be modified.
 */
HANDLE getNewFile( HWND hwnd, String *pstrFile );

// end of file
