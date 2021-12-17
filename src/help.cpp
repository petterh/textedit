/*
 * $Header: /Book/help.cpp 10    5.03.02 10:03 Oslph312 $
 */

#include "precomp.h"
#include "help.h"
#include "resource.h"
#include "utils.h"
#include "menuUtils.h"


LPCTSTR getHelpFile( void ) {
   
   static PATHNAME szHelpFile = { 0 };
   
   if ( 0 == szHelpFile[ 0 ] ) {
      String strModule = getModuleFileName();
      const int nLastDelimiter = strModule.find_last_of( _T( '\\' ) );
      if ( 0 < nLastDelimiter ) {
         strModule = strModule.substr( 0, nLastDelimiter );
      } else {
         strModule.erase();
      }
      wsprintf( szHelpFile, _T( "%s\\%s" ), 
         strModule.c_str(), _T( "TextEdit.hlp>proc" ) );
   }

   return szHelpFile;
}


PRIVATE inline bool setHelpContextMenuID( 
   HWND hwnd, HELPINFO *pHelpInfo ) 
{
#ifdef IDS_FILE_MENU

   static const int anMenuHelpIDs[] = {
      IDS_FILE_MENU, IDS_EDIT_MENU, IDS_FIND_MENU, 
      IDS_VIEW_MENU, IDS_HELP_MENU,
   };

   const HMENU hmenu = GetMenu( hwnd );
   for ( int iItem = 0; iItem < dim( anMenuHelpIDs ); ++iItem ) {
      const HMENU hmenuSub = GetSubMenu( hmenu, iItem );
      if ( containsItem( hmenuSub, pHelpInfo->iCtrlId ) ) {
         pHelpInfo->dwContextId = anMenuHelpIDs[ iItem ];
         return true; //*** FUNCTION EXIT POINT
      }
   }

#endif // IDS_FILE_MENU

   return false;
}


static DWORD s_dwHelpID = 0;

void setHelpID( DWORD dwID ) {
   s_dwHelpID = dwID;
}


void help( HWND hwnd, HELPINFO *pHelpInfo ) {

   assert( 0 != pHelpInfo );
   UINT uiCommand = HELP_CONTEXTPOPUP;
   if ( HELPINFO_MENUITEM == pHelpInfo->iContextType ) {
      uiCommand = HELP_CONTEXT;
   }
   uiCommand = HELP_CONTEXT;

   if ( !setHelpContextMenuID( hwnd, pHelpInfo ) ) {
      pHelpInfo->dwContextId = s_dwHelpID;
   }

   if ( 0 == pHelpInfo->dwContextId ) {
      uiCommand = HELP_FINDER;
   }

   WinHelp( hwnd, getHelpFile(), uiCommand, pHelpInfo->dwContextId );
}

// end of file
