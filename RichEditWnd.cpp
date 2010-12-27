/*
 * $Header: /Book/RichEditWnd.cpp 24    16.07.04 10:42 Oslph312 $
 */

#include "precomp.h"
#include "RichEdit.h"
#include "AbstractEditWnd.h"
#include "Exception.h"
#include "AutoArray.h"
#include "InstanceSubclasser.h"
#include "winUtils.h"
#include "utils.h"
#include "resource.h"
#include "EditWordBreakProc.h"

#define MAX_UNDO 500

#pragma code_seg( "RICHEDIT_SEG" )


#ifndef EM_SETTYPOGRAPHYOPTIONS
#define EM_SETTYPOGRAPHYOPTIONS	(WM_USER + 202)
#endif

#ifndef EM_GETTYPOGRAPHYOPTIONS
#define EM_GETTYPOGRAPHYOPTIONS	(WM_USER + 203)
#endif

#ifndef TO_SIMPLELINEBREAK
#define	TO_SIMPLELINEBREAK 2
#endif


/**
 * http://msdn.microsoft.com/en-us/library/bb787873(VS.85).aspx
 */
class RichEditWnd : public AbstractEditWnd {
private:
   LPTSTR getText( int nStart, int nEnd ) const;
   bool _isV3;
   void adjustSettings( void ); // TODO - copy from EditWnd
   int m_nSpacesPerTab;         // TODO - copy from EditWnd

public:
   RichEditWnd( HWND hwndParent, LPCTSTR pszText );

   virtual int getSearchText( LPTSTR psz, UINT cb ) const;
   virtual void getText( LPTSTR psz, UINT cb ) const;
   virtual void setText( LPCTSTR psz );

   virtual int getTextLength( bool bCurPosCompatible = false ) const;
   virtual Point getCurPos( void ) const;

   virtual bool getSel( int *pnStart = 0, int *pnEnd = 0 ) const;
   virtual bool getSel( String *pstrSelection ) const;
   virtual void setSel( int nStart = 0, int nEnd = -1 );
   virtual void replaceSel( LPCTSTR psz );
   virtual bool getWord( String *pstrWord ) const;
   
   virtual int lineFromChar( UINT ich ) const;
   virtual int getFirstVisibleLine( void ) const;
   virtual void setFirstVisibleLine( int nLine ) const;

   virtual bool canUndo( void ) const;
   virtual bool undo( void );
   virtual String getUndoName( void ) const;

   virtual bool hasRedo( void ) const;
   virtual bool canRedo( void ) const;
   virtual bool redo( void );
   virtual String getRedoName( void ) const;

   virtual bool canSetTabs( void ) const;

   virtual bool isDirty( void ) const;
   virtual void clean( bool bEmptyUndo = true );

   virtual void cutSelection( void );
   virtual void copySelection( void );
   virtual void paste( void );
   virtual void deleteSelection( void );

   virtual void setReadOnly( bool bReadOnly );
   virtual bool isReadOnly( void ) const;

   virtual void setSpacesPerTab( int nSpaces );
   virtual int getSpacesPerTab( void ) { return m_nSpacesPerTab; }
   virtual void setWordWrap( bool bWordWrap );

#if 0
   virtual bool searchAndSelect( 
      const String& strSearchPattern, 
      const bool bMatchWholeWord, 
      const bool bMatchCase,
      const int  nDirection );

   virtual int replaceInSelection( 
      const String& strSearchPattern, 
      const String& strReplacePattern,
      const bool bMatchWholeWord, 
      const bool bMatchCase );
#endif

private:
#if 0
   int search( const UINT uiFlags, FINDTEXTEX *pFindText );
#endif

   String translateUndoCommand ( const UNDONAMEID unid ) const;
};


void RichEditWnd::adjustSettings( void ) {
   setSpacesPerTab( m_nSpacesPerTab );
   sendMessage( EM_SETMARGINS, EC_USEFONTINFO );
   sendMessage( EM_SETLIMITTEXT, 0 );
}


AbstractEditWnd *createRichEdit( HWND hwndParent, LPCTSTR pszText ) {
   return new RichEditWnd( hwndParent, pszText );
}


RichEditWnd::RichEditWnd( HWND hwndParent, LPCTSTR pszText ) 
   : m_nSpacesPerTab( 3 ) 
{
   assert( IsWindow( hwndParent ) );
   assert( isGoodStringPtr( pszText ) );

   {
       HWND hwnd = CreateWindowEx(
           WS_EX_CLIENTEDGE, RICHEDIT_CLASS,
           _T( "" ),
           WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | 
           ES_AUTOHSCROLL | ES_AUTOVSCROLL | 
           ES_MULTILINE | ES_SELECTIONBAR | ES_DISABLENOSCROLL,
           0, 0, 0, 0, hwndParent, (HMENU) IDC_EDIT,
           getModuleHandle(), 0 );
       if ( !IsWindow( hwnd ) ) {
           throwException( _T( "error creating rich edit window" ) );
       }

       // Trick to figure out whether this is RE 2.0 or RW 3.0:
       // Must do this before cubclassing, because the toolbar isn't done yet
       // and the parentListener depends on that. TODO -- bad dependency!
       SNDMSG( hwnd, EM_SETTYPOGRAPHYOPTIONS, TO_SIMPLELINEBREAK, TO_SIMPLELINEBREAK );
       _isV3 = 0 != SNDMSG( hwnd, EM_GETTYPOGRAPHYOPTIONS, 0, 0 );
       
       attach( hwnd );
       
       // TODO -- throw in attach on invalid hwnd?
       if ( !IsWindow( m_hwnd ) ) {
           throwException( _T( "error creating rich edit window" ) );
       }
   }

   sendMessage( EM_SETTEXTMODE , TM_PLAINTEXT | TM_MULTILEVELUNDO );
   sendMessage( EM_SETUNDOLIMIT, MAX_UNDO );
   sendMessage( EM_EXLIMITTEXT , 0, INT_MAX ); // 2 GB - 1

   sendMessage( EM_SETOPTIONS, ECOOP_OR, ECO_AUTOWORDSELECTION );
   sendMessage( EM_AUTOURLDETECT, 1, 0 );

   sendMessage( WM_SETFONT, (WPARAM) GetStockFont( DEFAULT_GUI_FONT ) );
   sendMessage( EM_SETMARGINS, EC_USEFONTINFO );
   DWORD dwEventMask = sendMessage( EM_GETEVENTMASK );
   dwEventMask |= ENM_CHANGE;
   sendMessage( EM_SETEVENTMASK, 0, dwEventMask );
   installEditWordBreakProcW( *this );
   setText( pszText );
   clean();
}


/**
 * EM_FINDTEXTEX can blow up when under some circumstances. 
 * Catching the access violation does not help; 
 * the whole application becomes decidedly unstable.
 */
#if 0

int RichEditWnd::search( UINT uiFlags, FINDTEXTEX *pFindText ) {

   assert( isGoodConstPtr( this ) );
   assert( isGoodPtr( pFindText ) );
   if ( 0 == pFindText->chrg.cpMin && -1 == pFindText->chrg.cpMax ) {
      AutoString pszText( getText( 0, -1 ) );
      if ( _tcslen( pFindText->lpstrText ) == _tcslen( pszText ) ) {
         return -1; // ... or we die horribly.
      }
   }

   try {
      return sendMessage( EM_FINDTEXTEX, uiFlags, 
         reinterpret_cast< LPARAM >( pFindText ) );
   }
   catch ( const AccessViolationException& x ) {
      trace( _T( "AccessViolationException in RichEditWnd::search" ),
         x.what() );
      // TODO -- see what happens with BC in place
   }
   return -1;
}

#endif // 0


/**
 * This is used by both getUndoName and getRedoName.
 */
String RichEditWnd::translateUndoCommand ( UNDONAMEID unid ) const {

   int nStringID = 0;
   switch ( unid ) {
   case UID_TYPING  : nStringID = IDS_UNDO_TYPING  ; break;
   case UID_DELETE  : nStringID = IDS_UNDO_DELETE  ; break;
   case UID_DRAGDROP: nStringID = IDS_UNDO_DRAGDROP; break;
   case UID_CUT     : nStringID = IDS_UNDO_CUT     ; break;
   case UID_PASTE   : nStringID = IDS_UNDO_PASTE   ; break;
   }

   if ( 0 != nStringID ) {
      return loadString( nStringID );
   }

   return _T( "" );
}


/**
 * The value retrieved in psz uses CR (!) as line feed
 * @return the length of the string.
 */
int RichEditWnd::getSearchText( LPTSTR psz, UINT cb ) const {

   assert( isGoodStringPtr( psz ) );

   TEXTRANGE textRange = { { 0, cb }, psz };
   const long lResult = sendMessage( EM_GETTEXTRANGE, 0, (LPARAM) &textRange );

#ifdef _DEBUG
   const long nRealLength = _tcsclen( textRange.lpstrText );
   assert( nRealLength == lResult );
#endif

   return lResult;
}


/**
 * Returns CR/LF as line feed
 */
void RichEditWnd::getText( LPTSTR psz, UINT cb ) const {

   assert( isGoodStringPtr( psz ) );
   GetWindowText( m_hwnd, psz, cb );
}


void RichEditWnd::setText( LPCTSTR psz ) {

   verify( sendMessage( WM_SETTEXT, 0, 
      reinterpret_cast< LPARAM >( psz ) ) );
}


// The rich edit control reports text length as though it has
// two characters (CR-LF) separating lines, but reports position
// (i.e., getCurPos) as though it has a single character separating
// lines. This is the reason for the bCurPosCompatible parameter;
// when we enable/disable the delete command, we really want to
// know if the caret is at the end of the file, and must compare
// getTextLength(true) with getCurPos().x. Using getTextLength(false)
// yields correct results only on files with no lines feeds at all.
int RichEditWnd::getTextLength( bool bCurPosCompatible ) const {

   int nTextLength = AbstractEditWnd::getTextLength();
   if ( bCurPosCompatible ) {
      int nStart = 0;
      getSel( &nStart );
      const int nLine = lineFromChar( nStart );
      nTextLength -= nLine;
   }
   return nTextLength;
}


Point RichEditWnd::getCurPos( void ) const {
   int nStart = 0;
   getSel( &nStart );
   const int nLine = lineFromChar( nStart );
   const int nColumn = nStart - sendMessage( EM_LINEINDEX, nLine );
   return Point( nColumn, nLine );
}


bool RichEditWnd::getSel( int *pnStart, int *pnEnd ) const {
   
   CHARRANGE charRange = { 0 };
   sendMessage( EM_EXGETSEL, 0, reinterpret_cast< LPARAM >( &charRange ) );

   if ( 0 != pnStart ) {
      *pnStart = charRange.cpMin;
   }
   if ( 0 != pnEnd ) {
      *pnEnd = charRange.cpMax;
   }

   return charRange.cpMin < charRange.cpMax;
}


LPTSTR RichEditWnd::getText( int nStart, int nEnd ) const {

   assert( isGoodConstPtr( this ) );

   const int nLength = nEnd - nStart;
   LPTSTR pszText = new TCHAR[ nLength + 1 ];
   memset( pszText, 0, (nLength + 1 ) * sizeof *pszText );

   if ( 0 < nLength ) {
      TEXTRANGE textRange = { { nStart, nEnd }, pszText };
#ifdef _DEBUG
      const long lResult = 
#endif
         sendMessage( EM_GETTEXTRANGE, 0, (LPARAM) &textRange );
      
#ifdef _DEBUG
      const long nRealLength = _tcsclen( textRange.lpstrText );
      assert( nRealLength == lResult );
#endif
      pszText[ nLength ] = 0;
   }

   return pszText;
}


// LATER: Some common code with getWord.
bool RichEditWnd::getSel( String *pstrSelection ) const {
   
   assert( isGoodConstPtr( this ) );
   assert( isGoodPtr( pstrSelection ) );
   
   int nStart = 0;
   int nEnd   = 0;
   if ( !getSel( &nStart, &nEnd ) ) {
      pstrSelection->erase();
      return false;
   }

   assert( nStart < nEnd );
   const int nLength = nEnd - nStart;
   pstrSelection->reserve( nLength + 1 );

   AutoString pszText( getText( nStart, nEnd ) );
   pstrSelection->assign( pszText );

   return true;
}


void RichEditWnd::setSel( int nStart, int nEnd ) {

   assert( isGoodPtr( this ) );
   CHARRANGE charRange = { nStart, nEnd };
   sendMessage( EM_EXSETSEL, 0, reinterpret_cast< LPARAM >( &charRange ) );
}


void RichEditWnd::replaceSel( LPCTSTR psz ) {
   
   assert( isGoodPtr( this ) );
   assert( isGoodStringPtr( psz ) );
   sendMessage( EM_REPLACESEL, TRUE, reinterpret_cast< LPARAM >( psz ) );
}


bool RichEditWnd::getWord( String *pstrWord ) const {

   assert( isGoodConstPtr( this ) );
   assert( isGoodPtr( pstrWord ) );

   int nStart = 0;
   getSel( &nStart );
   const int nLine = lineFromChar( nStart );
   int nLineStart = sendMessage( EM_LINEINDEX, nLine );
   assert( 0 <= nLineStart );

   int nLineEnd = sendMessage( EM_LINEINDEX, nLine + 1 );
   if ( -1 == nLineEnd ) { // // If nLine is the last line.
      nLineEnd = getTextLength(); 
   }
   const int nLength = nLineEnd - nLineStart;
   AutoString pszText( getText( nLineStart, nLineEnd ) );
   int nEnd = nStart;

   const int nWordBreakFlags = sendMessage( EM_FINDWORDBREAK, WB_CLASSIFY, nStart );

   if ( ((WBF_ISWHITE | WBF_BREAKLINE) & nWordBreakFlags) || getTextLength() <= nStart ) {
      nStart = sendMessage( EM_FINDWORDBREAK, WB_MOVEWORDLEFT, nStart );
   } else {
      if ( nLineStart < nStart ) {
         const int nClassAndFlags =
            sendMessage( EM_FINDWORDBREAK, WB_CLASSIFY, nStart - 1 );
         if ( nWordBreakFlags == nClassAndFlags ) {
            nStart = sendMessage( EM_FINDWORDBREAK, WB_MOVEWORDLEFT, nStart );
         }
      }
      while ( nEnd < nLineEnd && 
              nWordBreakFlags == sendMessage( EM_FINDWORDBREAK, WB_CLASSIFY, nEnd ) )
      {
         ++nEnd;
      }
   }

   nStart -= nLineStart;
   if ( nStart < 0 ) {
      nStart = 0;
   }

   nEnd -= nLineStart;
   if ( nLength < nEnd ) {
      nEnd = nLength;
   }

   pszText[ nEnd ] = 0;
   pstrWord->assign( pszText + nStart );

   return nStart < nEnd;
}


int RichEditWnd::lineFromChar( UINT ich ) const {
   assert( isGoodConstPtr( this ) );
   int nLine = (int) sendMessage( EM_EXLINEFROMCHAR, 0, ich );

   // The rich edit control has a bug -- if the caret is on the last
   // line, and the line is empty, it reports one line less than
   // the actual line.
   if ( 0 < ich ) {
      AutoString pszPrevChar( getText( ich - 1, ich + 1 ) );
      if ( _T( '\r' ) == pszPrevChar[ 0 ] && 0 == pszPrevChar[ 1 ] ) {
         ++nLine;
      }
   }
   return nLine;
}


int RichEditWnd::getFirstVisibleLine( void ) const {
   return sendMessage( EM_GETFIRSTVISIBLELINE );
}


void RichEditWnd::setFirstVisibleLine( int nLine ) const {
   sendMessage( EM_LINESCROLL, 0, nLine );
   sendMessage( EM_SCROLLCARET );
}


bool RichEditWnd::canUndo( void ) const {
   return !isReadOnly() && 0 != sendMessage( EM_CANUNDO );
}


bool RichEditWnd::undo( void ) {
   return !isReadOnly() && 0 != sendMessage( WM_UNDO );
}


String RichEditWnd::getUndoName( void ) const {

   if ( canUndo() ) {
      const UNDONAMEID unid = (UNDONAMEID) sendMessage( EM_GETUNDONAME );
      return translateUndoCommand( unid );
   }
   return _T( "" );
}


bool RichEditWnd::hasRedo( void ) const {
   
   return true;
}


bool RichEditWnd::canRedo( void ) const {
   
   return !isReadOnly() && 0 != sendMessage( EM_CANREDO );
}


bool RichEditWnd::redo( void ) {

   return !isReadOnly() && 0 != sendMessage( EM_REDO );
}


String RichEditWnd::getRedoName( void ) const {
   
   if ( canRedo() ) {
      const UNDONAMEID unid = (UNDONAMEID) sendMessage( EM_GETREDONAME );
      return translateUndoCommand( unid );
   }
   return _T( "" );
}


bool RichEditWnd::canSetTabs( void ) const {

   return _isV3;
}


bool RichEditWnd::isDirty( void ) const {
   
   return 0 != sendMessage( EM_GETMODIFY );
}


void RichEditWnd::clean( bool bEmptyUndo ) {
   
   assertValid();
   sendMessage( EM_SETMODIFY, 0 );
   if ( bEmptyUndo ) {
      sendMessage( EM_EMPTYUNDOBUFFER );
   }
   assert( !isDirty() );
}


void RichEditWnd::cutSelection( void ) {
   
   sendMessage( WM_CUT );
}


void RichEditWnd::copySelection( void ) {
   
   sendMessage( WM_COPY );
}


void RichEditWnd::paste( void ) {
   
   sendMessage( WM_PASTE );
}


void RichEditWnd::deleteSelection( void ) {
   
   sendMessage( WM_CLEAR );
}


void RichEditWnd::setReadOnly( bool bReadOnly ) {
   
   assertValid();
   assert( !isDirty() );
   sendMessage( EM_SETREADONLY, (WPARAM) bReadOnly );
   const COLORREF crBk = GetSysColor( COLOR_3DFACE );
   sendMessage( EM_SETBKGNDCOLOR, !bReadOnly, crBk );
}


bool RichEditWnd::isReadOnly( void ) const {
   
   return 0 != (ES_READONLY & GetWindowLong( *this, GWL_STYLE ) ); 
}


/**
 * This is not a nice way of doing it, 
 * but what choice do we have? EM_SETTABSTOPS doesn't work...
 * Come to think of it, this doesn't work either.
 * As of REC3, EM_SETTABSTOPS actually works.
 */
void RichEditWnd::setSpacesPerTab( int nSpaces ) {

   m_nSpacesPerTab = nSpaces;
   const int nDlgUnitsPerHorizontalChar = 4;
   DWORD dwTab = nSpaces * nDlgUnitsPerHorizontalChar;
   sendMessage( EM_SETTABSTOPS, 1, 
      reinterpret_cast< LPARAM >( &dwTab ) );
   InvalidateRect( m_hwnd, 0, false );

#if 0 // Unsuccesful attempt to fix pre-3 versions of the REC
   int nStart = 0;
   int nEnd   = 0;
   getSel( &nStart, &nEnd ); // Hide updates during this operation?
   //setSel( 0, -1 );
   //setSel( 0, 0 );

   static const TCHAR sz20_Blanks[] = _T( "                    " );
   assert( 20 == _tcslen( sz20_Blanks ) );
   const SIZE size = 
      measureString( sz20_Blanks, GetWindowFont( *this ) );
   const int nTwipsPerSpace = printerPointsFromDevPoints( size.cx );
   const int nTwipsPerTab = nSpaces * nTwipsPerSpace;
   int nTwipsForThisTab = 0;

   PARAFORMAT2 paraFormat;
   paraFormat.cbSize = sizeof paraFormat;
   paraFormat.dwMask = PFM_TABSTOPS;
   sendMessage( EM_GETPARAFORMAT, 
      0, reinterpret_cast< LPARAM >( &paraFormat ) );
   paraFormat.cTabCount = MAX_TAB_STOPS;
   for ( int iTabstop = 0; iTabstop < MAX_TAB_STOPS; ++iTabstop ) {
      nTwipsForThisTab += nTwipsPerTab;
      trace( _T( "Tabstop %2d: %4d -> %4d\n" ), 
         iTabstop, paraFormat.rgxTabs[ iTabstop ], nTwipsForThisTab );
      paraFormat.rgxTabs[ iTabstop ] = nTwipsForThisTab;
   }

   const int nLength = getTextLength();
   AutoString pszContents( nLength + 1 ); 
   getText( pszContents, nLength + 1 );
   setText( _T( "" ) );
   sendMessage( EM_SETTEXTMODE, TM_RICHTEXT | TM_MULTILEVELUNDO );
   sendMessage( EM_SETPARAFORMAT, 
      0, reinterpret_cast< LPARAM >( &paraFormat ) );
   sendMessage( EM_SETTEXTMODE, TM_PLAINTEXT | TM_MULTILEVELUNDO );
   setText( pszContents );

   setSel( nStart, nEnd );

   InvalidateRect( *this, 0, false );
#endif
}


void RichEditWnd::setWordWrap( bool bWordWrap )  {

	AbstractEditWnd::setWordWrap( bWordWrap ); // Call base class!

	// www.codeguru.com (Zafir Anjum): This is undocumented!

   if ( bWordWrap ) {
      modifyStyle( m_hwnd, 0, WS_HSCROLL );
   } else {
      modifyStyle( m_hwnd, WS_HSCROLL, 0 );
   }
   sendMessage( EM_HIDESELECTION, TRUE, FALSE );
   sendMessage( EM_SETTARGETDEVICE, 0, bWordWrap ? 0 : 1 );
   sendMessage( EM_HIDESELECTION, FALSE, FALSE );

   // ...and this does not work:
   //sendMessage( EM_SETOPTIONS, ECOOP_XOR, ECO_AUTOHSCROLL );
}


#if 0
bool RichEditWnd::searchAndSelect(
   const String& strSearchPattern, 
   const bool bMatchWholeWord, 
   const bool bMatchCase,
   const int  nDirection )
{
   UINT uiFlags = 0;
   if ( 1 == nDirection ) {
      uiFlags |= FR_DOWN;
   }
   if ( bMatchWholeWord ) {
      uiFlags |= FR_WHOLEWORD;
   }
   if ( bMatchCase ) {
      uiFlags |= FR_MATCHCASE;
   }

   int nStart = 0;
   if ( getSel( &nStart ) ) {
      ++nStart;
   }

   FINDTEXTEX findText = { 
      { nStart, -1 }, 
      const_cast< LPTSTR >( strSearchPattern.c_str() ) 
   };

   int nPos = search( uiFlags, &findText );
   if ( -1 == nPos ) {
      findText.chrg.cpMin = 0;
      if ( -1 == nDirection ) {
         findText.chrg.cpMin = getTextLength();
      }
      nPos = search( uiFlags, &findText );
   }
   if ( 0 <= nPos ) {
      setSel( nPos, nPos + strSearchPattern.length() );
   }

   return 0 <= nPos;
}

#endif // 0

// end of file
