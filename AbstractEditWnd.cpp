/*
 * $Header: /Book/AbstractEditWnd.cpp 21    16.07.04 10:42 Oslph312 $
 * Contains functionality common to edit and rich edit controls.
 * TODO: Split into a _really_ abstract base class and a concrete impl for Edit & RE
 */

#include "precomp.h"
#include <richedit.h>
#include "AbstractEditWnd.h"
#include "EditWnd.h"
#include "RichEditWnd.h"
#include "ClientDC.h"
#include "InstanceSubclasser.h"
#include "search.h"
#include "trace.h"
#include "utils.h"
#include "RedrawHider.h"


#ifdef _DEBUG
bool AbstractEditWnd::bForceEdit = false;
#endif


PRIVATE LRESULT CALLBACK editParentSpy( 
   HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );

PRIVATE InstanceSubclasser parentSpy( editParentSpy );

PRIVATE LRESULT CALLBACK editParentSpy( 
   HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam ) 
{
   assert( IsWindow( hwnd ) );
   if ( WM_COMMAND == msg ) {
      AbstractEditWnd *pEditWnd = reinterpret_cast< 
         AbstractEditWnd * >( parentSpy.getUserData( hwnd ) );
      if ( 0 != pEditWnd ) {
         assert( isGoodPtr( pEditWnd ) );

         const HWND hwndCtl = GET_WM_COMMAND_HWND( wParam, lParam );
         const int  idCtl   = GET_WM_COMMAND_ID  ( wParam, lParam );
         if ( (HWND) *pEditWnd == hwndCtl &&
              GetWindowID( *pEditWnd ) == idCtl )
         {
            assert( IsWindow( *pEditWnd ) );
            assert( GetParent( *pEditWnd ) == hwnd );
            EditListener *pListener = pEditWnd->getEditListener();
            assert( isGoodPtr( pListener ) );

            switch ( GET_WM_COMMAND_CMD( wParam, lParam ) ) {
            case EN_CHANGE  : return pListener->onChange  (), 0;
            case EN_ERRSPACE: return pListener->onErrSpace(), 0;
            case EN_MAXTEXT : return pListener->onMaxText (), 0;
            default:
               break;
            }
         }
      }
   }
   return parentSpy.callOldProc( hwnd, msg, wParam, lParam );
}


/**
 * Static factory method to create 
 * a concrete implementation of an AbstractEditWnd.
 */
AbstractEditWnd *AbstractEditWnd::create( 
   HWND hwndParent, LPCTSTR pszText, 
   EditListener *pEditListener, bool bWordWrap ) 
{
   const HMODULE hmodule = 

#ifdef _DEBUG
      bForceEdit ? 0 :
#endif

   GetModuleHandle(_T("RICHED20.DLL"));
   GetModuleHandle(_T("Msftedit.DLL"));

#if 0
   pszText = "!This is a test!";
#endif

   AbstractEditWnd *pEditWnd = 0;
   if ( 0 != hmodule ) {
      pEditWnd = createRichEdit( hwndParent, pszText );
   } else {
      pEditWnd = createEdit( hwndParent, pszText, bWordWrap );
   }

#if 0
	for ( int i = 0; i < 256; ++i ) {
		TCHAR test[ 2 ] = "x";
		test[ 0 ] = i;
		pEditWnd->setSel( 0, 1 );
		pEditWnd->replaceSel( test );
		int result = SendMessage( *pEditWnd, EM_FINDWORDBREAK, WB_CLASSIFY, 0 );
		trace( "%3d: %d %lc\n", i, result, i );
	}
#endif

   assert( 0 != pEditWnd );
   assert( 0 != pEditListener );
   pEditWnd->m_pEditListener = pEditListener;
   parentSpy.subclass( hwndParent, pEditWnd );
   return pEditWnd;
}


class AbstractEditContents {
private:
   const AbstractEditWnd *m_pEditWnd;
   LPTSTR m_psz;
   int m_nLength;

public:
   AbstractEditContents( const AbstractEditWnd *pEditWnd ) 
      : m_pEditWnd( pEditWnd )
      , m_psz( 0 ) 
   {
      lock();
   }
   ~AbstractEditContents() {
      release();
   }
   int length( void ) {
      return m_nLength;
   }
   void lock( void ) {
      const int nLength = m_pEditWnd->getTextLength();
      m_psz = new TCHAR[ nLength + 1 ];
      m_nLength = m_pEditWnd->getSearchText( m_psz, nLength + 1 );
      assert( m_nLength <= nLength );
      assert( isGoodStringPtr( m_psz ) );
      assert( 0 == m_psz[ m_nLength ] );
      m_psz[ m_nLength ] = 0;
   }
   void release( void ) {
      assert( isGoodStringPtr( m_psz ) );
      delete[] m_psz;
      reset_pointer( m_psz );
   }
   operator LPCTSTR() const {
      assert( isGoodStringPtr( m_psz ) );
      return m_psz;
   }
   operator LPTSTR() {
      assert( isGoodStringPtr( m_psz ) );
      return m_psz;
   }
   TCHAR operator [] ( int index ) {
      assert( isGoodStringPtr( m_psz ) );
	  assert( 0 <= index && index < m_nLength );
      return m_psz[ index ];
   }
};


inline bool isCharMsg( UINT msg ) {
	return WM_KEYFIRST <= msg && msg <= WM_KEYLAST;
}


bool AbstractEditWnd::indentSelection( const bool execute ) {

	int start = 0;
	int end = 0;
	if ( !getSel( &start, &end ) ) {
		return true;
	}
	if ( !execute ) {
		return false;
	}

	const int lineStart = lineFromChar( start );
	start = sendMessage( EM_LINEINDEX, lineStart );

	int lineEnd = lineFromChar( end );
	int trialEnd = sendMessage( EM_LINEINDEX, lineEnd );

	AbstractEditContents contents( this );
	if ( trialEnd < end ) {
		trialEnd = sendMessage( EM_LINEINDEX, ++lineEnd );
		if ( trialEnd < end ) { // We must be on the last line, with no linefeed
			end = contents.length();
		} else if ( lineStart == lineEnd - 1 ) { // Selection on single line
			return true; //*** FUNCTION EXIT POINT
		} else {
			end = trialEnd;
		}
	}
	if ( shiftKeyPressed() ) { // Outdent
		for ( int line = lineStart; line < lineEnd; ++line ) {
			contents.release();
			const int charPos = sendMessage( EM_LINEINDEX, line );
			contents.lock();
			int len = 0;
			for ( ; ; ) {
				const TCHAR ch = contents[ charPos + len ];
				if ( ch == _T( '\t' ) ) {
					++len;
					break;
				} else if ( ch == _T( ' ' ) ) {
					if ( getSpacesPerTab() <= ++len ) {
						break;
					}
				} else {
					break;
				}
			}
			setSel( charPos, charPos + len );
			replaceSel( _T( "" ) );
			end -= len;
		}
	} else { // Indent
		for ( int line = lineStart; line < lineEnd; ++line ) {
			const int charPos = sendMessage( EM_LINEINDEX, line );
			setSel( charPos, charPos );
			replaceSel( _T( "\t" ) );
			++end;
		}
	}
	setSel( start, end );
	return false;
}


/**
 * The whole point of this method is to be able to update the 
 * line and column numbers on the status bar, and to enable or
 * disable the cut and copy commands on the tool bar.
 *
 * We don't get any notifications from the edit control when the 
 * user moves the cursor, for example, so we just listen to all 
 * messages and  check if the current position has changed.
 */
LRESULT AbstractEditWnd::dispatch( 
	UINT msg, WPARAM wParam, LPARAM lParam ) 
{
	assertValid();
	static bool bLock = false;
	if ( !bLock ) {
		bLock = true;

		int nStart = 0;
		int nEnd = 0;
		getSel( &nStart, &nEnd );

		const int nLine = lineFromChar( nStart );
		const int nColumn = nStart - sendMessage( EM_LINEINDEX, nLine );
		int numerator = 0;
		int denominator = 0;
		bool zoomChanged = false;
		if ( sendMessage( EM_GETZOOM,
			reinterpret_cast< WPARAM >( &numerator   ),
			reinterpret_cast< WPARAM >( &denominator ) ) )
		{
			zoomChanged = numerator != this->m_numerator ||
			            denominator != this->m_denominator;
		}
		assert( 0 <= nLine );
		assert( 0 <= nColumn );
		if ( nColumn != m_nCurColumn || 
			 nLine   != m_nCurLine   || 
			 nEnd    != m_nLastEnd   || zoomChanged )
		{
			EditListener *pListener = getEditListener();
			if ( 0 != pListener ) { // May be 0 during startup
				m_nCurLine   = nLine;
				m_nCurColumn = nColumn;
				m_nLastEnd   = nEnd;
				this->m_numerator = numerator;
				this->m_denominator = denominator;
				pListener->onPosChange( Point( nColumn, nLine ) );
				if (  zoomChanged ) {
					int zoomPercentage = 100;
					if ( 0 < denominator ) {
						zoomPercentage = MulDiv( 100, numerator, denominator );
					}
					pListener->onZoomChange( zoomPercentage );
				}
			} else {
				resetPosition();
			}
		}
		bLock = false;
	}

	// Indent/outdent?
	// TODO: Extract into own proc(s)
	// TODO: Hide the flashing. Test in both editors.
	// TODO: If single line selection, don't!
	bool doDispatch = true;
	if ( isCharMsg( msg ) && VK_TAB == wParam && !this->isReadOnly() && !wordWrap ) {
		RedrawHider rh( *this );
		doDispatch = indentSelection( WM_CHAR == msg );
	}

	LRESULT result = 0;
	if ( doDispatch ) {
		result = Window::dispatch( msg, wParam, lParam );
	}

	/* TODO -- backspace to match lines above at start of line! */
	// Own function for this!
	if ( WM_CHAR == msg && VK_RETURN == wParam && !this->isReadOnly() ) {
		int nStart = 0;
		int nEnd = 0;
		getSel( &nStart, &nEnd );
		const int nLine = lineFromChar( nStart );
		const int nLineStart = sendMessage( EM_LINEINDEX, nLine - 1 );
		const int nLineLength = sendMessage( EM_LINELENGTH, nLineStart );
		nEnd = sendMessage( EM_LINEINDEX, nLine );
		AbstractEditContents contents( this );
		LPTSTR prevLine = contents + nLineStart;
		prevLine[ nLineLength ] = 0;
		LPTSTR firstNonBlank;
		for ( firstNonBlank = prevLine; isspace( *firstNonBlank ); ++firstNonBlank ) {
			;
		}
		*firstNonBlank = 0;
		replaceSel( prevLine );
	}
	if ( EM_FINDWORDBREAK == msg ) {
		// Never sent...
		trace( _T( "FindWordBreak: %d\n" ), result );
	}
	return result;
}


void AbstractEditWnd::scroll( UINT nWhat ) { 
   
   assertValid();
   sendMessage( WM_VSCROLL, 
      MAKEWPARAM( nWhat, 0 ), (LPARAM)(HWND)*this );
}


int AbstractEditWnd::getVisibleLineCount( HFONT hfont ) const {

   assertValid();
   assert( 0 != hfont );
   
   int nVisibleLineCount = 0;
   RECT rc;
   sendMessage( EM_GETRECT, 0, reinterpret_cast< LPARAM >( &rc ) );
   const int nHeight = rc.bottom - rc.top;

   ClientDC dc( *this );
   if ( dc.isValid() ) {
      HFONT hfontSaved = SelectFont( dc, hfont );
      TEXTMETRIC textMetric;
      bool bOK = 0 != GetTextMetrics( dc, &textMetric );
      SelectFont( dc, hfontSaved );
   
      if ( bOK ) {
         const int nLineHeight = 
            textMetric.tmHeight + textMetric.tmExternalLeading;
         assert( 0 < nLineHeight );
         if ( 0 < nLineHeight ) {
            nVisibleLineCount = nHeight / nLineHeight;
         }
      }
   }

   return nVisibleLineCount;
}


/**
 * Helper method for bringCaretToWindow.
 * Returns new, adjusted insertion point.
 */
int AbstractEditWnd::moveInsertionPoint( int nLine, int nColumn ) {

   assertValid();
   const int nLineStart = sendMessage( EM_LINEINDEX, nLine );
   const int nLineLength = sendMessage( EM_LINELENGTH, nLineStart );
   if ( nLineLength < nColumn ) {
      nColumn = nLineLength;
   }
   return  nLineStart + nColumn;
}


/**
 * Bring the caret into view by moving the caret position,
 * not by scrolling the text in the window, as EM_SCROLLCARET does.
 * Takes no action if there is an actual selection, as it would
 * screw up the selecion.
 */
void AbstractEditWnd::bringCaretToWindow( HFONT hfont ) {

   assertValid();
   int nSelStart, nSelEnd;
   if ( !getSel( &nSelStart, &nSelEnd ) ) {
      assert( nSelStart == nSelEnd );

      const int nTop = sendMessage( EM_GETFIRSTVISIBLELINE );
      const int nCurLine = sendMessage( EM_LINEFROMCHAR, nSelStart );
      int nCurColumn = 
         nSelStart - sendMessage( EM_LINEINDEX, nCurLine );

      if ( nCurLine < nTop ) {
         nSelStart = moveInsertionPoint( nTop, nCurColumn );
         setSel( nSelStart, nSelStart );
      } else {
         const int nLines = getVisibleLineCount( hfont );
         const int nLast = nTop + nLines - 1;
         if ( nLast < nCurLine ) {
            nSelStart = moveInsertionPoint( nLast, nCurColumn );
            setSel( nSelStart, nSelStart );
         }
      }
   }
}


int AbstractEditWnd::getLineCount( void ) const {
   
   assertValid();
   const int nChars = getTextLength();
   if ( 0 == nChars ) {
      return 0;
   }
   return lineFromChar( nChars );
}


int AbstractEditWnd::getTextLength( bool ) const {

   return sendMessage( WM_GETTEXTLENGTH );
}


bool AbstractEditWnd::searchAndSelect( 
   const String& strSearchPattern, 
   const bool bMatchWholeWord, 
   const bool bMatchCase,
   const int  nDirection )
{
   if ( 0 != nDirection && 0 == strSearchPattern.length() ) {
      return false;
   }

   // Find the current selection range and text length:
   int nStart = 0;
   const bool bHasSelection = getSel( &nStart );

   AbstractEditContents pszText( this );
   const int nLength = pszText.length();

   LPCTSTR pszFound = 0;
   int nFound = nStart;

   const int nSearchLength = strSearchPattern.length();
   assert( 0 < nSearchLength );
   LPCTSTR pszSearchPattern = strSearchPattern.c_str();
   if ( 0 < nDirection ) {   // Forward search.
      LPCTSTR pszStart = pszText + nStart + (bHasSelection ? 1 : 0);
      pszFound = find( pszText, pszStart, pszSearchPattern, 
         bMatchWholeWord, bMatchCase );
      if ( 0 == pszFound && (LPCTSTR) pszText < pszStart ) {
         pszFound = find( pszText, pszText, pszSearchPattern, 
            bMatchWholeWord, bMatchCase );
         assert( 0 == pszFound || pszFound - pszText <= nStart );
      }
   } else {                  // Backwards search
      if ( 0 < nStart ) {
         pszFound = findBackwards( pszText, pszText + nStart - 1, 
            pszSearchPattern, bMatchWholeWord, bMatchCase );
      }
      if ( 0 == pszFound ) {
         pszFound = findBackwards( pszText, pszText + nLength, 
            pszSearchPattern, bMatchWholeWord, bMatchCase );
      }
   }

   if ( 0 != pszFound ) {
      nFound = pszFound - pszText;
      setSel( nFound, nFound + nSearchLength );
      sendMessage( EM_SCROLLCARET );
   }

   return 0 != pszFound;
}


int AbstractEditWnd::replaceInSelection( 
   const String& strSearchPattern, 
   const String& strReplacePattern,
   const bool bMatchWholeWord, 
   const bool bMatchCase )
{
   int nStart = 0;
   int nEnd   = 0;
   verify( getSel( &nStart, &nEnd ) );

   const int nSearchLength = strSearchPattern.length();
   assert( 0 < nSearchLength );
   LPCTSTR pszSearchPattern = strSearchPattern.c_str();

   const int nReplaceLength = strReplacePattern.length();
   LPCTSTR pszReplacePattern = strReplacePattern.c_str();

   const int nExtra = nReplaceLength - nSearchLength;

   AbstractEditContents pszText( this );

   int nReplacements;
   for ( nReplacements = 0; ; ++nReplacements ) {
      const LPCTSTR pszStart = pszText + nStart;
      const LPCTSTR pszFound = find( pszText,
         pszStart, pszSearchPattern, bMatchWholeWord, bMatchCase );
      if ( 0 == pszFound ) {
         break; //*** LOOP EXIT POINT
      }
      const int nFound = pszFound - (LPCTSTR) pszText;
      assert( nStart <= nFound );
      if ( nEnd < nFound + nSearchLength ) {
         break; //*** LOOP EXIT POINT
      }

      pszText.release(); 
      setSel( nFound, nFound + nSearchLength );
      replaceSel( pszReplacePattern );
      pszText.lock();
      nStart = nFound + nReplaceLength;
      nEnd += nExtra;
   }

   return nReplacements;
}

// end of file
