/*
 * $Header: /Book/HTML.cpp 27    16.07.04 10:42 Oslph312 $
 * 
 * Subclassing of class "static" to display simple rich text.
 */

#include "precomp.h"
#include "Window.h"
#include "AutoArray.h"
#include "ClientDC.h"
#include "PaintStruct.h"
#include "HTML.h"
#include "InstanceSubclasser.h"
#include "fileUtils.h"
#include "graphics.h"


#define DT_COMMON_PARMS DT_TOP | DT_LEFT | DT_SINGLELINE | DT_EXTERNALLEADING
#ifdef HTML_SHOW_PREFIX
#define DT_PARMS DT_COMMON_PARMS
#else
#define DT_PARMS DT_COMMON_PARMS | DT_NOPREFIX
#endif

PRIVATE LRESULT CALLBACK Simple_HTML_WndProc( 
   HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );


class HTML_Subclasser : public InstanceSubclasser {
public:
   explicit HTML_Subclasser( WNDPROC wndProc )
      : InstanceSubclasser( wndProc )
   {
   }

   bool subclass( HWND hwnd, String *pUserData ) {
      return InstanceSubclasser::subclass( hwnd, pUserData );
   }

   virtual bool unSubclass( HWND hwnd ) {
      String *pString = getUserData( hwnd );
      delete pString;
      return InstanceSubclasser::unSubclass( hwnd );
   }

   String *getUserData( HWND hwnd ) const {
      return (String *) InstanceSubclasser::getUserData( hwnd );
   }

   LRESULT callOldProc( 
      HWND hwnd, UINT uiMsg, WPARAM wParam, LPARAM lParam )
   {
      return InstanceSubclasser::callOldProc( hwnd, uiMsg, wParam, lParam );
   }
};


PRIVATE HTML_Subclasser s_HTML_Subclasser( Simple_HTML_WndProc );


PRIVATE void trim( String &str ) {

   while ( _istspace( str[ 0 ] ) ) { 
      str.erase( 0, 1 );
   }
}


PRIVATE bool getToken( 
   String &str, bool &bPara, bool &bBold, String &strWord ) 
{
   if ( str.empty() ) {
      return false; //*** FUNCTION EXIT POINT
   }

   strWord = _T( "" );

   const int iSpace   = str.find_first_of( _T( " \t\r\n" ) );
   const int iPara    = str.find( _T( "<p>" ) );
   const int iBold    = str.find( _T( "<b>" ) );
   const int iEndBold = str.find( _T( "</b>" ) );

   bPara = ( 0 == iPara );
   if ( 0 == iSpace ) {
      strWord = _T( " " );
      trim( str );
      bPara = 0 == str.find( _T( "<p>" ) );
      if ( bPara ) {
         str = str.substr( 3 );
         trim( str );
      }
      return true;
   }

   if ( bPara ) {
      str = str.substr( 3 );
      trim( str );
   } else if ( 0 == iBold ) {
      bBold = true;
      str = str.substr( 3 );
      //trim( str );
   } else if ( 0 == iEndBold ) {
      bBold = false;
      str = str.substr( 4 );
      //trim( str );
   } else {
      int iNext = str.length();
      if ( 0 < iSpace && iSpace < iNext ) {
         iNext = iSpace;
      }
      if ( 0 < iPara && iPara < iNext ) {
         iNext = iPara;
      }
      if ( 0 < iBold && iBold < iNext ) {
         iNext = iBold;
      }
      if ( 0 < iEndBold && iEndBold < iNext ) {
         iNext = iEndBold;
      }
      
      if ( iNext < 0 ) {
         strWord = str;
         str.erase();
         trim( strWord );
      } else {
         strWord = str.substr( 0, iNext );
         str = str.substr( iNext );
      }
   }
   
   //trim( str );

   return true;
}

PRIVATE inline void textOut( HDC hdc, int x, int y, LPCTSTR psz ) {

   RECT rc = { x, y, x + 1000, y + 1000 };
   verify( DrawText( hdc, psz, _tcslen( psz ), &rc, DT_PARMS ) );
}


PRIVATE void textOut( HDC hdc, LPCTSTR psz, bool bDisabled ) {

   if ( bDisabled ) {
      const COLORREF crText = GetSysColor( COLOR_3DHIGHLIGHT );
      const COLORREF crSaved = SetTextColor( hdc, crText );
      POINT ptCurrPos;
      verify( GetCurrentPositionEx( hdc, &ptCurrPos ) );
      const int nTextAlign = GetTextAlign( hdc );
      verify( GDI_ERROR != 
         SetTextAlign( hdc, nTextAlign & ~TA_UPDATECP ) );
      textOut( hdc, ptCurrPos.x + 1, ptCurrPos.y + 1, psz );
      verify( MoveToEx( hdc, ptCurrPos.x, ptCurrPos.y, 0 ) );
      verify( GDI_ERROR != SetTextAlign( hdc, nTextAlign ) );
      SetTextColor( hdc, crSaved );
   }
   textOut( hdc, 0, 0, psz );
}


PRIVATE void doPaint( HDC hdc, LPCTSTR pszString, const RECT &rc, 
   HFONT hfontNormal, HFONT hfontBold, DWORD dwFlags, bool bDisabled )
{
   if ( bDisabled ) {
      SetBkMode( hdc, TRANSPARENT );
   }

   verify( MoveToEx( hdc, rc.left, rc.top, 0 ) );

   SIZE sizeText;
   verify( 
      GetTextExtentPoint32( hdc, _T( "Åy" ), 2, &sizeText ) );
   const int nLineHeight = sizeText.cy;

   bool bPara = false;
   bool bBold = false;
   String str( pszString );
   while ( !str.empty() ) {
      POINT ptCurrPos;
      verify( GetCurrentPositionEx( hdc, &ptCurrPos ) );

      String strWord;
      if ( !getToken( str, bPara, bBold, strWord ) ) {
         assert( str.empty() );
         break; //*** LOOP EXIT POINT
      }

      if ( bPara ) {
         if ( PHTML_SINGLE_LINE & dwFlags ) {
            textOut( hdc, _T( " " ), bDisabled );
         } else {
            MoveToEx( hdc, rc.left, ptCurrPos.y + 3 * nLineHeight / 2, 0 );
         }
      } else if ( !strWord.empty() ) {
         const HFONT hfont = bBold ? hfontBold : hfontNormal;
         SelectFont( hdc, hfont );

         verify( GetTextExtentPoint32( hdc, 
            strWord.c_str(), strWord.length(), &sizeText ) );
         if ( rc.right < ptCurrPos.x + sizeText.cx ) {
            if ( PHTML_SINGLE_LINE & dwFlags ) {
               const int nWidth = rc.right - ptCurrPos.x;
               strWord = compactPath( 
                  strWord.c_str(), nWidth, hfont );
               textOut( hdc, strWord.c_str(), bDisabled );
               
               break; //*** LOOP EXIT POINT
            
            } else if ( rc.left < ptCurrPos.x ) {
               verify( MoveToEx( 
                  hdc, rc.left, ptCurrPos.y + nLineHeight, 0 ) );
            }
         }
         verify( GetCurrentPositionEx( hdc, &ptCurrPos ) );
         if ( rc.left < ptCurrPos.x || !_istspace( strWord[ 0 ] ) ) {
            textOut( hdc, strWord.c_str(), bDisabled );
         }
         if ( rc.bottom < ptCurrPos.y ) {
            break;
         }
      }
   }
}


/**
 * This is reused elsewhere; in the status bar, for example.
 * This function does *not* change the DC colors.
 */
void paintHTML( 
   HDC hdc, LPCTSTR pszString, 
   RECT *pRect, HFONT hfontNormal, DWORD dwFlags, bool bDisabled ) 
{
   HFONT hfontBold = 0;
   LOGFONT logFont = { 0 };
   if ( GetObject( hfontNormal, sizeof logFont, &logFont ) ) {
      logFont.lfWeight = FW_BOLD;
      hfontBold = CreateFontIndirect( &logFont );
   }
   if ( 0 == hfontBold ) {
      hfontBold = hfontNormal;
   }

   HRGN hrgnClip = CreateRectRgnIndirect( pRect );
   assert( 0 != hrgnClip );

   const int nSavedDC = SaveDC( hdc );

   const int nTextAlign = GetTextAlign( hdc );
   verify( 
      GDI_ERROR != SetTextAlign( hdc, nTextAlign | TA_UPDATECP ) );
   verify( ERROR != SelectClipRgn( hdc, hrgnClip ) );
   SelectFont( hdc, hfontNormal );

   doPaint( hdc, pszString, *pRect, 
      hfontNormal, hfontBold, dwFlags, bDisabled );

   verify( RestoreDC( hdc, nSavedDC ) );

   DeleteRgn( hrgnClip );
   if ( hfontNormal != hfontBold ) {
      DeleteFont( hfontBold );
   }
}

PRIVATE void paintWithDC( HWND hwnd, HDC hdc ) {

   const String *pString = (String *) s_HTML_Subclasser.getUserData( hwnd );
   const int bufferSize = pString->length() + 1;
   AutoString pszText( new TCHAR[ bufferSize ] );
   _tcscpy_s( pszText, bufferSize, pString->c_str() );

   Rect rc = getClientRect( hwnd );
   --rc.bottom;

   const int nSavedDC = SaveDC( hdc );
   
   const int nHighlight = getHighlight( hwnd );
   const COLORREF crText = GetSysColor( 
      1 == nHighlight ? COLOR_HIGHLIGHTTEXT : 
      2 == nHighlight ? COLOR_INFOTEXT      : 
      IsWindowEnabled( hwnd ) ? COLOR_BTNTEXT : COLOR_GRAYTEXT );
   const COLORREF crBk = GetSysColor( 
      1 == nHighlight ? COLOR_HIGHLIGHT : 
      2 == nHighlight ? COLOR_INFOBK    : COLOR_3DFACE );

   SetTextColor( hdc, crText );
   SetBkColor  ( hdc, crBk   );

   if ( 0 < nHighlight ) {
      rc.left += 3; // Looks best with some extra offset
   }

#ifdef _ESTORE // TODO!!!
   const HFONT hfont = GetWindowFont( hwnd );
#else
   const HFONT hfont = MenuFont::getFont();
#endif

   paintHTML( hdc, pszText, &rc, hfont,
      PHTML_MULTI_LINE, !IsWindowEnabled( hwnd ) );

   verify( RestoreDC( hdc, nSavedDC ) );
}


PRIVATE void onPaint( HWND hwnd, HDC hdc ) {

   if ( 0 != hdc ) {
      paintWithDC( hwnd, hdc );
   } else {
      PaintStruct ps( hwnd );
      paintWithDC( hwnd, ps.hdc );
   }
}


PRIVATE void fillBk( HWND hwnd, HDC hdc ) {

   assert( IsWindow( hwnd ) );
   const Rect rc = getClientRect( hwnd );
   HRGN hrgnClip = CreateRectRgnIndirect( &rc );
   assert( 0 != hrgnClip );
   const int nSavedDC = SaveDC( hdc );
   verify( ERROR != SelectClipRgn( hdc, hrgnClip ) );
   const int nHighlight = getHighlight( hwnd );
   const int sysColorIndex = 
      1 == nHighlight ? COLOR_HIGHLIGHT : 
      2 == nHighlight ? COLOR_INFOBK    : COLOR_3DFACE;
   fillSysColorSolidRect( hdc, &rc, sysColorIndex );
   verify( RestoreDC( hdc, nSavedDC ) );
   DeleteRgn( hrgnClip );
}


PRIVATE LRESULT CALLBACK Simple_HTML_WndProc( 
   HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) 
{
   LRESULT lResult = 0;
   if ( WM_PAINT == msg ) {
      onPaint( hwnd, reinterpret_cast< HDC >( wParam ) );
   } else if ( WM_ERASEBKGND == msg ) {
      fillBk( hwnd, reinterpret_cast< HDC >( wParam ) );
   } else if ( WM_SETTEXT == msg ) {
#if 1
      String *pString = s_HTML_Subclasser.getUserData( hwnd );
      pString->assign( (LPCTSTR) lParam );
      //const Rect rc = getClientRect( hwnd );
      //InvalidateRect( hwnd, &rc, TRUE );
      ClientDC dc( hwnd );
      fillBk     ( hwnd, dc );
      paintWithDC( hwnd, dc );
#else
      // The nasty static paints in response to WM_SETTEXT! 
      SetWindowRedraw( hwnd, FALSE );
      lResult = 
         s_HTML_Subclasser.callOldProc( hwnd, msg, wParam, lParam );
      SetWindowRedraw( hwnd, TRUE );
      //InvalidateRect( hwnd, 0, TRUE );
#if 1
      const bool bHighLight = hasHighlight( hwnd );
      const Rect rc = getClientRect( hwnd );
      HDC hdc = GetDC( hwnd );
      fillSysColorSolidRect( hdc, &rc,
         bHighLight ? COLOR_HIGHLIGHT : COLOR_BTNFACE );
      paintWithDC( hwnd, hdc );
      ReleaseDC( hwnd, hdc );
#endif
#endif
   } else {
      if ( WM_NCDESTROY == msg ) {
         removeHighlight( hwnd );
      }
      if ( 0x128 != msg ) { // Discuss!
         lResult = s_HTML_Subclasser.callOldProc( 
            hwnd, msg, wParam, lParam );
      }
   }

   return lResult;
}


void subclassHTML( HWND hwnd ) throw( SubclassException ) {

   assert( IsWindow( hwnd ) );
   const int nLength = GetWindowTextLength( hwnd );
   AutoString pszWindowText( new TCHAR[ nLength + 1 ] );
   GetWindowText( hwnd, pszWindowText, nLength + 1 );
   String *pString = new String( pszWindowText );
   verify( s_HTML_Subclasser.subclass( hwnd, pString ) );
}

// end of file
