/*
 * $Header: /Book/print.cpp 8     20.08.99 16:33 Oslph312 $
 */

#include "precomp.h"
#include "Document.h"
#include "Exception.h"
#include "WaitCursor.h"
#include "persistence.h"
#include "graphics.h"
#include "geometry.h"
#include "utils.h"

HFONT Document::createPrintFont( HDC hdc ) {
   
   LOGFONT logFont = { 0 };
   if ( getFixedFont() ) {
      _tcscpy_s( logFont.lfFaceName, getFixedFace().c_str() );
      logFont.lfHeight = devPointsFromPrinterPoints( getFixedHeight(), hdc );
      logFont.lfWeight = getFixedWeight();
      logFont.lfItalic = getFixedItalic();
      logFont.lfCharSet = getFixedCharSet();
   } else {
      _tcscpy_s( logFont.lfFaceName, getProportionalFace().c_str() );
      logFont.lfHeight = devPointsFromPrinterPoints( getProportionalHeight(), hdc );
      logFont.lfWeight = getProportionalWeight();
      logFont.lfItalic = getProportionalItalic();
      logFont.lfCharSet = getProportionalCharSet();
   }
   return CreateFontIndirect( &logFont );
}


PRIVATE String getLine( LPCTSTR *ppszText ) {

   assert( isGoodPtr( ppszText ) );
   assert( isGoodStringPtr( *ppszText ) );

   const int nLength = _tcscspn( *ppszText, _T( "\r\n" ) );
   String strLine( *ppszText, nLength );
   *ppszText += nLength;
   if ( _T( '\r' ) == **ppszText ) {
      ++*ppszText;
   }
   if ( _T( '\n' ) == **ppszText ) {
      ++*ppszText;
   }

   return strLine;
}


class PrinterDC {
private:
   HDC   m_hdc;
   HFONT m_hfont;
   int   m_nSavedContext;

public:
   void restore() {
      SelectFont( m_hdc, m_hfont );
      SetTextAlign( m_hdc, TA_TOP | TA_LEFT );
   };

   PrinterDC( HDC hdc, HFONT hfont ) 
      : m_hdc( hdc )
      , m_hfont( hfont )
      , m_nSavedContext( SaveDC( hdc ) )
   {
      assert( 0 != m_hdc );
      assert( 0 != m_hfont );
      assert( 0 != m_nSavedContext );
      restore();
   };

   ~PrinterDC() {
      verify( RestoreDC( m_hdc, m_nSavedContext ) );
      verify( DeleteFont( m_hfont ) );
   };

   operator HDC() const {
      return m_hdc;
   };
};


class PrinterDoc {
private:
   HDC m_hdc;

public:
   PrinterDoc( HDC hdc, const String& strTitle ) : m_hdc( hdc ) {

      DOCINFO docInfo = {
         sizeof( DOCINFO ),
         strTitle.c_str(),
      };

      const int nPrintJobId = StartDoc( hdc, &docInfo );
      if ( nPrintJobId <= 0 ) {
         throwException( 
            _T( "Unable to print document (StartDoc failed)" ) );
      }
   };

   ~PrinterDoc() {
      if ( EndDoc( m_hdc ) <= 0 ) {
         trace( _T( "EndDoc failed (%u)\n" ), GetLastError() );
      }
   }
};


PRIVATE void pageHeader( 
   PrinterDC& printerDC, const String strHeader, int *pnPage,
   const RECT& rcPrintArea, LPPOINT pptCurrPos ) 
{
   assert( isGoodPtr( pnPage ) );

   pptCurrPos->y = rcPrintArea.top;
   if ( 0 != *pnPage ) {
      if ( EndPage( printerDC ) <= 0 ) {
         throwException( _T( "EndPage failed" ) );
      }
   }
   if ( StartPage( printerDC ) <= 0 ) {
      throwException( _T( "StartPage failed" ) );
   }

   // Because of Win95, which resets the DC on StartPage.
   printerDC.restore(); 
   ++*pnPage;

   const COLORREF crSaved = 
      SetBkColor( printerDC, RGB( 226, 226, 226 ) );
   const int nSavedBkMode = SetBkMode( printerDC, TRANSPARENT );
   
   SIZE size;
   verify( GetTextExtentPoint32( 
      printerDC, strHeader.c_str(), strHeader.length(), &size ) );

   RECT rcHeader = rcPrintArea;
   rcHeader.bottom = rcHeader.top + size.cy;
   const COLORREF crHeaderBk = RGB( 192, 192, 192 );
   fillSolidRect( printerDC, &rcHeader, crHeaderBk );
   
   verify( DrawText( printerDC, strHeader.c_str(), -1, 
      &rcHeader, DT_LEFT | DT_VCENTER | DT_NOPREFIX ) );
   
   TCHAR szPage[ 10 ] = { 0 };
   wsprintf( szPage, _T( "%d" ), *pnPage );
   verify( DrawText( printerDC, szPage, -1, 
      &rcHeader, DT_RIGHT | DT_VCENTER | DT_NOPREFIX ) );
   
   SetBkMode( printerDC, nSavedBkMode );
   SetBkColor( printerDC, crSaved );

   pptCurrPos->y += MulDiv( size.cy, 3, 2 );
}


void printSingleCopy( PrinterDC& printerDC, const RECT& rcPrintArea,
   LPCTSTR pszText, const String strHeader, int nTabs ) 
{
   TEXTMETRIC tm = { 0 };
   if ( !GetTextMetrics( printerDC, &tm ) ) {
      throwException( _T( "Failure in GetTextMetrics" ) );
   }
   const int nLineHeight = tm.tmHeight + tm.tmExternalLeading;

   SIZE sizeSpace;
   GetTextExtentPoint32( printerDC, _T( " " ), 1, &sizeSpace );
   const int nPixelsPerTab = nTabs * sizeSpace.cx;

   Point ptCurrPos( rcPrintArea.left, rcPrintArea.top );
   int nPage = 0;
   pageHeader( 
      printerDC, strHeader, &nPage, rcPrintArea, &ptCurrPos );
   
   while ( 0 != *pszText ) {
      String strLine = getLine( &pszText );
      ptCurrPos.x = rcPrintArea.left;
      while ( !strLine.empty() ) {
         int nSpace = strLine.find_first_of( _T( " \t" ) );
         if ( 0 == nSpace ) {
            if ( _T( '\t' ) == strLine[ 0 ] ) {
               int nOffset = ptCurrPos.x - rcPrintArea.left;
               nOffset = (nOffset + nPixelsPerTab) / nPixelsPerTab;
               nOffset *= nPixelsPerTab;
               ptCurrPos.x = rcPrintArea.left + nOffset;
            } else {
               assert( _T( ' ' ) == strLine[ 0 ] );
               ptCurrPos.x += sizeSpace.cx;
            }
            strLine.erase( 0, 1 );
         } else {
            if ( nSpace < 0 ) {
               nSpace = strLine.length();
            }
            assert( 0 < nSpace );
            String strWord( strLine.substr( 0, nSpace ) );
            strLine.erase( 0, nSpace );
            SIZE size;
            GetTextExtentPoint32( 
               printerDC, strWord.c_str(), strWord.length(), &size );
            if ( rcPrintArea.right < ptCurrPos.x + size.cx ) {
               ptCurrPos.x = rcPrintArea.left;
               ptCurrPos.y += nLineHeight;
               if ( rcPrintArea.bottom < ptCurrPos.y + nLineHeight ) {
                  pageHeader( printerDC, 
                     strHeader, &nPage, rcPrintArea, &ptCurrPos );
               }
            }
            verify( TextOut( printerDC, ptCurrPos.x, ptCurrPos.y, 
               strWord.c_str(), strWord.length() ) );
            ptCurrPos.x += size.cx;
         }
      }
      ptCurrPos.y += nLineHeight;
      if ( rcPrintArea.bottom < ptCurrPos.y + nLineHeight ) {
         if ( 0 != *pszText ) {
            pageHeader( printerDC, 
               strHeader, &nPage, rcPrintArea, &ptCurrPos );
         }
      }
   }
   
   if ( EndPage( printerDC ) <= 0 ) {
      throwException( _T( "EndPage failed" ) );
   }
}


/**
 * NOTE: Both MS-DOS and Unix-style line feeds are OK for pszText.
 */
void Document::print( HDC hdc, LPCTSTR pszText, int nCopies ) {

   assertValid();
   assert( isGoodStringPtr( pszText ) );
   WaitCursor waitCursor( _T( "print.ani" ) );

   const int nLogPixelsX      = GetDeviceCaps( hdc, LOGPIXELSX      );
   const int nLogPixelsY      = GetDeviceCaps( hdc, LOGPIXELSY      );
   const int nPhysicalOffsetX = GetDeviceCaps( hdc, PHYSICALOFFSETX );
   const int nPhysicalOffsetY = GetDeviceCaps( hdc, PHYSICALOFFSETY );
   const int nPhysicalWidth   = GetDeviceCaps( hdc, PHYSICALWIDTH   );
   const int nPhysicalHeight  = GetDeviceCaps( hdc, PHYSICALHEIGHT  );

   const int nUnitsPerInch = getMarginsAreMetric() ? 2540 : 1000;
   assert( 0 != nUnitsPerInch );

   int nLeftMargin   = 
      MulDiv( getLeftMargin  (), nLogPixelsX, nUnitsPerInch );
   int nTopMargin    = 
      MulDiv( getTopMargin   (), nLogPixelsY, nUnitsPerInch );
   int nRightMargin  = 
      MulDiv( getRightMargin (), nLogPixelsX, nUnitsPerInch );
   int nBottomMargin = 
      MulDiv( getBottomMargin(), nLogPixelsY, nUnitsPerInch );

   // One-inch defaults:
   if ( 0 == nLeftMargin ) {
      nLeftMargin = nRightMargin = nLogPixelsX;
      nTopMargin = nBottomMargin = nLogPixelsY;
   }

   // Adjust to physical offsets:
   nLeftMargin   = __max( nPhysicalOffsetX, nLeftMargin   );
   nTopMargin    = __max( nPhysicalOffsetY, nTopMargin    );
   nRightMargin  = __max( nPhysicalOffsetX, nRightMargin  );
   nBottomMargin = __max( nPhysicalOffsetY, nBottomMargin );

   const RECT rcPrintArea = {
      nLeftMargin - nPhysicalOffsetX,
      nTopMargin  - nPhysicalOffsetY,
      nPhysicalWidth  - nRightMargin  - nPhysicalOffsetX,
      nPhysicalHeight - nBottomMargin - nPhysicalOffsetY,
   };

   PrinterDoc printerDoc( hdc, getTitle() );
   PrinterDC printerDC( hdc, createPrintFont( hdc ) );

   for ( int iCopy = 0; iCopy < nCopies; ++iCopy ) {
      printSingleCopy(
         printerDC, rcPrintArea, pszText, getTitle(), getTabs() );
   }
}

// end of file
