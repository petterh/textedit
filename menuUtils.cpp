/*
 * $Header: /Book/menuUtils.cpp 10    3.07.99 17:46 Oslph312 $
 */

#include "precomp.h"
#include "menuUtils.h"
#include "formatMessage.h"
#include "String.h"

/**
 * The recursive case is depth-first.
 */ 
bool containsItem( HMENU hmenu, UINT uiCmd, bool bRecursive ) {

   assert( IsMenu( hmenu ) );
   const int nItems = GetMenuItemCount( hmenu );
   for ( int iItem = 0; iItem < nItems; ++iItem ) {
      HMENU hmenuSub = GetSubMenu( hmenu, iItem );
      if ( IsMenu( hmenuSub ) ) {
         return bRecursive 
            ? containsItem( hmenuSub, uiCmd, true ) : false;
      } else if ( GetMenuItemID( hmenu, iItem ) == uiCmd ) {
         return true;
      }
   }
   return false;
}


String getMenuItemText( HMENU hmenu, UINT uiCmd ) {

   TCHAR szMenuItem[ 100 ] = { 0 };
   GetMenuString( 
      hmenu, uiCmd, szMenuItem, dim( szMenuItem ), MF_BYCOMMAND );
   return szMenuItem;
}


/**
 * This avoids the error-prone ID repetition required by ModifyMenu.
 */
void __cdecl setMenuItemText( 
   HMENU hmenu, UINT uiCmd, LPCTSTR pszFmt, ... ) 
{
   va_list vl;
   va_start( vl, pszFmt );
   String strMenuText( formatMessageV( pszFmt, vl ) );
   va_end( vl );

   ModifyMenu( 
      hmenu, uiCmd, MF_BYCOMMAND, uiCmd, strMenuText.c_str() );
}


void appendMenuItem( 
   HMENU hmenu, UINT item, LPCTSTR pszText, bool bDefault ) 
{
   assert( 0 == offsetof( MENUITEMINFO, cbSize ) );
   MENUITEMINFO menuItemInfo = {
      sizeof( MENUITEMINFO ), MIIM_ID | MIIM_TYPE | MIIM_STATE,
      MFT_STRING, MFS_ENABLED, item, 0, 0, 0, 0,
      const_cast< LPTSTR >( pszText ), 
      sizeof( TCHAR ) * _tcsclen( pszText ),
   };
   if ( bDefault ) {
      menuItemInfo.fState |= MFS_DEFAULT;
   }
   verify( InsertMenuItem( hmenu, item, false, &menuItemInfo ) );
}


void appendSeparator( HMENU hmenu ) {

   assert( 0 == offsetof( MENUITEMINFO, cbSize ) );
   MENUITEMINFO menuItemInfo = {
      sizeof( MENUITEMINFO ), MIIM_TYPE,
         MFT_SEPARATOR, 0, 0, 0, 0, 0, 0, 0, 0 
   };
   verify( InsertMenuItem( hmenu, (UINT) -1, false, &menuItemInfo ) );
}

// end of file
