/*
 * $Header: /Book/Editor.cpp 28    16.07.04 10:42 Oslph312 $
 */

#include "precomp.h"
#include "Editor.h"
#include "AutoArray.h"
#include "Registry.h"
#include "Exception.h"
#include "WaitCursor.h"
#include "FontDlg.h"
#include "MRU.h"
#include "formatMessage.h"
#include "mainwnd.h"
#include "fileUtils.h"
#include "menuUtils.h"
#include "winUtils.h"
#include "activateOldInstance.h"
#include "createNewFile.h"
#include "saveFile.h"
#include "utils.h"
#include "resource.h"
#include "persistence.h"
#include "timers.h"
#include <richedit.h>


LPTSTR Editor::getContents( void ) {

   assert( isGoodPtr( this ) );
   LPTSTR pszOrgText = m_pDocument->getContents();
   assert( isGoodStringPtr( pszOrgText ) );
   m_pDocument->addCRs( &pszOrgText );
   return pszOrgText;
}


void Editor::setDocument( Document *pNewDocument ) {

   assert( isGoodPtr( this ) );
   KillTimer( m_hwndMain, timer_save_document );
   SetFocus( m_hwndMain ); // In case we're on the tool bar.

   assert( isGoodPtr( this ) );
   assert( isGoodPtr( m_pDocument ) );
   Document *pOldDocument = m_pDocument;

   try {
      assert( isGoodPtr( pNewDocument ) );
      m_pDocument = pNewDocument;
      MRU().addFile( m_pDocument->getPath() );
      const AutoString pszOrgText( getContents() );
      assert( isGoodPtr( m_pEditWnd ) );
      m_pEditWnd->resetPosition();
      m_pEditWnd->setText( pszOrgText );
   }
   catch ( ... ) {
      m_pDocument = pOldDocument;
      throw;
   }

   delete pOldDocument;
   m_isClean = true;
   m_isWhistleClean = true;
}


void Editor::setReadOnly( bool bReadOnly ) {

   assert( isGoodPtr( this ) );
   if ( getEditWnd()->isReadOnly() != bReadOnly ) {
      m_isWhistleClean = false;
   }

   if ( getEditWnd()->isReadOnly() ) {
      assert( isClean() );
   } else if ( !isClean() ) {
      if ( !save() ) {
         throwException();
      }
   }
   if ( !m_pDocument->modifyAttribs( 
      bReadOnly ? FILE_ATTRIBUTE_READONLY : 0,
      bReadOnly ? 0 : FILE_ATTRIBUTE_READONLY ) )
   {
      throwException();
   }
   m_pEditWnd->setReadOnly( bReadOnly );
   assert( !m_pDocument->isAccessDenied() );
   m_pToolbar->setReadOnly( bReadOnly );
   updateToolbar();
}


bool Editor::getReadOnly( void ) const {
   return m_pToolbar->getReadOnly();
}


bool Editor::save( void ) throw() {

   static bool s_bBusy = false;
   if ( s_bBusy ) {
      debugBreak();
      return false;
   }
   s_bBusy = true;

   assert( isGoodPtr( this ) );
   KillTimer( m_hwndMain, timer_save_document );
   TemporaryStatusIcon statusIcon( m_pStatusbar, STD_FILESAVE );

   bool bSaved = false;
   try {
      assert( isGoodPtr( m_pEditWnd ) );
      // NOTE: nLength may be larger than the actual text!
      const int nLength = m_pEditWnd->getTextLength();
      AutoString pszContents( new TCHAR[ nLength + 1 ] ); 
      m_pEditWnd->getText( pszContents, nLength + 1 );
      pszContents[ nLength ] = 0;
      m_pDocument->update( 
         GetLastActivePopup( m_hwndMain ), pszContents, nLength );
      setTitle();
      m_pStatusbar->setFileType( m_pDocument->isUnicode() );
      bSaved = true;
   }
   catch ( ... ) {
      messageBox( GetLastActivePopup( m_hwndMain ), 
         MB_ICONERROR | MB_OK, IDS_SAVE_EXCEPTION );
   }

   if ( bSaved ) {
      trace( _T( "Saved %s successfully\n" ), 
         m_pDocument->getPath().c_str() );
      m_pDocument->clean();
      m_pEditWnd->clean( /* bEmptyUndo = */ false );
      m_isClean = true;
      m_isWhistleClean = false; // Time stamp will have changed
   } else {
      trace( _T( "Unable to save %s\n" ), 
         m_pDocument->getPath().c_str() );
   }

   s_bBusy = false;

   return bSaved;
}


bool Editor::saveIfNecessary( void ) {
   
   assert( isGoodPtr( this ) );
   if ( !isClean() ) {
      save();
   }
   return isClean();
}


void CALLBACK Editor::autoSaveTimerProc(
   HWND hwnd, UINT msg, UINT id, DWORD dwTime ) 
{
   assert( timer_save_document == id );
   KillTimer( hwnd, id );

   Editor *pEditor = getEditor( hwnd );
   assert( isGoodPtr( pEditor ) );
   assert( hwnd == pEditor->m_hwndMain );
   pEditor->saveIfNecessary();
}


void Editor::restoreOriginal( void ) {

   assert( isGoodPtr( this ) );
   KillTimer( m_hwndMain, timer_save_document );
   TemporaryStatusIcon statusIcon( m_pStatusbar, STD_UNDO );

   int nBytes = 0;
   AutoString pszOrgText( m_pDocument->getOrgContents( &nBytes ) );
   assert( isGoodStringPtr( pszOrgText ) );
   if ( (LPTSTR) 0 == pszOrgText ) {
      throwException( _T( "m_pDocument->getOrgCopy failed!" ) );
   }

   // TODO: doc::restore, specially if I move whistleclean.
   m_pDocument->save( 
      GetLastActivePopup( m_hwndMain ), pszOrgText, nBytes );
   m_pDocument->addCRs( &pszOrgText );

   assert( isGoodPtr( m_pEditWnd ) );
   m_pEditWnd->resetPosition();

#if 0 // If we do this, we loose the ability to undo!
   m_pEditWnd->setText( pszOrgText );
#else // ...so we do this instead:
   m_pEditWnd->setSel( 0, -1 );
   m_pEditWnd->replaceSel( pszOrgText );
#endif

   m_pEditWnd->setSel( 0, 0 ); // LATER: Try to retain selection?
   m_pEditWnd->sendMessage( EM_SCROLLCARET );

   m_isClean = true;
   m_isWhistleClean = true;
}


bool Editor::searchAndSelect( 
   const String& strSearchPattern, bool *pbWrapped ) 
{
   assert( isGoodPtr( this ) );

   WaitCursor waitCursor( _T( "wrdcount.ani" ) );
   const HIMAGELIST hImageList = reinterpret_cast< HIMAGELIST >(
      m_pToolbar->sendMessage( TB_GETIMAGELIST ) );
   TemporaryStatusIcon statusIcon( 
      m_pStatusbar, hImageList, find_icon );

   assert( isGoodPtr( m_pEditWnd ) );
   int nOldStart = 0;
   int nOldEnd = 0;
   m_pEditWnd->getSel( &nOldStart, &nOldEnd );

   const bool bFound = m_pEditWnd->searchAndSelect( strSearchPattern,
      getMatchWholeWord(), getMatchCase(), getBackwards() ? -1 : 1 );
   
   if ( bFound ) {
      int nNewStart = 0;
      int nNewEnd = 0;
      m_pEditWnd->getSel( &nNewStart, &nNewEnd );
      
      bool bWrapAround = false;
      if ( getBackwards() ) {
         bWrapAround = 
            nOldStart < nNewStart ? true :
            nNewStart < nOldStart ? false :
            nOldEnd == nNewEnd; // In case nNewStart == nOldStart.
      } else {
         bWrapAround = 
            nNewStart < nOldStart ? true :
            nOldStart < nNewStart ? false :
            nOldEnd == nNewEnd; // In case nNewStart == nOldStart.
      }
         
      if ( bWrapAround ) {
         m_pStatusbar->setHighlightMessage( 
            getBackwards() 
               ? IDS_PASSED_START_OF_FILE : IDS_PASSED_END_OF_FILE,
            strSearchPattern.c_str() );
      } else {
         m_pStatusbar->setMessage( 
            IDS_STRING_FOUND, strSearchPattern.c_str() );
      }

      if ( 0 != pbWrapped ) {
         assert( isGoodPtr( pbWrapped ) );
         *pbWrapped = bWrapAround;
      }
   }

   return bFound;
}


int Editor::getAutoSaveTime( void ) const {
   
   assertValid();
   return m_pDocument->isFloppy() ? 600000 : 60000;
}


void Editor::saveState( ) {

   assert( isGoodPtr( this ) );
   assert( IsWindow( m_hwndMain ) );

   // Get window position and min/max state:
   WINDOWPLACEMENT wpl = { sizeof wpl };
   assert( 0 == offsetof( WINDOWPLACEMENT, length ) );
   if ( !GetWindowPlacement( m_hwndMain, &wpl ) ) {
      GetWindowRect( m_hwndMain, &wpl.rcNormalPosition );
      wpl.showCmd = IsZoomed( m_hwndMain ) ? SW_SHOWMAXIMIZED : SW_NORMAL;
   }

   // TODO: Window placement is screen relative, not workspace relative.
   // The window sizing functions are workspace relative...

   // Save window position and state for document:
   assert( isGoodPtr( m_pDocument ) );
   m_pDocument->setLeft  ( wpl.rcNormalPosition.left );
   m_pDocument->setTop   ( wpl.rcNormalPosition.top  );
   m_pDocument->setWidth ( 
      wpl.rcNormalPosition.right  - wpl.rcNormalPosition.left );
   m_pDocument->setHeight( 
      wpl.rcNormalPosition.bottom - wpl.rcNormalPosition.top  );
   m_pDocument->setWindowState( wpl.showCmd );

   // Save edit window state
   assert( isGoodPtr( m_pEditWnd ) );
   m_pDocument->setFirstLine( m_pEditWnd->getFirstVisibleLine() );
   int nSelStart = 0;
   int nSelEnd = 0;
   m_pEditWnd->getSel( &nSelStart, &nSelEnd );
   m_pDocument->setSelStart ( nSelStart );
   m_pDocument->setSelEnd   ( nSelEnd );
}


// Note: activateOldInstance will activate us, 
// too, if we're reloading the same file!
void Editor::openFile( const String& strPath ) {

   assert( isGoodPtr( this ) );
   TemporaryStatusIcon statusIcon( m_pStatusbar, STD_FILEOPEN );
   if ( !activateOldInstance( strPath.c_str() ) ) {
      WaitCursor waitCursor( _T( "load.ani" ) );
      m_pStatusbar->setMessage( IDS_LOADING, strPath.c_str() );
      Document *pNewDcument = new Document( m_hwndMain, strPath.c_str() );
      setDocument( pNewDcument );
      setSettings();
      updateToolbar();
      m_pStatusbar->update( m_pEditWnd->getCurPos() );
   }
}


// TODO -- too similar to openFile; extract "load" part
void Editor::reload() {
		
	assert( isGoodPtr( this ) );
	TemporaryStatusIcon statusIcon( m_pStatusbar, STD_FILEOPEN );
    WaitCursor waitCursor( _T( "load.ani" ) );
	const String strPath = m_pDocument->getPath();
    m_pStatusbar->setMessage( IDS_LOADING, strPath.c_str() );
    Document *pNewDcument = new Document( m_hwndMain, strPath.c_str() );
    setDocument( pNewDcument );
    setSettings();
    updateToolbar();
    m_pStatusbar->update( m_pEditWnd->getCurPos() );
}

// TODO: Unit test new safe string API
void Editor::copyFile( void ) {
   assert( isGoodPtr( this ) );
   String strPath = m_pDocument->getPath();
   const int nEnd = strPath.find_last_of( _T( "\\" ) );
   assert( 0 < nEnd );
   strPath.erase( nEnd );
   String strNewPath = createNewFile( m_hwndMain, file_for_copy, 0, strPath, m_pDocument->getTitle() );
   while ( saveFile( GetLastActivePopup( m_hwndMain ), &strNewPath, IDS_COPY_FILE, IDD_COPY_CHILD ) )
   {
      if ( strNewPath.empty() ) {
         continue;
      }

      TCHAR szOldPath[ MAX_PATH + 2 ] = { 0 };
	  LPCTSTR oldString = m_pDocument->getPath().c_str();
	  _tcscpy_s( szOldPath, std::min( dim( szOldPath ), _tcslen( oldString ) + 1 ), oldString );
      TCHAR szNewPath[ MAX_PATH + 2 ] = { 0 };
	  LPCTSTR newString = strNewPath.c_str();
	  _tcscpy_s( szNewPath, std::min( dim (szNewPath ), _tcslen( newString ) + 1 ), newString );
      SHFILEOPSTRUCT shFileOpStruct = {
         m_hwndMain, FO_COPY, szOldPath, szNewPath, 
         FOF_NOCONFIRMATION | FOF_SIMPLEPROGRESS | FOF_MULTIDESTFILES,
      };

      WaitCursor waitCursor( _T( "save.ani" ) );
      const int nErr = SHFileOperation( &shFileOpStruct );
      if ( 0 == nErr ) {
         openFile( strNewPath );
         break; //*** SUCCESS LOOP EXIT
      }
   }
}


void Editor::refreshToolbar( void ) {

   assert( isGoodPtr( this ) );
   assert( isGoodPtr( m_pEditWnd ) );
   m_pToolbar.reset( new Toolbar( this, IDC_TOOLBAR,
      m_pEditWnd->hasRedo(), m_pEditWnd->canSetTabs() ) );
}

// TODO: Unit test new safe string API
PRIVATE void initFont( LOGFONT *pLogFont, HFONT *phfont,
   const String& strFace, long lHeight, long lWeight,
   unsigned char cItalic, unsigned char cCharSet )
{
   memset( pLogFont, 0, sizeof *pLogFont );
   _tcscpy_s( pLogFont->lfFaceName, strFace.c_str() );
   pLogFont->lfHeight = devPointsFromPrinterPoints( lHeight );
   pLogFont->lfWeight = lWeight;
   pLogFont->lfItalic = cItalic;
   pLogFont->lfCharSet = cCharSet;
   *phfont = CreateFontIndirect( pLogFont );
   verify( GetObject( *phfont, sizeof( *pLogFont ), pLogFont ) );
}


/**
 * Constructs the one and only Editor object, our link 
 * to the edit window. This constructor constructs 
 * the actual edit window.
 */
Editor::Editor( HWND hwndParent, AutoDocument *ppDocument )
   : m_hwndMain( hwndParent )
   , m_haccel( 0 )
   , m_pToolbar( 0 )
   , m_pStatusbar( 0 )
   , m_hasFixedFont( true )
   , m_hfontProportional( 0 )
   , m_hfontFixed( 0 )
   , m_isClean( true )
   , m_isWhistleClean( true )
{
   assert( isGoodPtr( this ) );
   assert( isGoodPtr( ppDocument ) );

   // Do this carefully, as we're transferring ownership of a pointer
   // that's already protected by autoprotection (in init.cpp).
   m_pDocument = *ppDocument;
   *ppDocument = 0;

   const AutoString pszOrgText( getContents() ); 
   MRU().addFile( m_pDocument->getPath() );

   LoadLibrary( _T( "RICHED20.DLL" ) );

   m_pEditWnd = AbstractEditWnd::create( 
      hwndParent, pszOrgText, this, 0 != m_pDocument->getWordWrap() );
   assert( isGoodPtr( m_pEditWnd ) );

   refreshToolbar();
   assert( isGoodPtr( m_pToolbar ) );
   assert( IsWindow( *m_pToolbar ) );
   if ( !getToolbarVisible() ) {
      ShowWindow( *m_pToolbar, SW_HIDE );
   }
   
   m_pStatusbar = new Statusbar( hwndParent, IDC_STATUSBAR );
   assert( isGoodPtr( m_pStatusbar ) );
   assert( IsWindow( *m_pStatusbar ) );
   if ( !getStatusbarVisible() ) {
      ShowWindow( *m_pStatusbar, SW_HIDE );
   }

   initFont( &m_logFontFixed, &m_hfontFixed,
      getFixedFace   (),
      getFixedHeight (),
      getFixedWeight (),
      getFixedItalic (),
      getFixedCharSet() );

   initFont( &m_logFontProportional, &m_hfontProportional,
      getProportionalFace   (),
      getProportionalHeight (),
      getProportionalWeight (),
      getProportionalItalic (),
      getProportionalCharSet() );
}


static bool is_alive = true;
Editor::~Editor() {
   
   if ( 0 != m_hfontProportional ) {
      verify( DeleteFont( m_hfontProportional ) );
      m_hfontProportional = 0;
   }
   if ( 0 != m_hfontFixed ) {
      verify( DeleteFont( m_hfontFixed ) );
      m_hfontFixed = 0;
   }

   assert( 0 != m_haccel );
   verify( DestroyAcceleratorTable( m_haccel ) );
   is_alive = false;

   FreeLibrary( GetModuleHandle( _T( "RICHED20.DLL" ) ) );
}


HFONT Editor::getFont( bool bFixedFont ) const {

   assert( isGoodConstPtr( this ) );

   HFONT hfont = bFixedFont ? m_hfontFixed : m_hfontProportional;
   if ( 0 == hfont ) {
      trace( _T( "Editor::getFont: hfont is null\n" ) );
      hfont = GetStockFont( 
         hasFixedFont() ? ANSI_FIXED_FONT : DEFAULT_GUI_FONT );
   }
   return hfont;
}


HFONT Editor::getFont( void ) const {
   
   assert( isGoodConstPtr( this ) );
   return getFont( hasFixedFont() );
}


void Editor::setTitle( void ) {

   assert( isGoodPtr( this ) );

   const String strTitle = m_pDocument->getTitle();
   String strWindowTitle = 
      formatMessage( IDS_APP_TITLE, strTitle.c_str() ); 

#ifdef _DEBUG
   if ( AbstractEditWnd::bForceEdit ) {
      strWindowTitle += _T( " [Debug - EDIT]" ); 
   } else {
      strWindowTitle += _T( " [Debug - " ) RICHEDIT_CLASS _T( "]" ); 
   }
#endif

   assert( IsWindow( m_hwndMain ) );
   SetWindowText( m_hwndMain, strWindowTitle.c_str() );

   // Change the large and small icons to match the file type:

   SHFILEINFO fileInfo = { 0 };
   SHGetFileInfo( m_pDocument->getPath().c_str(), 0, 
      &fileInfo, sizeof fileInfo, SHGFI_ICON | SHGFI_SMALLICON );
   if ( 0 != fileInfo.hIcon ) {
      SNDMSG( m_hwndMain, WM_SETICON, ICON_SMALL, 
         reinterpret_cast< LPARAM >( fileInfo.hIcon ) );
   }
   SHGetFileInfo( m_pDocument->getPath().c_str(), 0, 
      &fileInfo, sizeof fileInfo, SHGFI_ICON );
   if ( 0 != fileInfo.hIcon ) {
      SNDMSG( m_hwndMain, WM_SETICON, ICON_BIG, 
         reinterpret_cast< LPARAM >( fileInfo.hIcon ) );
   }
}


void Editor::setWordWrap( bool bOn ) {

   assert( isGoodPtr( m_pEditWnd ) );
   m_pEditWnd->setWordWrap( bOn );
   InvalidateRect( *m_pEditWnd, 0, FALSE );
   m_pDocument->setWordWrap( bOn );

   HMENU hmenu = GetMenu( m_hwndMain );
   assert( IsMenu( hmenu ) );
   checkMenuItem( hmenu, ID_VIEW_WORDWRAP, bOn );
   m_pToolbar->check(    ID_VIEW_WORDWRAP, bOn );
}


void Editor::setSettings( void ) {

   setTitle();

   const int nFixedFont = m_pDocument->getFixedFont();
   setFont( 0 != nFixedFont );
   setWordWrap( 0 != m_pDocument->getWordWrap() );

   const bool bReadOnly = m_pDocument->isReadOnly();
   assert( isGoodPtr( m_pEditWnd ) );
   m_pEditWnd->setReadOnly(
      bReadOnly || m_pDocument->isAccessDenied() );
   m_pToolbar->setReadOnly( 
      bReadOnly, m_pDocument->isAccessDenied() );

   if ( m_pEditWnd->canSetTabs() ) {
      const int nTabs = m_pDocument->getTabs();
      m_pEditWnd->setSpacesPerTab( nTabs );
      m_pToolbar->setSpacesPerTab( nTabs );
   }

   m_pStatusbar->setFileType( m_pDocument->isUnicode() );
   saveState();
}


void Editor::setFont( bool isFixedFont ) {

   m_pDocument->setFixedFont( isFixedFont ? 1 : 0 );
   m_hasFixedFont = isFixedFont;

   SetWindowFont( *m_pEditWnd, getFont(), true );
   UpdateWindow( *m_pEditWnd );
   m_pEditWnd->sendMessage( EM_REQUESTRESIZE );

   HMENU hmenu = GetMenu( m_hwndMain );
   assert( IsMenu( hmenu ) );
   checkMenuRadioItem( hmenu, 
      ID_VIEW_FIXEDFONT, ID_VIEW_PROPORTIONALFONT, 
      0 == m_pDocument->getFixedFont() 
      ? ID_VIEW_PROPORTIONALFONT : ID_VIEW_FIXEDFONT );

   m_pToolbar->check( ID_VIEW_FIXEDFONT       ,  isFixedFont );
   m_pToolbar->check( ID_VIEW_PROPORTIONALFONT, !isFixedFont );
}


void Editor::setLogFont( const LOGFONT *pLogFont, bool bFixed ) {

   assert( isGoodPtr( this ) );
   assert( isGoodConstPtr( pLogFont ) );

   if ( bFixed ) {
      if ( 0 != m_hfontFixed ) {
         DeleteFont( m_hfontFixed );
         m_hfontFixed = 0;
      }
      m_logFontFixed = *pLogFont;
      m_hfontFixed = CreateFontIndirect( &m_logFontFixed );

      setFixedFace   ( m_logFontFixed.lfFaceName );
      setFixedHeight ( 
         printerPointsFromDevPoints( m_logFontFixed.lfHeight ) );
      setFixedWeight ( m_logFontFixed.lfWeight   );
      setFixedItalic ( m_logFontFixed.lfItalic   );
      setFixedCharSet( m_logFontFixed.lfCharSet  );
   } else {
      if ( 0 != m_hfontProportional ) {
         DeleteFont( m_hfontProportional );
         m_hfontProportional = 0;
      }
      m_logFontProportional = *pLogFont;
      m_hfontProportional = 
         CreateFontIndirect( &m_logFontProportional );

      setProportionalFace   ( m_logFontProportional.lfFaceName );
      setProportionalHeight ( printerPointsFromDevPoints( 
         m_logFontProportional.lfHeight ) );
      setProportionalWeight ( m_logFontProportional.lfWeight   );
      setProportionalItalic ( m_logFontProportional.lfItalic   );
      setProportionalCharSet( m_logFontProportional.lfCharSet  );
   }

   if ( m_hasFixedFont == bFixed ) {
      setFont( bFixed );
   }
}


/**
 * If you use cascading submenus, start with the innermost menus.
 */
String Editor::getMenuDescription( HMENU hmnuPopup ) const {

   assert( IsMenu( hmnuPopup ) );

   int nStringID = 0;
   if ( containsItem( hmnuPopup, ID_FILE_NEW ) ) {
      nStringID = IDS_FILE_MENU;
   } else if ( containsItem( hmnuPopup, ID_EDIT_UNDO ) ) {
      nStringID = IDS_EDIT_MENU;
   } else if ( containsItem( hmnuPopup, ID_EDIT_FIND ) ) {
      nStringID = IDS_FIND_MENU;
   } else if ( containsItem( hmnuPopup, ID_VIEW_TOOLBAR ) ) {
      nStringID = IDS_VIEW_MENU;
   } else if ( containsItem( hmnuPopup, ID_HELP_CONTENTS ) ) {
      nStringID = IDS_HELP_MENU;
   } else if ( containsItem( hmnuPopup, ID_FILE_SENDTO_MAILRECIPIENT ) ) {
      nStringID = IDS_FILE_SENDTO_MENU;
   } else if ( containsItem( hmnuPopup, SC_RESTORE ) ) {
      nStringID = IDS_SYSTEM_MENU;
   }

   return loadString( nStringID );
}


String Editor::getMenuItemDescription( 
   int nItem, const String& strLast ) const 
{
   String strArg;

   switch ( nItem ) {
   case SC_CLOSE:
      nItem = ID_FILE_CLOSE;

      //*** FALL THROUGH

   case ID_FILE_DELETE:
   case ID_FILE_PAGESETUP:
   case ID_FILE_PRINT:
   case ID_FILE_PROPERTIES:
   case ID_FILE_ABANDONCHANGES:
   case ID_FILE_CLOSE:
   case ID_FILE_COPY:
   case ID_FILE_RENAME:
      strArg = m_pDocument->getPath();
      break;

#if 0
   case ID_VIEW_WORDWRAP:
      if ( 0 != m_pDocument->getWordWrap() ) {
         nItem = ID_VIEW_WORDWRAPOFF;
      } else {
         nItem = ID_VIEW_WORDWRAPON;
      }
      break;
#endif

   case ID_EDIT_UNDO:
      {
         String strAction = m_pEditWnd->getUndoName();
         if ( !strAction.empty() ) {
            strArg = strAction;
            nItem = ID_EDIT_UNDO_ARG;
         }
      }
      break;

   case ID_EDIT_REDO:
      {
         String strAction = m_pEditWnd->getRedoName();
         if ( !strAction.empty() ) {
            strArg = strAction;
            nItem = ID_EDIT_REDO_ARG;
         }
      }
      break;

   case ID_EDIT_COPY:
   case ID_EDIT_CUT:
      {
         String strSel;
         if ( m_pEditWnd->getSel( &strSel ) && 
            (int) strSel.find( _T( '\n' ) ) < 0 ) 
         {
            assert( !strSel.empty() );
            nItem = ID_EDIT_COPY == nItem 
               ? ID_EDIT_COPY_ARG : ID_EDIT_CUT_ARG;
            strArg = compactPath( strSel.c_str(), 100 );
         }
      }
      break;

   case ID_EDIT_FINDNEXT:
      if ( !strLast.empty() ) {
         nItem = ID_EDIT_FINDNEXT_ARG;
         strArg = strLast;
      }
      break;

   case ID_EDIT_FINDPREVIOUS:
      if ( !strLast.empty() ) {
         nItem = ID_EDIT_FINDPREVIOUS_ARG;
         strArg = strLast;
      }
      break;

   case ID_EDIT_FINDSELECTION:
      if ( m_pEditWnd->getSel( &strArg ) ) {
         nItem = ID_EDIT_FINDSELECTION_ARG;
      } else if ( m_pEditWnd->getWord( &strArg ) ) {
         nItem = ID_EDIT_FINDWORD;
      }
      break;

   case ID_MRU_1: case ID_MRU_2: case ID_MRU_3:
   case ID_MRU_4: case ID_MRU_5: case ID_MRU_6:
   case ID_MRU_7: case ID_MRU_8: case ID_MRU_9:
      strArg = MRU().getFileTitle( nItem );
      nItem = ID_MRU_1;
      break;
   }

   return 0 == nItem 
      ? _T( "" ) : loadMenuDescription( nItem, strArg.c_str() );
}


void Editor::updateToolbar( void ) {

   assert( isGoodPtr( this ) );
   if ( (Toolbar *) 0 == m_pToolbar ) {
       MessageBox( HWND_DESKTOP, _T( "0 == m_pToolbar" ), 0, MB_OK );
       debugBreak();
   }
   if ( !isGoodPtr( m_pToolbar ) ) {
       MessageBox( HWND_DESKTOP, _T( "!isGoodPtr( m_pToolbar )" ), 0, MB_OK );
       debugBreak();
   }
   assert( isGoodPtr( m_pToolbar ) );

   const bool bCanUndo = m_pEditWnd->canUndo();
   m_pToolbar->setEnabled( ID_EDIT_UNDO, bCanUndo );
   if ( m_pEditWnd->hasRedo() ) {
      m_pToolbar->setEnabled( ID_EDIT_REDO, m_pEditWnd->canRedo() );
   }

   int nStart = 0;
   int nEnd   = 0;
   
   const bool bHasSelection = m_pEditWnd->getSel( &nStart, &nEnd );
   const int  nTextLength   = m_pEditWnd->getTextLength( true );
   
   // The book says that the delete command should be enabled
   // if and only if there is text and the file is writable.
   // This is not quite right; if the caret is at the bottom
   // of the file, no delete is possible.
   const bool bDeleteOK = nStart < nTextLength;
   const bool bHasText  = 0 < m_pEditWnd->getTextLength();
   const bool bReadOnly = m_pEditWnd->isReadOnly();

   m_pToolbar->setEnabled( ID_EDIT_CUT   , bHasSelection && !bReadOnly );
   m_pToolbar->setEnabled( ID_EDIT_COPY  , bHasSelection );
   m_pToolbar->setEnabled( ID_EDIT_DELETE, !bReadOnly && bDeleteOK );
   m_pToolbar->setEnabled( ID_EDIT_FIND  , bHasText );
}


// implementation of EditListener:

void Editor::onChange( void ) {

   assert( isGoodPtr( this ) );
   updateToolbar();

   m_isClean = false;
   m_isWhistleClean = false;

   SetTimer( m_hwndMain, timer_save_document, 
      getAutoSaveTime(), autoSaveTimerProc );
}


void Editor::onMaxText( void ) {
   assert( isGoodPtr( this ) );
   m_pStatusbar->setHighlightMessage( IDS_ERR_SPACE );
}


void Editor::onErrSpace( void ) {
   assert( isGoodPtr( this ) );
   m_pStatusbar->setHighlightMessage( IDS_ERR_SPACE );
}


void Editor::onPosChange( const Point& position ) {

   assert( isGoodPtr( this ) );

   // May get called before constructor is done, so be careful:
   if ( (Toolbar *) 0 != m_pToolbar ) {
      assert( isGoodPtr( m_pToolbar ) );
      updateToolbar();
   }
   if ( (Statusbar *) 0 != m_pStatusbar ) {
      assert( isGoodPtr( m_pStatusbar ) );
      m_pStatusbar->update( position );
   }
}


void Editor::onZoomChange( const int zoomPercentage ) {
   if ( (Statusbar *) 0 != m_pStatusbar ) {
      assert( isGoodPtr( m_pStatusbar ) );
      m_pStatusbar->update( zoomPercentage );
   }
}


void Editor::loadAcceleratorTable( HINSTANCE hinst ) {

   assert( is_alive );
   if ( 0 != m_haccel ) {
      verify( DestroyAcceleratorTable( m_haccel ) );
   }
   m_haccel = 
      LoadAccelerators( hinst, MAKEINTRESOURCE( IDR_ACCELERATOR ) );
   assert( 0 != m_haccel );
}


// assignment within conditional expression
#pragma warning( disable: 4706 ) 

int Editor::run( void ) {

   assert( isGoodPtr( this ) );
   loadAcceleratorTable( getModuleHandle() );

   MSG msg = { 0, 0, static_cast< WPARAM >( -1 ) };

   // TEST: Force m_hwndMain to null and see what happens:
   // The thing is surprisingly well-behaved.
   // Accelerators don't work, of course, and it triggers a lot
   // of assertions, but it seems to work quite reliably.
   // (Menu updates are also a problem in some cases.)

   BOOL bRun = true;
   while ( bRun ) {
      try {
         assertValid();
         while ( bRun = GetMessage( &msg, 0, 0, 0 ) ) {
            assertValid();
            if ( !TranslateAccelerator( m_hwndMain, m_haccel, &msg )
                 && !isToolbarDialogMessage( &msg ) )
            {
               TranslateMessage( &msg );
               DispatchMessage ( &msg );
            }
         }
      }
      catch ( const NullPointerException& ) {
         trace( _T( "NullPointerException in Editor::run; this is OK.\n" ) );
      }
      catch ( const CommonDialogException& x ) {
         // *All* common dialog failures go through here.
         trace( _T( "CommonDialogException in Editor::run; %s\n" ), x.what() );
         m_pStatusbar->setHighlightMessage( IDS_STRING, x.what() );
      }
      catch ( const Exception& x ) {
         trace( _T( "Exception in Editor::run: \"%s\"\n" ), x.what() );
         if ( (Statusbar *) 0 != m_pStatusbar && 
              IsWindowVisible( *m_pStatusbar ) ) 
         {
            m_pStatusbar->setHighlightMessage( IDS_STRING, x.what() );
         } else {
            messageBox( GetLastActivePopup( m_hwndMain ),
               MB_OK, IDS_STRING, x.what() );
         }
      }
   }

   return msg.wParam;
}

// assignment within conditional expression
#pragma warning( default: 4706 )

// end of file
