/*
 * $Header: /Book/search.h 3     3.07.99 17:46 Oslph312 $
 */

#pragma once

LPCTSTR find( 
   LPCTSTR pszText, LPCTSTR pszStart, LPCTSTR pszSearch, 
   bool bMatchWholeWord, bool bMatchCase );
LPCTSTR findBackwards( 
   LPCTSTR pszText, LPCTSTR pszLast, LPCTSTR pszSearch, 
   bool bMatchWholeWord, bool bMatchCase );

// end of file
