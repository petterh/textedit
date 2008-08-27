/*
 * $Header: /Book/FontDlg.cpp 12    28.11.99 22:18 Oslph312 $
 */

#include "precomp.h"
#include "Editor.h"
#include "resource.h"
#include "utils.h"
#include "winUtils.h"
#include "FontDlg.h"


UINT CALLBACK FontHookProc( 
   HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) 
{
   static bool s_wasPositioned = false;

   if ( WM_INITDIALOG == msg ) {
      const CHOOSEFONT *pChooseFont = 
         reinterpret_cast< const CHOOSEFONT * >( lParam );
      assert( isGoodConstPtr( pChooseFont ) );
      centerDialog( hwnd );
      
      const Rect *prcAvoid = 
         reinterpret_cast< const Rect * >( pChooseFont->lCustData );
      s_wasPositioned = 0 != prcAvoid;
      if ( s_wasPositioned ) {
         moveWindow( hwnd, prcAvoid );
         adjustToScreen( hwnd );
      } else {
         restorePosition( hwnd, IDD_FONT );
      }
      return TRUE;
   } else if ( WM_DESTROY == msg ) {
      if ( !s_wasPositioned ) {
         savePosition( hwnd, IDD_FONT );
      }
   }
   return 0;
}


bool selectFont( HWND hwndParent, LOGFONT *pLogFont, 
   const Rect *prcAvoid, DWORD dwExtraFlags )
{
   const DWORD chooseFontFlags = 
      CF_SCREENFONTS | CF_ENABLEHOOK | CF_ENABLETEMPLATE | 
      CF_INITTOLOGFONTSTRUCT | CF_FORCEFONTEXIST;

   CHOOSEFONT chooseFont = {
      sizeof( CHOOSEFONT ), hwndParent, 0, pLogFont, 0,
      chooseFontFlags | dwExtraFlags,
   };
   chooseFont.lpfnHook = FontHookProc;
   chooseFont.lpTemplateName = MAKEINTRESOURCE( IDD_FONT );
   chooseFont.hInstance = getModuleHandle();
   chooseFont.lCustData = reinterpret_cast< LPARAM >( prcAvoid );
   return 0 != ChooseFont( &chooseFont );
}

// end of file
