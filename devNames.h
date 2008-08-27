/*
 * $Header: /Book/devNames.h 5     5.09.99 13:07 Oslph312 $
 */

#pragma once

HGLOBAL getDevNames(
   LPCTSTR pszPrinter = 0, 
   LPCTSTR pszDriver  = 0, 
   LPCTSTR pszPort    = 0 );
void setDevNames( HGLOBAL hDevNames );

// end of file
