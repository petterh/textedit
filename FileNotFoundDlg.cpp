/*
 * $Header: /Book/FileNotFoundDlg.cpp 18    11.07.01 14:40 Oslph312 $
 *
 * This dialog is displayed when a file can't be found.
 * It allows the user to create the file, select a different
 * file, or cancel the whole thing.
 */

#include "precomp.h"
#include "resource.h"
#include "Exception.h"
#include "FileNotFoundDlg.h"
#include "HTML.h"
#include "MRU.h"
#include "openFileDlg.h"
#include "fileUtils.h"
#include "formatMessage.h"
#include "utils.h"


class FileNotFoundDlg : public Dialog {
private:
   LPCTSTR m_pszOldFile;
   virtual UINT getResourceID( void ) const;

   virtual BOOL onInitDialog( HWND hwndFocus, LPARAM lParam );
   virtual void onDlgCommand( int id, HWND hwndCtl, UINT codeNotify );

public:
   PATHNAME m_szNewFile;
   FileNotFoundDlg( LPCTSTR pszOldFile );
};


FileNotFoundDlg::FileNotFoundDlg( LPCTSTR pszOldFile )
   : m_pszOldFile( pszOldFile )
{
   _tcscpy_s( m_szNewFile, m_pszOldFile );
}


UINT FileNotFoundDlg::getResourceID( void ) const {
   return IDD_FILENOTFOUND;
}


BOOL FileNotFoundDlg::onInitDialog( HWND hwndFocus, LPARAM lParam ) {

   subclassHTML( getDlgItem( IDC_MESSAGE ) );

   const String strMessage = 
      formatMessage( IDS_FILE_NOT_FOUND, m_pszOldFile );
   setDlgItemText( IDC_MESSAGE, strMessage.c_str() );

   const HICON hicon = 
      LoadIcon( getModuleHandle(), MAKEINTRESOURCE( IDI_TEXTEDIT1 ) );
   sendMessage( WM_SETICON, ICON_SMALL, 
      reinterpret_cast< LPARAM >( hicon ) );
   sendMessage( WM_SETICON, ICON_BIG, 
      reinterpret_cast< LPARAM >( hicon ) );

   MessageBeep( MB_ICONQUESTION );

   return TRUE; // We did *not* set the focus.
}


void FileNotFoundDlg::onDlgCommand( 
   int id, HWND hwndCtl, UINT codeNotify ) 
{
   switch ( id ) {
   case IDOK:
      verify( EndDialog( *this, id ) );
      break;

   case IDCANCEL:
      verify( EndDialog( *this, id ) );
      break;

   case IDC_BROWSE:
      const bool bOpenOK = 
         openFileDlg( *this, m_szNewFile, dim( m_szNewFile ), 0, false );
      if ( bOpenOK ) {
         verify( EndDialog( *this, id ) );
      }
      break;
   }
}


static inline HANDLE openNewFile( const String& strNewFile ) {

   HANDLE hFile = CreateFile( strNewFile.c_str(), 
      GENERIC_READ_WRITE, FILE_SHARE_READ, 0, 
      OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0 );
   if ( INVALID_HANDLE_VALUE == hFile ) { // Read-only?
      hFile = CreateFile( strNewFile.c_str(), 
         GENERIC_READ, FILE_SHARE_READ, 0, 
         OPEN_ALWAYS, 0, 0 );
   }
   return hFile;
}


PRIVATE void cleanup( LPCTSTR pszOld, LPCTSTR pszNew = 0 ) {
   
   if ( 0 == pszNew || !areFileNamesEqual( pszOld, pszNew ) ) {
      MRU mru;
      mru.removeFile( pszOld );
   }
}


HANDLE getNewFile( HWND hwnd, String *pstrFile ) throw(CancelException) {

   assert( 0 != pstrFile );
   const String strOldFile( *pstrFile );

   FileNotFoundDlg fileNotFoundDlg( strOldFile.c_str() );
   const UINT uiRet = fileNotFoundDlg.doModal( hwnd );
   switch ( uiRet ) {
   case IDC_BROWSE:
   case IDOK:
      pstrFile->assign( fileNotFoundDlg.m_szNewFile );
      cleanup( strOldFile.c_str(), pstrFile->c_str() );
      return openNewFile( *pstrFile );

   case IDCANCEL:
      cleanup( strOldFile.c_str() );
      throw CancelException();
   }

//   assert( false ); // Should never happen!
   return INVALID_HANDLE_VALUE;
}

// end of file
