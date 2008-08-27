/*
 * $Header: /Book/menuUtils.h 7     3.07.99 17:46 Oslph312 $
 */

#pragma once

#include "String.h"

inline void enableMenuItem( HMENU hmenu, UINT item, bool bEnable ) {
   assert( ::IsMenu( hmenu ) );
   EnableMenuItem( hmenu, item, 
      MF_BYCOMMAND | ((bEnable) ? MF_ENABLED : MF_GRAYED) );
}


inline void checkMenuItem( HMENU hmenu, UINT item, bool bCheck ) {
   assert( ::IsMenu( hmenu ) );
   CheckMenuItem( hmenu, item, bCheck ? MF_CHECKED : MF_UNCHECKED );
}


inline void checkMenuRadioItem( 
   HMENU hmenu, UINT itemFirst, UINT itemLast, UINT itemCheck ) 
{
   assert( ::IsMenu( hmenu ) );
   CheckMenuRadioItem( 
      hmenu, itemFirst, itemLast, itemCheck, MF_BYCOMMAND );
}


bool containsItem( HMENU hmenu, UINT uiCmd, bool bRecursive = false );
void appendMenuItem( 
   HMENU hmenu, UINT item, LPCTSTR pszText, bool bDefault = false );
void appendSeparator( HMENU hmenu );

String getMenuItemText( HMENU hmenu, UINT uiCmd );
void __cdecl setMenuItemText( 
   HMENU hmenu, UINT uiCmd, LPCTSTR pszFmt, ... );

// end of file
