/*
 * $Header: /Book/AboutDlg.cpp 19    5.03.02 9:59 Oslph312 $
 * 
 * Handles the About dialog box. This is general enough for reuse.
 */

#include "precomp.h"
#include "resource.h"
#include "formatMessage.h"
#include "AboutDlg.h"
#include "VersionInfo.h"
#include "HTML.h"
#include "utils.h"

// TODO: Unit test new safe string API
void AboutDlg::setFonts( void ) {
   m_hfontBig = GetWindowFont( getDlgItem( IDC_TITLE ) );
   m_hfontBold = m_hfontBig;
   if ( 0 != m_hfontBig ) {
      LOGFONT logFont = { 0 };
      if ( GetObject( m_hfontBig, sizeof logFont, &logFont ) ) {
         logFont.lfWeight = FW_HEAVY;
         logFont.lfWidth  *= 2;
         logFont.lfHeight *= 2;
         const String strSavedFaceName( logFont.lfFaceName );
         _tcscpy_s( logFont.lfFaceName, _T( "Comic Sans MS" ) );
         m_hfontBig = CreateFontIndirect( &logFont );
         if ( 0 == m_hfontBig ) {
            _tcscpy_s( logFont.lfFaceName, _T( "Arial" ) );
            m_hfontBig = CreateFontIndirect( &logFont );
         }
         if ( 0 == m_hfontBig ) {
            _tcscpy_s( logFont.lfFaceName, strSavedFaceName.c_str() );
            m_hfontBig = CreateFontIndirect( &logFont );
         }
      }
      assert( 0 != m_hfontBold );
      if ( GetObject( m_hfontBold, sizeof logFont, &logFont ) ) {
         logFont.lfWeight = FW_BOLD;
         m_hfontBold = CreateFontIndirect( &logFont );
      }
   }
   if ( 0 != m_hfontBig ) {
      SetWindowFont( getDlgItem( IDC_TITLE ), m_hfontBig, false );
   }
   if ( 0 != m_hfontBold ) {
      SetWindowFont( getDlgItem( IDC_VERSION ), m_hfontBold, false );
   }
}


void AboutDlg::setInfo( void ) {

   const VersionInfo vi( getModuleHandle() );
   if ( !vi.isValid() ) {
      trace( _T( "Unable to retrieve version info\n" ) );
      return;
   }
   
   // Program name:
   LPCTSTR pszTitle = vi.getStringFileInfo( _T( "FileDescription" ) );
   assert( 0 != pszTitle );
   setDlgItemText( IDC_TITLE, pszTitle );
   String strFmt = getWindowText();
   setWindowText( formatMessage( strFmt, pszTitle ).c_str() );
   
   // Program version:
   const LPCTSTR pszVersion = vi.getStringFileInfo( _T( "FileVersion" ) );
   assert( 0 != pszVersion );
   strFmt = getDlgItemText( IDC_VERSION );

#ifdef _DEBUG
   strFmt += _T( " Debug" );
#endif

#ifdef UNICODE
   strFmt += L" (Unicode build)";
#endif

   setDlgItemTextF( IDC_VERSION, strFmt, pszVersion );

   // Copyright: // TODO: Replace "\n" with '\n'
   String copyright( vi.getStringFileInfo( _T( "LegalCopyright" ) ) );
   replace( &copyright, _T( "\\n" ), _T( "\n" ) );
   replace( &copyright, _T( "<p>" ), _T( "\n" ) );
   replace( &copyright, _T( "%n"  ), _T( "\n" ) );
   setDlgItemText( IDC_COPYRIGHT, copyright.c_str() );
   
   // Comments:
   const LPCTSTR psz = vi.getStringFileInfo( _T( "Comments" ) );
   assert( 0 != psz );
   setDlgItemText( IDC_COMMENTS, psz );
}


BOOL AboutDlg::onInitDialog( HWND hwndFocus, LPARAM lParam ) {

   setFonts();
   setInfo();

   subclassHTML( getDlgItem( IDC_COMMENTS   ) );
   subclassHTML( getDlgItem( IDC_COMMERCIAL ) );

   return true; // Let dialog manager set the focus.
}


UINT AboutDlg::getResourceID( void ) const {
   return IDD_ABOUT;
}


AboutDlg::AboutDlg( HWND hwndParent ) 
   : m_hfontBig ( 0 ) 
   , m_hfontBold( 0 )
{
#ifdef _DEBUG
   const UINT uiRetCode = 
#endif

   doModal( hwndParent );
   assert( IDOK == uiRetCode || IDCANCEL == uiRetCode );
}


AboutDlg::~AboutDlg() {

   if ( 0 != m_hfontBold && m_hfontBig != m_hfontBold ) {
      verify( DeleteFont( m_hfontBold ) );
      m_hfontBold = 0;
   }
   if ( 0 != m_hfontBig ) {
      verify( DeleteFont( m_hfontBig ) );
      m_hfontBig = 0;
   }
}

// end of file
