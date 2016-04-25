/*
 * $Header: /Book/OptionsDlg.cpp 16    6-09-01 12:50 Oslph312 $
 * 
 * Handles the TextEdit Options dialog box. 
 */

#include "precomp.h"
#include "OptionsDlg.h"
#include "FontDlg.h"
#include "VersionInfo.h"
#include "ClientDC.h"
#include "formatMessage.h"
#include "winUtils.h"
#include "language.h"
#include "persistence.h"
#include "os.h"
#include "utils.h"
#include "resource.h"


/**
 * Helper function for getFontDescription.
 * Part of the information in ENUMLOGFONTEX seems to be garbage
 * under Windows 95, which explains the outer conditional.
 */
PRIVATE int CALLBACK EnumFontFamExProc(
   const LOGFONT *pLogFont, const TEXTMETRIC *, 
   DWORD dwType, LPARAM lParam )
{
   String *pstrDescription = reinterpret_cast< String * >( lParam );
   const ENUMLOGFONTEX *pEnumLogFontEx = 
      reinterpret_cast< const ENUMLOGFONTEX * >( pLogFont );
   assert( isGoodConstPtr( pEnumLogFontEx ) );

   if (0 != pEnumLogFontEx->elfStyle[0]) {
       *pstrDescription = formatMessage(_T("%1 %2 %%1!d! (%3)"),
           pEnumLogFontEx->elfFullName,
           pEnumLogFontEx->elfStyle,
           pEnumLogFontEx->elfScript);
   } else {
       *pstrDescription = formatMessage(_T("%1 %%1!d! (%2)"),
           pEnumLogFontEx->elfFullName,
           pEnumLogFontEx->elfScript);
   }

   return 0;
}


PRIVATE String getFontDescription( const LOGFONT *pLogFont ) {

   assert( 0 != pLogFont );
   String strDescription( pLogFont->lfFaceName ); // Default

   ClientDC dc;
   if ( dc.isValid() ) {
      EnumFontFamiliesEx( dc, 
         const_cast< LOGFONT *>( pLogFont ), EnumFontFamExProc,
         reinterpret_cast< LPARAM >( &strDescription ), 0 ); 
   }
   const int nHeight = 
      printerPointsFromDevPoints( pLogFont->lfHeight );
   return formatMessage( strDescription, nHeight );
}


BOOL OptionsDlg::onInitDialog( HWND hwndFocus, LPARAM lParam ) {

   initLanguageComboBox( *this );

   SetWindowFont( getDlgItem( IDC_FIXED_FONT_SAMPLE ), 
      m_hfontFixed, true );
   SetWindowFont( getDlgItem( IDC_PROPORTIONAL_FONT_SAMPLE ), 
      m_hfontProportional, true );

   String str = getFontDescription( &m_logFontFixed );
   setDlgItemText( IDC_FIXED_FONT_SAMPLE, str );

   str = getFontDescription( &m_logFontProportional );
   setDlgItemText( IDC_PROPORTIONAL_FONT_SAMPLE, str );

   str = getDocumentPath();
   setDlgItemText( IDC_DOC_PATH, str );

   Button_SetCheck( getDlgItem( IDC_SHOWDELETEDIALOG ), 
      getShowDeleteDialog() ? 1 : 0 );

#if 0
   EnableWindow( getDlgItem( IDC_SHOWDELETEDIALOG2 ), FALSE );
#endif

   return true; // Let dialog manager set the focus.
}


// Note: The actual font change is done by caller (if dialog is terminated with OK).
// Should this dialog have an Apply button?
void OptionsDlg::changeFont(
   HWND hwndSample, LOGFONT *pLogFont, 
   HFONT *phfont, DWORD dwExtraFlags ) 
{
   const Rect rcSample = getWindowRect( hwndSample );
   const bool bOK = selectFont(
      *this, pLogFont, &rcSample, dwExtraFlags );
   if ( bOK ) {
      verify( DeleteFont( *phfont ) );
      *phfont = CreateFontIndirect( pLogFont );
      SetWindowFont( hwndSample, *phfont, true );
      const String str = getFontDescription( pLogFont );
      SetWindowText( hwndSample, str.c_str() );
      gotoDlgItem( IDOK );
   }
}


void OptionsDlg::onDlgCommand( 
   int id, HWND hwndCtl, UINT codeNotify ) 
{
   switch ( id ) {
   case IDC_CHANGE_FIXED_FONT:
      if ( BN_CLICKED == codeNotify ) {
         changeFont( getDlgItem( IDC_FIXED_FONT_SAMPLE ),
            &m_logFontFixed, &m_hfontFixed, CF_FIXEDPITCHONLY );
      }
      break;

   case IDC_CHANGE_PROPORTIONAL_FONT:
      if ( BN_CLICKED == codeNotify ) {
         changeFont( getDlgItem( IDC_PROPORTIONAL_FONT_SAMPLE ),
            &m_logFontProportional, &m_hfontProportional );
      }
      break;

   case IDOK:
      exitLanguageComboBox( *this );
      setShowDeleteDialog( 
         0 != Button_GetCheck( getDlgItem( IDC_SHOWDELETEDIALOG ) ) );
      setDocumentPath( getDlgItemText( IDC_DOC_PATH ).c_str() );
      //*** FALL THROUGH

   case IDCANCEL:
      verify( EndDialog( *this, id ) );
      break;
   }
}


UINT OptionsDlg::getResourceID( void ) const {
   return IDD_OPTIONS;
}


OptionsDlg::OptionsDlg( 
   const LOGFONT *pLogFontFixed, 
   const LOGFONT *pLogFontProportional )

   : m_logFontFixed( *pLogFontFixed )
   , m_logFontProportional( *pLogFontProportional )
   , m_hfontFixed( CreateFontIndirect( pLogFontFixed ) )
   , m_hfontProportional( CreateFontIndirect( pLogFontProportional ) )
{
   m_logFontFixed.lfHeight = -abs( m_logFontFixed.lfHeight );
   m_logFontProportional.lfHeight = 
      -abs( m_logFontProportional.lfHeight );
}


OptionsDlg::~OptionsDlg() {

   verify( DeleteFont( m_hfontFixed ) );
   verify( DeleteFont( m_hfontProportional ) );
}

// end of file
