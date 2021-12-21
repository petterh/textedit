/*
 * $Header: /Book/language.cpp 5     6.11.01 11:39 Oslph312 $
 */

#include "precomp.h"
#include "VersionInfo.h"
#include "persistence.h"
#include "os.h"
#include "utils.h"
#include "resource.h"
#include "exeContainsLanguageResources.h"


PRIVATE void transformToExistingLanguage( int *pLanguage ) {

   assert( isGoodPtr( pLanguage ) );
   if ( !exeContainsLanguageResources( (WORD) *pLanguage ) ) {
      // Language not found; set to English:
      *pLanguage = 0x409;
   }
}


void setNewLanguage( int nLanguage ) {

   setLanguage( nLanguage );
   if (-1 == nLanguage) {
       nLanguage = GetUserDefaultLangID();
   } else if (0 == nLanguage) {
       nLanguage = GetSystemDefaultLangID();
   }
   transformToExistingLanguage(&nLanguage);
   verify(setThreadLocale(MAKELCID(nLanguage, SORT_DEFAULT)));
   setLanguage(nLanguage);
}


PRIVATE void getLanguageName( int nLang, LPTSTR psz, UINT chars ) {

   assert( isGoodStringPtr( psz ) );
   VerLanguageName( nLang, psz, chars );
   if ( 0 == _tcscmp( psz, _T( "Norwegian (Bokmal)" ) ) ) {
      _tcscpy_s( psz, chars, _T( "Norwegian (Bokmål)" ) );
   }
}


void initLanguageComboBox( HWND hwnd ) {

   assert( IsWindow( hwnd ) );
   HWND hwndLanguage = GetDlgItem( hwnd, IDC_LANGUAGE );
   assert( IsWindow( hwndLanguage ) );
   ComboBox_SetExtendedUI( hwndLanguage, true );

   TCHAR szLanguage[ 100 ] = { 0 };
   TCHAR szMessage [ 200 ] = { 0 };

   int nIndex = 0;

   const int nUserDefaultLanguage = GetUserDefaultLangID();
   if (exeContainsLanguageResources(nUserDefaultLanguage)) {
       getLanguageName(
           nUserDefaultLanguage, szLanguage, dim(szLanguage));
       wsprintf(szMessage, _T("*User Default - %s"), szLanguage);
       nIndex = ComboBox_AddString(hwndLanguage, szMessage);
       assert(0 <= nIndex);
       ComboBox_SetItemData(hwndLanguage, nIndex, -1);
   }

   const int nSystemDefaultLanguage = GetSystemDefaultLangID();
   if ( exeContainsLanguageResources( nSystemDefaultLanguage ) ) {
      getLanguageName( 
         nSystemDefaultLanguage, szLanguage, dim( szLanguage ) );
      wsprintf( szMessage, _T( "*System Default - %s" ), szLanguage );
      nIndex = ComboBox_AddString( hwndLanguage, szMessage );
      assert( 0 <= nIndex );
      const int nSysLang = 0;
      ComboBox_SetItemData( hwndLanguage, nIndex, nSysLang );
   }

   const VersionInfo vi( getModuleHandle() );
   if ( vi.isValid() ) {
      const int nLanguages = vi.getLanguageCount();
      for ( int iLanguage = 0; iLanguage < nLanguages; ++iLanguage ) {
         const UINT uiLangID = vi.getLanguage( iLanguage );
         getLanguageName( uiLangID, szLanguage, dim( szLanguage ) );
         nIndex = ComboBox_AddString( hwndLanguage, szLanguage );
         assert( 0 <= nIndex );
         ComboBox_SetItemData( hwndLanguage, nIndex, uiLangID );
      }
   }

   const int nSelectedLanguage = getLanguage();
   const int nLanguages = ComboBox_GetCount( hwndLanguage );
   for ( int iLanguage = 0; iLanguage < nLanguages; ++ iLanguage ) {
      const int nCurrLang = 
         ComboBox_GetItemData( hwndLanguage, iLanguage );
      if ( nCurrLang == nSelectedLanguage ) {
         ComboBox_SetCurSel( hwndLanguage, iLanguage );
         break; //*** LOOP EXIT POINT
      }
   }
}


void exitLanguageComboBox( HWND hwnd ) {

   assert( IsWindow( hwnd ) );
   const HWND hwndLanguage = GetDlgItem( hwnd, IDC_LANGUAGE );
   assert( IsWindow( hwndLanguage ) );
   int nCurSel = ComboBox_GetCurSel( hwndLanguage );
   if ( 0 <= nCurSel ) {
      const int language = ComboBox_GetItemData( hwndLanguage, nCurSel );
      setNewLanguage( language );
   }
}

// end of file
