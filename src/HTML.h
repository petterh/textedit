/*
 * $Header: /Cleaner/HTML.h 12    2.04.02 13:53 Oslph312 $
 * 
 */

#pragma once

#include "winUtils.h"
#include "Exception.h"


void subclassHTML( HWND hwnd ) throw( SubclassException );


#define PHTML_MULTI_LINE  0x0
#define PHTML_SINGLE_LINE 0x1
#define PHTML_NO_TOKENS   0x2

void paintHTML( HDC hdc, LPCTSTR pszString, RECT *pRect, HFONT hfont,
   DWORD flags = PHTML_MULTI_LINE, bool bDisabled = false );


#define HIGHLIGHT_PROP _T( "Highlight" )

inline int getHighlight( HWND hwnd ) {
   
   assert( IsWindow( hwnd ) );
   assert( isClass( hwnd, _T( "static" ) ) || isClass( hwnd, _T( "mystatic" ) ) );
   return (int) GetProp( hwnd, HIGHLIGHT_PROP );
}


inline bool hasHighlight( HWND hwnd ) {
   
   return 0 != getHighlight( hwnd );
}


inline void removeHighlight( HWND hwnd ) {

   assert( IsWindow( hwnd ) );
   assert( isClass( hwnd, _T( "static" ) ) || isClass( hwnd, _T( "mystatic" ) ) );
   if ( hasHighlight( hwnd ) ) {
      RemoveProp( hwnd, HIGHLIGHT_PROP );
      InvalidateRect( hwnd, 0, TRUE );
   }
}


inline void setHighlight( HWND hwnd, int highlight = 1 ) {
   
   assert( IsWindow( hwnd ) );
   removeHighlight( hwnd );
   SetProp( hwnd, HIGHLIGHT_PROP, (HANDLE) highlight );
   InvalidateRect( hwnd, 0, TRUE );
}


inline void setHighlight( HWND hwnd, bool bHighlight ) {

   if ( bHighlight ) {
      setHighlight( hwnd );
   } else {
      setHighlight( hwnd, 2 );
      //removeHighlight( hwnd );
   }
}

#undef HIGHLIGHT_PROP

// end of file
