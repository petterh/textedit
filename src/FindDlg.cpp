/*
 * $Header: /Book/FindDlg.cpp 19    16.07.04 10:42 Oslph312 $
 *
 * LATER: Real MRU for search strings; include strings 
 * searched by Ctrl+F3 also.
 */


#include "precomp.h"
#include "FindDlg.h"
#include "InstanceSubclasser.h"
#include "HTML.h"
#include "resource.h"
#include "formatMessage.h"
#include "winUtils.h"
#include "persistence.h"
#include "utils.h"
#include "RedrawHider.h"


PRIVATE inline int findStringExact( 
   HWND hwndCombo, const String& strSearch ) 
{
   return findStringExact( hwndCombo, strSearch.c_str() );
}


String FindDlg::getComboContents( HWND hwndCtl, bool bSelChange ) {

   assert( isGoodPtr( this ) );
   assert( IsWindow( hwndCtl ) ); // isCombo too?

   const int nSelection = ComboBox_GetCurSel( hwndCtl );
   TCHAR szContents[ MAX_TEXT_LENGTH + 1 ] = { 0 };

   if ( bSelChange && CB_ERR != nSelection ) {
      assert( ComboBox_GetLBTextLen( 
         hwndCtl, nSelection ) < dim( szContents ) );
      ComboBox_GetLBText( hwndCtl, nSelection, szContents );
   } else {
      assert( ComboBox_GetTextLength( hwndCtl ) < dim( szContents ) );
      ComboBox_GetText( hwndCtl, szContents, dim( szContents ) );
   }

   return szContents;
}


String FindDlg::getComboContentsAndSetSelection( HWND hwndCtl ) {
   
   assert( isGoodPtr( this ) );
   assert( IsWindow( hwndCtl ) );

   String strContents;
   if ( 0 < ComboBox_GetCount( hwndCtl ) ) {
      ComboBox_SetCurSel( hwndCtl, 0 );
      strContents = getComboContents( hwndCtl, true );
   } else {
      ComboBox_SetCurSel( hwndCtl, -1 );
   }
   return strContents;
}


void FindDlg::addComboString( HWND hwndCtl, const String& str ) {

   assert( IsWindow( hwndCtl ) );

   const int nCurrentIndex = findStringExact( hwndCtl, str );
   if ( 0 <= nCurrentIndex ) {
      ComboBox_DeleteString( hwndCtl, nCurrentIndex );
   }
   ComboBox_InsertString( hwndCtl, 0, str.c_str() );
   while ( MAX_STRINGS < ComboBox_GetCount( hwndCtl ) ) {
      ComboBox_DeleteString( hwndCtl, MAX_STRINGS );
   }
   ComboBox_SetCurSel( hwndCtl, 0 );
}


void FindDlg::loadStrings( void ) {

   assert( isGoodPtr( this ) );
   HWND hwndSearchPattern = getDlgItem( IDC_SEARCHPATTERN );
   assert( IsWindow( hwndSearchPattern ) );
   int iString;
   for ( iString = 0; iString < MAX_STRINGS; ++iString ) {
      const String str = getPattern( iString );
      if ( str.empty() ) {
         break;
      }
      ComboBox_AddString( hwndSearchPattern, str.c_str() );
   }

   if ( m_isReplace ) {
      HWND hwndReplacement = getDlgItem( IDC_REPLACEMENT );
      assert( IsWindow( hwndReplacement ) );
      for ( iString = 0; iString < MAX_STRINGS; ++iString ) {
         const String str = getReplacement( iString );
         if ( str.empty() ) {
            break; //*** LOOP EXIT POINT
         }
         ComboBox_AddString( hwndReplacement, str.c_str() );
      }
      if ( 0 < iString ) {
         ComboBox_SetCurSel( hwndReplacement, 0 );
      }
   }
}


/**
 * CB_FINDSTRINGEXACT is not case sensitive, hence this function,
 * which makes sure that the we have a case sensitive match.
 * CB_FINDSTRINGEXACT wraps around, which is why we need nPrevIndex.
 */
PRIVATE int findStringExact( HWND hwndCombo, LPCTSTR pszSearch ) {

   int nPrevIndex = -1;
   int nIndex = -1;
   while ( 0 <= (nIndex = 
      ComboBox_FindStringExact( hwndCombo, nIndex, pszSearch ) ) ) 
   {
      TCHAR szString[ FindDlg::MAX_TEXT_LENGTH + 1 ] = { 0 };
      assert( ComboBox_GetLBTextLen( 
         hwndCombo, nIndex ) < dim( szString ) );
      ComboBox_GetLBText( hwndCombo, nIndex, szString );
      if ( 0 == _tcscmp( szString, pszSearch ) ) {
         break; //*** LOOP EXIT POINT
      }
      if ( nIndex <= nPrevIndex ) {
         nIndex = -1;
         break; //*** LOOP EXIT POINT
      }
      nPrevIndex = nIndex;
   }

   return nIndex;
}


void FindDlg::saveStrings( bool bSaveReplacements ) {

   assert( isGoodPtr( this ) );
   HWND hwndSearchPattern = getDlgItem( IDC_SEARCHPATTERN );
   addComboString( hwndSearchPattern, m_strSearchPattern );

   const int nSearchCount =
      __min( MAX_STRINGS, ComboBox_GetCount( hwndSearchPattern ) );
   int iString;
   for ( iString = 0; iString < nSearchCount; ++iString ) {
      TCHAR szString[ MAX_TEXT_LENGTH + 1 ] = { 0 };
      assert( ComboBox_GetLBTextLen( 
         hwndSearchPattern, iString ) < dim( szString ) );
      ComboBox_GetLBText( hwndSearchPattern, iString, szString );
      setPattern( iString, szString );
   }

   if ( bSaveReplacements ) {
      assert( m_isReplace );
      HWND hwndReplacement = getDlgItem( IDC_REPLACEMENT );
      addComboString( hwndReplacement, m_strReplacePattern );

      const int nReplaceCount =
         __min( MAX_STRINGS, ComboBox_GetCount( hwndReplacement ) );
      int nString = 0;
      for ( iString = 0; iString < nReplaceCount; ++iString ) {
         TCHAR szString[ MAX_TEXT_LENGTH + 1 ] = { 0 };
         const int nLength =
            ComboBox_GetLBTextLen( hwndReplacement, iString );
         assert( nLength < dim( szString ) );
         if ( 0 < nLength ) { // Don't want empty replacement strings.
            ComboBox_GetLBText( hwndReplacement, iString, szString );
            setReplacement( nString, szString );
            ++nString;
         }
      }
   }
}


void FindDlg::positionOutsideSelection( void ) {

   assert( isGoodPtr( this ) );
   assert( isGoodPtr( m_pEditor ) );
   AbstractEditWnd *pEditWnd = m_pEditor->getEditWnd();
   int nStart = 0;
   int nEnd = 0;
   pEditWnd->getSel( &nStart, &nEnd );

   DWORD dwStart = pEditWnd->sendMessage( EM_POSFROMCHAR, nStart );
   DWORD dwEnd   = pEditWnd->sendMessage( EM_POSFROMCHAR, nEnd   );

   RECT rcSel = { 
      LOWORD( dwStart ), HIWORD( dwStart ), 
      LOWORD( dwEnd   ), HIWORD( dwEnd   ), 
   };

   rcSel.bottom += abs( m_pEditor->getLogFont()->lfHeight );
   verify( MapWindowPoints( *pEditWnd, HWND_DESKTOP, 
      reinterpret_cast< LPPOINT >( &rcSel ), 2 ) );

   Rect rcDlg = getWindowRect( *this );
   if ( rcSel.right <= rcDlg.left || rcDlg.right <= rcSel.left ) {
      return;
   }
   if ( rcSel.bottom <= rcDlg.top || rcDlg.bottom <= rcSel.top ) {
      return;
   }

   Rect rcScreen;
   verify( SystemParametersInfo( SPI_GETWORKAREA, 0, &rcScreen, 0 ) );

   int nDeltaDown = rcSel.bottom - rcDlg.top;
   int nDeltaUp   = rcDlg.bottom - rcSel.top;

   const bool isRoomBelow = 
      rcDlg.bottom + nDeltaDown < rcScreen.bottom;
   const bool isRoomAbove = rcScreen.top < rcDlg.top + nDeltaUp;

   if ( isRoomBelow ) {
      rcDlg.top += nDeltaDown;
   } else if ( isRoomAbove ) {
      rcDlg.top -= nDeltaUp;
   } else if ( nDeltaUp < nDeltaDown ) {
      rcDlg.top += rcScreen.bottom - rcDlg.bottom;
   } else {
      rcDlg.top += rcScreen.bottom - rcDlg.bottom;
   }

   moveWindow( *this, rcDlg.left, rcDlg.top );
}


BOOL FindDlg::onInitDialog( HWND hwndFocus, LPARAM lParam ) {

   assert( isGoodPtr( this ) );
   positionOutsideSelection();
   HWND hwndSearchPattern = getDlgItem( IDC_SEARCHPATTERN );

   ComboBox_ResetContent( hwndSearchPattern );
   ComboBox_SetExtendedUI( hwndSearchPattern, true );
   ComboBox_LimitText( hwndSearchPattern, MAX_TEXT_LENGTH );

   Button_SetCheck( 
      getDlgItem( IDC_MATCH_WHOLE_WORD ), getMatchWholeWord() );
   Button_SetCheck( 
      getDlgItem( IDC_MATCH_CASE       ), getMatchCase     () );

   m_strOriginalTip = getDlgItemText( IDC_SEARCH_TIP );
   subclassHTML( getDlgItem( IDC_SEARCH_TIP ) );

   loadStrings();

   if ( m_strSearchPattern.empty() ) {
      m_strSearchPattern = 
         getComboContentsAndSetSelection( hwndSearchPattern );
   } else {
      const int nCurrentIndex = findStringExact( 
         hwndSearchPattern, m_strSearchPattern );
      if ( 0 <= nCurrentIndex ) {
         ComboBox_SetCurSel( hwndSearchPattern, nCurrentIndex ); 
      } else {
         ComboBox_SetText( 
            hwndSearchPattern, m_strSearchPattern.c_str() );
      }
   }

   if ( m_isReplace ) {
      HWND hwndReplacement = getDlgItem( IDC_REPLACEMENT );
      ComboBox_SetExtendedUI( hwndReplacement, true );
      ComboBox_LimitText( hwndReplacement, MAX_TEXT_LENGTH );

      m_strReplacePattern = 
         getComboContentsAndSetSelection( hwndReplacement );

      assert( isGoodPtr( m_pEditor ) );
      const AbstractEditWnd *pEditWnd = m_pEditor->getEditWnd();
      enableDlgItem( IDC_SELECTION, pEditWnd->getSel() );
      verify( CheckRadioButton( *this, 
         IDC_SELECTION, IDC_WHOLE_FILE, IDC_WHOLE_FILE ) );
   } else {
      toggleIcon( IDC_ARROW_UP, IDC_ARROW_DOWN, getBackwards() );
      verify( CheckRadioButton( *this, 
         IDC_UP, IDC_DOWN, getBackwards() ? IDC_UP : IDC_DOWN ) );
   }

   adjustButtons();

   return TRUE; // ...since we did NOT set the focus.
}


void FindDlg::adjustButtons( void ) {
   
   assert( isGoodPtr( this ) );
   HWND hwndFind = getDlgItem( IDC_SEARCHPATTERN );
   bool bClosed = 0 == ComboBox_GetDroppedState( hwndFind );
   if ( bClosed && m_isReplace ) {
      HWND hwndReplacement = getDlgItem( IDC_REPLACEMENT );
      bClosed = 0 == ComboBox_GetDroppedState( hwndReplacement );
   }

   HWND hwndOK = getDlgItem( IDOK );
   HWND hwndReplace = m_isReplace ? getDlgItem( IDC_REPLACE ) : 0;
   HWND hwndReplaceAll = 
      m_isReplace ? getDlgItem( IDC_REPLACE_ALL ) : 0;

   const HWND hwndFocus = GetFocus();
   if ( m_isReplace ) {
      assert( isGoodPtr( m_pEditor ) );
      const AbstractEditWnd *pEditWnd = m_pEditor->getEditWnd();
      const bool hasSelection = pEditWnd->getSel();
      if ( !hasSelection ) {
         m_bReplaceInSelection = false;
         verify( CheckRadioButton( *this, 
            IDC_SELECTION, IDC_WHOLE_FILE, IDC_WHOLE_FILE ) );
         if ( hwndFocus == getDlgItem( IDC_SELECTION ) ) {
            gotoDlgItem( IDC_WHOLE_FILE );
         }
      }
      enableDlgItem( IDC_SELECTION, hasSelection );
   }

   if ( m_bReplaceInSelection ) {
      if ( hwndFocus == hwndOK || hwndFocus == hwndReplace ) {
         gotoDlgItem( IDC_REPLACE_ALL );
      }
      EnableWindow( hwndOK, false );
      EnableWindow( hwndReplace, false );
      EnableWindow( hwndReplaceAll, true );
      sendMessage( DM_SETDEFID, IDC_REPLACE_ALL );
      setButtonStyle( hwndReplaceAll, 
         bClosed ? BS_DEFPUSHBUTTON : BS_PUSHBUTTON );
      return; //*** FUNCTION EXIT POINT
   }

   const bool canSearch = !m_strSearchPattern.empty();
   bool canReplace = m_isReplace && canSearch;
   if ( m_isReplace ) {
      String strSelection;
      assert( isGoodPtr( m_pEditor ) );
      const AbstractEditWnd *pEditWnd = m_pEditor->getEditWnd();
      pEditWnd->getSel( &strSelection );
      bool bSearchPatternMatchesSelection = false;
      if ( getMatchCase() ) {
         bSearchPatternMatchesSelection = 
            0 == m_strSearchPattern.compare( strSelection );
      } else {
         bSearchPatternMatchesSelection = 0 == _tcsicmp( 
            m_strSearchPattern.c_str(), strSelection.c_str() ); 
      }
      if ( !bSearchPatternMatchesSelection ) {
         canReplace = false;
      }
   }

   if ( m_isReplace ) {
      EnableWindow( hwndReplace, canReplace );
      EnableWindow( hwndReplaceAll, canSearch );
      sendMessage( DM_SETDEFID, canReplace ? IDC_REPLACE : IDOK );
      setButtonStyle( hwndReplace, 
         canReplace && bClosed ? BS_DEFPUSHBUTTON : BS_PUSHBUTTON );
   }

   EnableWindow( hwndOK, canSearch );
   setButtonStyle( hwndOK, canSearch && !canReplace && bClosed 
      ? BS_DEFPUSHBUTTON : BS_PUSHBUTTON );
}


void FindDlg::onComboChanged( HWND hwndCtl, UINT codeNotify ) {

   assert( isGoodPtr( this ) );
   assert( IsWindow( hwndCtl ) );

   switch ( codeNotify ) {
   case CBN_EDITCHANGE:
   case CBN_SELCHANGE: 
      {
         const int nSelection = ComboBox_GetCurSel( hwndCtl );
         const String strContents = 
            getComboContents( hwndCtl, CBN_SELCHANGE == codeNotify );
         if ( getDlgItem( IDC_REPLACEMENT ) == hwndCtl ) {
            m_strReplacePattern = strContents;
         } else if ( getDlgItem( IDC_SEARCHPATTERN ) == hwndCtl ) {
            m_strSearchPattern = strContents;
         } else {
            assert( false ); // Should never get here.
         }
         trace( _T( "%s: %d %s\n" ), 
            CBN_SELCHANGE == codeNotify 
               ? _T( "CBN_SELCHANGE" ) : _T( "CBN_EDITCHANGE" ),
            nSelection, strContents.c_str() );
      }
      adjustButtons();
      break;

   case CBN_CLOSEUP:
      {
         const int nSelection = ComboBox_GetCurSel( hwndCtl );
         const String strContents = 
            getComboContents( hwndCtl, CBN_SELCHANGE == codeNotify );
         trace( _T( "CBN_CLOSEUP: %d %s\n" ), 
            nSelection, strContents.c_str() );
      }
      break;

   case CBN_SELENDOK:
      {
         const int nSelection = ComboBox_GetCurSel( hwndCtl );
         const String strContents = 
            getComboContents( hwndCtl, CBN_SELCHANGE == codeNotify );
         trace( _T( "CBNCBN_SELENDOK: %d %s\n" ), 
            nSelection, strContents.c_str() );
      }
      break;

   case CBN_SELENDCANCEL:
      {
         const int nSelection = ComboBox_GetCurSel( hwndCtl );
         const String strContents = 
            getComboContents( hwndCtl, CBN_SELCHANGE == codeNotify );
         trace( _T( "CBN_SELENDCANCEL: %d %s\n" ), 
            nSelection, strContents.c_str() );
      }
      break;

   default:
      {
         const int nSelection = ComboBox_GetCurSel( hwndCtl );
         const String strContents = 
            getComboContents( hwndCtl, CBN_SELCHANGE == codeNotify );
         trace( _T( "[%d]: %d %s\n" ), 
            codeNotify, nSelection, strContents.c_str() );
      }
   }

   // NOTE: Without this, we have flashing!
   if ( 0 != m_strOriginalTip.compare( 
      getDlgItemText( IDC_SEARCH_TIP ) ) ) 
   {
      setDlgItemText( IDC_SEARCH_TIP, m_strOriginalTip );
      removeHighlight( getDlgItem( IDC_SEARCH_TIP ) );
   }
}


void FindDlg::onFindNext( void ) {
   
   assert( isGoodPtr( this ) );
   m_uiRetCode = IDOK;
   saveStrings( false );
   setMatchWholeWord( 
      0 != Button_GetCheck( getDlgItem( IDC_MATCH_WHOLE_WORD ) ) );
   setMatchCase     ( 
      0 != Button_GetCheck( getDlgItem( IDC_MATCH_CASE       ) ) );
   if ( !m_isReplace ) {
      setBackwards( 0 != Button_GetCheck( getDlgItem( IDC_UP ) ) );
   }

   assert( isGoodPtr( m_pEditor ) );
   if ( !m_pEditor->searchAndSelect( m_strSearchPattern ) ) {
      // 1. Do things that will result in 
      //    IDC_SEARCHPATTERN notifications...
      gotoDlgItem( IDC_SEARCHPATTERN );
      SNDMSG( getDlgItem( IDC_SEARCHPATTERN ), EM_SETSEL, 0, -1 );
      
      // ...and 2. Set the error stuff. Order is important!
      setHighlight( getDlgItem( IDC_SEARCH_TIP ) );
      const String strMessage = formatMessage( 
         IDS_STRING_NOT_FOUND, m_strSearchPattern.c_str() );
      setDlgItemText( IDC_SEARCH_TIP, strMessage );
      MessageBeep( MB_ICONWARNING );
   } else if ( m_isReplace ) {
      positionOutsideSelection();
      adjustButtons();
   } else {
      verify( EndDialog( *this, m_uiRetCode ) );
   }
}


void FindDlg::onReplace( void ) {

   assert( isGoodPtr( this ) );
   m_uiRetCode = IDOK;
   saveStrings( true );
   assert( isGoodPtr( m_pEditor ) );
   AbstractEditWnd *pEditWnd = m_pEditor->getEditWnd();
   assert( pEditWnd->getSel() );
   pEditWnd->replaceSel( m_strReplacePattern.c_str() );
   setDlgItemText( IDCANCEL, loadString( IDS_CLOSE ) );
   if ( m_pEditor->searchAndSelect( m_strSearchPattern ) ) {
      positionOutsideSelection();
   } else {
      // 1. Do things that will result in 
      //    IDC_SEARCHPATTERN notifications...
      gotoDlgItem( IDC_SEARCHPATTERN );
      SNDMSG( getDlgItem( IDC_SEARCHPATTERN ), EM_SETSEL, 0, -1 );
      
      // ...and 2. Set the error stuff.
      setHighlight( getDlgItem( IDC_SEARCH_TIP ) );
      const String strMessage = formatMessage( 
         IDS_STRING_NOT_FOUND, m_strSearchPattern.c_str() );
      setDlgItemText( IDC_SEARCH_TIP, strMessage );
      MessageBeep( MB_ICONWARNING );
   }
   adjustButtons();
}


/**
 * QUESTION: Disable everything for the duration?
 * Would really require active Cancel button.
 */
void FindDlg::onReplaceAll( void ) {

   assert( isGoodPtr( this ) );
   removeHighlight( getDlgItem( IDC_SEARCH_TIP ) );
   saveStrings( true );
   int nReplacements = 0;
   assert( isGoodPtr( m_pEditor ) );
   AbstractEditWnd *pEditWnd = m_pEditor->getEditWnd();
   RedrawHider rh( *pEditWnd );
   if ( m_bReplaceInSelection ) {
      assert( pEditWnd->getSel() );
      nReplacements = pEditWnd->replaceInSelection( 
         m_strSearchPattern, m_strReplacePattern,
         getMatchWholeWord(), getMatchCase() );
   } else {
      int nStart = 0;
      pEditWnd->getSel( &nStart );
      pEditWnd->setSel( 0, 0 );
      bool bWrap = false;
      while ( m_pEditor->searchAndSelect( 
         m_strSearchPattern, &bWrap ) ) 
      {
         if ( bWrap ) {
            break;
         }
         assert( pEditWnd->getSel() );
         pEditWnd->replaceSel( m_strReplacePattern.c_str() );
         ++nReplacements;
         if ( GetAsyncKeyState( VK_ESCAPE ) < 0 ) {
            break; // In case of endless loop
         }
      }
      if ( 0 == nReplacements ) {
         pEditWnd->setSel( nStart, nStart );
      }
   }

   const String strMessage = formatMessage( 
      1 == nReplacements ? IDS_REPLACEMENTS1 : IDS_REPLACEMENTS,
      nReplacements );
   setDlgItemText( IDC_SEARCH_TIP, strMessage );
   setDlgItemText( IDCANCEL, loadString( IDS_CLOSE ) );
   adjustButtons();
   sendMessage( DM_SETDEFID, IDCANCEL );
}


void FindDlg::onDlgCommand( int id, HWND hwndCtl, UINT codeNotify ) {

   assert( isGoodPtr( this ) );
   switch ( id ) {
   case IDC_DOWN:
   case IDC_UP:
      assert( !m_isReplace );
      if ( BN_CLICKED == codeNotify ) {
         const bool isBackwards = 
            0 != Button_GetCheck( getDlgItem( IDC_UP ) );
         toggleIcon( IDC_ARROW_UP, IDC_ARROW_DOWN, isBackwards );
      }
      break;

   case IDOK:
      if ( BN_CLICKED == codeNotify ) {
         onFindNext();
      }
      break;

   case IDCANCEL:
      if ( BN_CLICKED == codeNotify ) {
         verify( EndDialog( *this, m_uiRetCode ) );
      }
      break;

   case IDC_REPLACE:
      if ( BN_CLICKED == codeNotify ) {
         onReplace();
      }
      break;

   case IDC_REPLACE_ALL:
      if ( BN_CLICKED == codeNotify ) {
         onReplaceAll();
      }
      break;

   case IDC_SEARCHPATTERN:
   case IDC_REPLACEMENT:
      onComboChanged( hwndCtl, codeNotify );
      break;

   case IDC_MATCH_WHOLE_WORD:
   case IDC_MATCH_CASE:
      setMatchWholeWord( 
         0 != Button_GetCheck( getDlgItem( IDC_MATCH_WHOLE_WORD ) ) );
      setMatchCase     ( 
         0 != Button_GetCheck( getDlgItem( IDC_MATCH_CASE       ) ) );
      if ( BN_CLICKED == codeNotify ) {
         adjustButtons();
      }
      break;

   case IDC_SELECTION:
   case IDC_WHOLE_FILE:
      m_bReplaceInSelection = IDC_SELECTION == id;
      adjustButtons();
   }
}


UINT FindDlg::getResourceID( void ) const {

   return m_isReplace ? IDD_EDITREPLACE : IDD_EDITFIND;
}


FindDlg::FindDlg( 
   Editor *pEditor, const String &strSearchPattern, bool isReplace )

   : m_pEditor( pEditor )
   , m_isReplace( isReplace )
   , m_uiRetCode( IDCANCEL )
   , m_bReplaceInSelection( false )
{
   assert( isGoodPtr( this ) );
   assert( isGoodPtr( m_pEditor ) );

   const int nNewLine = 
      strSearchPattern.find_first_of( _T( "\r\n" ) );
   if ( 0 <= nNewLine ) {
      m_strSearchPattern = strSearchPattern.substr( 0, nNewLine );
   } else {
      m_strSearchPattern = strSearchPattern;
   }
   if ( MAX_TEXT_LENGTH < m_strSearchPattern.length() ) {
      m_strSearchPattern = 
         m_strSearchPattern.substr( 0, MAX_TEXT_LENGTH );
   }
}

// end of file
