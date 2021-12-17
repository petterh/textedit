/*
 * $Header: /Book/printFile.h 8     5.09.99 13:07 Oslph312 $
 */

#pragma once

#include "Document.h"
#include "Statusbar.h"

void printFile( Document *pDocument, 
   LPCTSTR pszPrinter, LPCTSTR pszDriver, LPCTSTR pszPort );

// end of file
