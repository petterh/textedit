/*
 * $Header: /Book/EditWnd.cpp 20    16.07.04 10:42 Oslph312 $
 */

#include "precomp.h"
#include "Exception.h"
#include "AutoArray.h"
#include "AbstractEditWnd.h"
#include "winUtils.h"
#include "utils.h"
#include "search.h"
#include "os.h"
#include "resource.h"
#include "EditWordBreakProc.h"


#pragma code_seg( "EDIT_SEG" )


class EditWnd : public AbstractEditWnd {
private:
   void adjustSettings( void );
   int m_nSpacesPerTab;

public:
   EditWnd( HWND hwndParent, LPCTSTR pszText, bool bWordWrap );

   virtual int getSearchText( LPTSTR psz, UINT cb ) const;
   virtual void getText( LPTSTR psz, UINT cb ) const;
   virtual void setText( LPCTSTR psz );

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

   friend class EditContents;
};


AbstractEditWnd *createEdit( 
   HWND hwndParent, LPCTSTR pszText, bool bWordWrap ) 
{
   return new EditWnd( hwndParent, pszText, bWordWrap );
}


class EditContents {
private:
   const EditWnd *m_pEditWnd;
   LOCALHANDLE   m_handle;
   LPTSTR        m_psz;
   bool          m_bUseHandle;

public:
   EditContents( const EditWnd *pEditWnd ) 
      : m_pEditWnd( pEditWnd )
      , m_handle( reinterpret_cast< HANDLE >( 
         pEditWnd->sendMessage( EM_GETHANDLE ) ) )
      , m_psz( 0 ) 
      , m_bUseHandle( false ) // isWindowsNT() -- do we get Unicode with XP?
   {
      assert( 0 != m_handle );
      if ( 0 != m_handle ) {
         lock();
      }
   }
   ~EditContents() {
      release();
   }
   void lock( void ) {
      if ( m_bUseHandle ) {
         m_psz = reinterpret_cast< LPTSTR >( LocalLock( m_handle ) );
      } else {
         const int nLength = m_pEditWnd->getTextLength();
         m_psz = new TCHAR[ nLength + 1 ];
         m_pEditWnd->getText( m_psz, nLength + 1 );
         assert( 0 == m_psz[ nLength ] );
         m_psz[ nLength ] = 0;
      }
      assert( isGoodStringPtr( m_psz ) );
   }
   void release( void ) {
      assert( isGoodStringPtr( m_psz ) );
      if ( m_bUseHandle ) {
         LocalUnlock( m_handle );
      } else {
         delete[] m_psz;
      }
      reset_pointer( m_psz );
   }
   operator LPCTSTR() {
      assert( isGoodStringPtr( m_psz ) );
      return m_psz;
   }
};


void EditWnd::adjustSettings( void ) {
   setSpacesPerTab( m_nSpacesPerTab );
   sendMessage( EM_SETMARGINS, EC_USEFONTINFO );
   sendMessage( EM_SETLIMITTEXT, 0 );
}


PRIVATE DWORD getWindowStyle( bool bWordWrap ) {

   DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_VSCROLL | 
      DS_LOCALEDIT | ES_MULTILINE | ES_NOHIDESEL;
   if ( !bWordWrap ) {
      dwStyle |= WS_HSCROLL; // ES_AUTOHSCROLL
   }
   return dwStyle;
}


EditWnd::EditWnd( HWND hwndParent, LPCTSTR pszText, bool bWordWrap ) 
   : m_nSpacesPerTab( 3 ) 
{
   assert( IsWindow( hwndParent ) );
   assert( isGoodStringPtr( pszText ) );

   attach( CreateWindowEx(
      WS_EX_CLIENTEDGE | WS_EX_NOPARENTNOTIFY, 
      _T( "EDIT" ), _T( "" ),
      getWindowStyle( wordWrap = bWordWrap ), 0, 0, 0, 0,
      hwndParent, (HMENU) IDC_EDIT, getModuleHandle(), 0 ) );

   if ( !IsWindow( m_hwnd ) ) {
      throwException( _T( "error creating edit window" ) );
   }

   SetWindowFont( m_hwnd, GetStockFont( DEFAULT_GUI_FONT ), false );
   adjustSettings();
   // Skip this for edit wnd -- too weird for Ctrl+Arrow, inconsistent Unicode/Ansi across OSes.
   // installEditWordBreakProc( *this );
   setText( pszText );
   clean();
}


int EditWnd::getSearchText( LPTSTR psz, UINT cb ) const {

   assert( isGoodStringPtr( psz ) );
   getText( psz, cb );
   return cb - 1;
}


void EditWnd::getText( LPTSTR psz, UINT cb ) const {

   assert( isGoodStringPtr( psz ) );
   GetWindowText( m_hwnd, psz, cb );
}


/**
 * According to the documentation, SetWindowText returns TRUE
 * if succesful. This may be true on Windows NT, but on Windows 95
 * it returns the number of characters set, i.e., 0 in case of
 * an empty string.
 */
void EditWnd::setText( LPCTSTR psz ) {

   assert( isGoodStringPtr( psz ) );

#ifdef _DEBUG
   const BOOL bRetval =
#endif

   SetWindowText( m_hwnd, psz );

#ifdef _DEBUG
   trace( _T( "EditWnd::setText returns %d, strlen = %d\n" ), 
      bRetval, _tcslen( psz ) );
   assert( bRetval || ( !isWindowsNT() && 0 == *psz ) );
#endif
}


Point EditWnd::getCurPos( void ) const {

   int nStart = 0;
   getSel( &nStart );
   const int nLine = lineFromChar( nStart );
   const int nColumn = nStart - sendMessage( EM_LINEINDEX, nLine );
   return Point( nColumn, nLine );
}


bool EditWnd::getSel( int *pnStart, int *pnEnd ) const {

   int nStart = 0;
   int nEnd   = 0;
   sendMessage( EM_GETSEL, 
      reinterpret_cast< WPARAM >( &nStart ), 
      reinterpret_cast< LPARAM >( &nEnd   ) );

   if ( 0 != pnStart ) {
      *pnStart = nStart;
   }
   if ( 0 != pnEnd ) {
      *pnEnd = nEnd;
   }

   return nStart < nEnd;
}


bool EditWnd::getSel( String *pstrSelection ) const {
   
   assert( isGoodPtr( pstrSelection ) );

   int nStart = 0;
   int nEnd   = 0;
   const bool hasSelection = getSel( &nStart, &nEnd );

   if ( hasSelection ) {
      EditContents pszText( this );
      pstrSelection->assign( pszText + nStart, nEnd - nStart );
   }

   return hasSelection;
}


void EditWnd::setSel( int nStart, int nEnd ) {

   sendMessage( EM_SETSEL, nStart, nEnd );
}


void EditWnd::replaceSel( LPCTSTR psz ) {

   const WPARAM can_undo = TRUE;
   sendMessage( EM_REPLACESEL, can_undo, 
      reinterpret_cast< LPARAM >( psz ) );
}


bool EditWnd::getWord( String *pstrWord ) const {

   int nStart = 0;
   getSel( &nStart );
   const int nLine = lineFromChar( nStart );
   int nLineStart = sendMessage( EM_LINEINDEX, nLine );
   assert( 0 <= nLineStart );

   const int nLength = sendMessage( EM_LINELENGTH, nStart );
   if ( nLength <= 0 ) {
      return false;
   }

   AutoString pszText( new TCHAR[ nLength + 1 ] );

   // EM_GETLINE has a truly hackish way of accepting parameters.
   // The start of the buffer is interpreted as a WORD that 
   // contains the number of characters we have room for.
   WORD *pMaxChars = reinterpret_cast< WORD * >( (LPTSTR) pszText );
   assert( sizeof( WORD ) <= (nLength + 1) * sizeof( TCHAR ) );
   *pMaxChars = nLength;

#ifdef _DEBUG
   const long lResult = 
#endif

   sendMessage( EM_GETLINE, nLine, 
      reinterpret_cast< LPARAM >( (LPTSTR) pszText ) );
   assert( lResult == nLength );
   pszText[ nLength ] = 0;

   nStart -= nLineStart;
   int nEnd = nStart;
   while ( 0 < nStart && !isspace( pszText[ nStart - 1 ] ) ) { // TODO: Use char classes? What's in common with rich edit?
      --nStart;
   }
   while ( nEnd < nLength && !isspace( pszText[ nEnd ] ) ) {
      ++nEnd;
   }

   pszText[ nEnd ] = 0;
   pstrWord->assign( pszText + nStart );

   return nStart < nEnd;
}


int EditWnd::lineFromChar( UINT ich ) const {
   return (int) sendMessage( EM_LINEFROMCHAR, (WPARAM) ich );
}


int EditWnd::getFirstVisibleLine( void ) const {
   return sendMessage( EM_GETFIRSTVISIBLELINE );
}


// Only called at startup
void EditWnd::setFirstVisibleLine( int nLine ) const {

   sendMessage( EM_LINESCROLL, 0, nLine );
   sendMessage( EM_SCROLLCARET );
}


bool EditWnd::canUndo( void ) const {
   return !isReadOnly() && 0 != sendMessage( EM_CANUNDO );
}


bool EditWnd::undo( void ) {
   return !isReadOnly() && 0 != sendMessage( WM_UNDO );
}


String EditWnd::getUndoName( void ) const {
   return _T( "" );
}


bool EditWnd::hasRedo( void ) const {
   return false;
}


bool EditWnd::canRedo( void ) const {
   assert( false );
   return false;
}


bool EditWnd::redo( void ) {
   assert( false );
   return false;
}


String EditWnd::getRedoName( void ) const {
   assert( false );
   return _T( "" );
}


bool EditWnd::canSetTabs( void ) const {
   return true;
}


bool EditWnd::isDirty( void ) const {
   return 0 != sendMessage( EM_GETMODIFY );
}


void EditWnd::clean( bool bEmptyUndo ) {
   assertValid();
   sendMessage( EM_SETMODIFY, 0 );
   if ( bEmptyUndo ) {
      sendMessage( EM_EMPTYUNDOBUFFER );
   }
   assert( !isDirty() );
}


void EditWnd::cutSelection( void ) {
   sendMessage( WM_CUT );
}


void EditWnd::copySelection( void ) {
   sendMessage( WM_COPY );
}


void EditWnd::paste( void ) {
   sendMessage( WM_PASTE );
}


void EditWnd::deleteSelection( void ) {
   sendMessage( WM_CLEAR );
}


void EditWnd::setReadOnly( bool bReadOnly ) {
   sendMessage( EM_SETREADONLY, (WPARAM) bReadOnly );
}


bool EditWnd::isReadOnly( void ) const {
   return 0 != (ES_READONLY & GetWindowLong( m_hwnd, GWL_STYLE ) ); 
}


void EditWnd::setSpacesPerTab( int nSpaces ) {
   
   m_nSpacesPerTab = nSpaces;
   const int nDlgUnitsPerHorizontalChar = 4;
   DWORD dwTab = nSpaces * nDlgUnitsPerHorizontalChar;
   sendMessage( EM_SETTABSTOPS, 1, 
      reinterpret_cast< LPARAM >( &dwTab ) );
   InvalidateRect( m_hwnd, 0, false );
}


/**
 * This one is slightly nutty, as we have to switch windows
 * to get the thing to recognize the changed ES_AUTOHSCROLL.
 */
void EditWnd::setWordWrap( bool bWordWrap )  {

	AbstractEditWnd::setWordWrap( bWordWrap ); // Call base class!

   const DWORD dwOldStyle = GetWindowStyle( *this );
   const bool hadWordWrap = 0 == (WS_HSCROLL & dwOldStyle);
   if ( hadWordWrap == bWordWrap ) {
      return;
   }

   const bool wasReadOnly = isReadOnly();
   int nOldStart = 0;
   int nOldEnd = 0;
   getSel( &nOldStart, &nOldEnd );

   ShowWindow( m_hwnd, SW_HIDE );
   const int nLength = getTextLength();
   AutoString pszText( new TCHAR[ nLength + 1 ] );
   getText( pszText, nLength + 1 );
   setText( _T( "" ) );

   HWND hwndOld = m_hwnd;
   HFONT hfontOld = GetWindowFont( hwndOld );
   HWND hwndParent = GetParent( hwndOld );

   Rect rcPos = getWindowRectInParent( hwndOld );
   HWND hwndNew = CreateWindowEx(
   /* dwExStyle,    */ WS_EX_CLIENTEDGE | WS_EX_NOPARENTNOTIFY, 
   /* lpClassName,  */ _T( "EDIT" ),
   /* lpWindowName, */ _T( "" ),
   /* dwStyle,      */ getWindowStyle( bWordWrap ),
   /* X,            */ rcPos.left,
   /* Y,            */ rcPos.top,
   /* nWidth,       */ rcPos.width(),
   /* nHeight,      */ rcPos.height(),
   /* hwndParent ,  */ hwndParent,
   /* hMenu,        */ (HMENU) IDC_EDIT,
   /* hInstance,    */ getModuleHandle(),
   /* lpParam);     */ 0 );

   if ( !IsWindow( hwndNew ) ) {
      throwException( _T( "error recreating edit window" ) );
   }

   detach();
   attach( hwndNew );
   DestroyWindow( hwndOld );

   SetWindowFont( m_hwnd, hfontOld, true );
   adjustSettings();
   setReadOnly( wasReadOnly );
   setText( pszText );
   setSel( nOldStart, nOldEnd );
   SetFocus( m_hwnd );
   sendMessage( EM_SCROLLCARET );
}

// end of file
