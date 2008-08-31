/*
 * $Header: /Book/InstallDlg1.cpp 14    6.11.01 11:31 Oslph312 $
 */

#include "precomp.h"
#include "VersionInfo.h"
#include "Exception.h"
#include "Registry.h"
#include "HTML.h"
#include "InstallDlg1.h"
#include "AutoShellObject.h"
#include "FileType.h"
#include "formatMessage.h"
#include "menuUtils.h"
#include "setup.h"
#include "persistence.h"
#include "language.h"
#include "geometry.h"
#include "utils.h"
#include "fileUtils.h"
#include "winUtils.h"
#include "resource.h"


UINT InstallDlg1::getResourceID( void ) const {
   return IDD_INSTALL_1;
}


BOOL InstallDlg1::DlgProc( UINT msg, WPARAM wParam, LPARAM lParam ) {

   // Putting this here results in its getting called
   // more often than necessary. But my, how convenient!
   HMENU hmenu = GetSystemMenu( *this, false );
   enableMenuItem( hmenu, SC_RESTORE, 0 != IsIconic( *this ) );

   if ( WM_MOUSEWHEEL == msg ) {
      HWND hwndList = getDlgItem( IDC_FILETYPES );
      return SendMessage( hwndList, WM_MOUSEWHEEL, wParam, lParam );
   }
   return Dialog::DlgProc( msg, wParam, lParam );
}


BOOL InstallDlg1::onInitDialog( HWND hwndFocus, LPARAM lParam ) {

   initLanguageComboBox( *this );

   HMENU hmenu = GetSystemMenu( *this, false );
   DeleteMenu( hmenu, SC_MAXIMIZE, MF_BYCOMMAND );
   DeleteMenu( hmenu, SC_SIZE    , MF_BYCOMMAND );
   enableMenuItem( hmenu, SC_RESTORE, false );

   const HICON hicon = 
      LoadIcon( m_hinst, MAKEINTRESOURCE( IDI_TEXTEDIT1 ) );
   sendMessage( WM_SETICON, ICON_SMALL, 
      reinterpret_cast< LPARAM >( hicon ) );
   sendMessage( WM_SETICON, ICON_BIG, 
      reinterpret_cast< LPARAM >( hicon ) );
   subclassHTML( getDlgItem( IDC_TIP ) );
   
   setDlgItemText( IDC_PATH, m_strInstallDir );
   SetFocus( getDlgItem( IDC_PATH ) );
   SendDlgItemMessage( *this, IDC_PATH, EM_SETSEL, 0, -1 );

   setDlgItemText( IDC_DOC_PATH, m_strDataDir );
   setupList();

   return false; // Don't let dialog manager set the focus.
}


#define ListView_CheckItem( hwnd, item ) \
   ListView_SetItemState( hwnd, item, 2 << 12, LVIS_STATEIMAGEMASK)


void InstallDlg1::getList( void ) {

   HWND hwndList = getDlgItem( IDC_FILETYPES );
   assert( IsWindow( hwndList ) );

   const int nItems = ListView_GetItemCount( hwndList );
   for ( int iItem = 0; iItem < nItems; ++iItem ) {
      const bool bInclude = 
         0 != ListView_GetCheckState( hwndList, iItem );
      TCHAR sz[ 200 ] = { 0 };
      ListView_GetItemText( hwndList, iItem, 1, sz, dim( sz ) );
      FileType::include( sz, bInclude );
   }
}

// TODO: Unit test new safe string API
void InstallDlg1::setupList( void ) {

   HWND hwndList = getDlgItem( IDC_FILETYPES );
   assert( IsWindow( hwndList ) );

   DWORD dwExStyle = ListView_GetExtendedListViewStyle( hwndList );
   ListView_SetExtendedListViewStyle( hwndList, 
      dwExStyle | LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT );

   const Rect rcClient = getClientRect( hwndList );
   const int w = rcClient.width() - GetSystemMetrics( SM_CXVSCROLL );
   const int cx0 = MulDiv( w, 3, 4 );

   String str = loadString( IDS_FILE_TYPE );
   LV_COLUMN lvColumn = {
      LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH,
      LVCFMT_LEFT, cx0, 
      const_cast< LPTSTR >( str.c_str() ), 
      0, /* image */ 0, /* order */ 0, 
   };
   ListView_InsertColumn( hwndList, 0, &lvColumn );
   
   str = loadString( IDS_EXTENSIONS );
   lvColumn.pszText = const_cast< LPTSTR >( str.c_str() );
   lvColumn.cx = w - cx0;
   ListView_InsertColumn( hwndList, 1, &lvColumn );

   SHFILEINFO fileInfo = { 0 };
   HIMAGELIST hSysImageList = reinterpret_cast< HIMAGELIST >( 
      SHGetFileInfo( _T( ".txt" ), FILE_ATTRIBUTE_NORMAL, 
         &fileInfo, sizeof fileInfo, 
         SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES | 
         SHGFI_SMALLICON ) );
   ListView_SetImageList( hwndList, hSysImageList, LVSIL_SMALL );

   const int nTypes = FileType::getNumFileTypes();
   for ( int iType = 0; iType < nTypes; ++iType ) {

      const FileType *pFileType = FileType::getFileType( iType );
      if ( 0 == pFileType || !pFileType->exists() ) {
         continue; //*** LOOP CONTINUATION
      }
      assert( isGoodConstPtr( pFileType ) );

      const UINT uiFlags = 
         SHGFI_USEFILEATTRIBUTES |
         SHGFI_ICON              | 
         SHGFI_SMALLICON         | 
         SHGFI_TYPENAME          ;
      PATHNAME szExt = { 0 };
      _tcscpy_s( szExt, pFileType->getExtension() );
      LPTSTR pszSpace = _tcschr( szExt, _T( ' ' ) );
      if ( 0 != pszSpace ) {
         *pszSpace = 0;
      }
      SHGetFileInfo( szExt, FILE_ATTRIBUTE_NORMAL,
         &fileInfo, sizeof fileInfo, uiFlags );

      const int nItems = ListView_GetItemCount( hwndList );
      LV_ITEM lvItem = {
         LVIF_TEXT | LVIF_IMAGE | LVIF_STATE, nItems, 
         0, 0 == nItems ? (LVIS_FOCUSED | LVIS_SELECTED) : 0, 0,
         fileInfo.szTypeName, _tcsclen( fileInfo.szTypeName ), 
         fileInfo.iIcon, (LPARAM) 0,
      };
      ListView_InsertItem( hwndList, &lvItem );
      ListView_CheckItem( hwndList, nItems );

      LPCTSTR pszExt = pFileType->getExtension() + 1;
      lvItem.mask = LVIF_TEXT;
      lvItem.iSubItem = 1;
      lvItem.pszText = const_cast< LPTSTR >( pszExt );
      lvItem.cchTextMax = _tcsclen( pszExt );
      ListView_SetItem( hwndList, &lvItem );
   }
}


/**
 * Callback function for SHBrowseForFolder. It is used to
 * a) set the initial directory, and
 * b) ensure that the defaultness of the OK button is OK.
 */
PRIVATE int CALLBACK BrowseCallbackProc( 
   HWND hwnd, UINT msg, LPARAM, LPARAM lpData ) 
{
   if ( BFFM_INITIALIZED == msg ) {
      SNDMSG( hwnd, BFFM_SETSELECTION, TRUE, lpData );
   } else if ( BFFM_SELCHANGED == msg ) {
      HWND hwndOK = GetDlgItem( hwnd, IDOK );
      if ( IsWindowEnabled( hwndOK ) ) {
         setButtonStyle( hwndOK, BS_DEFPUSHBUTTON );
      } else {
         setButtonStyle( hwndOK, BS_PUSHBUTTON );
      }
   }

   return 0;
}


void InstallDlg1::onBrowse( UINT id, LPCTSTR pszTitle ) {

   const String strDefault = getDlgItemText( id );
   PATHNAME szPath = { 0 };
   
   // The pszDisplayName member of BROWSEINFO returns, for example,
   // "(C:)" or "3½ Floppy (A:)" instead of "C:\" or "A:\", so it's
   // not used. szPath is used as a placeholder to avoid declaring 
   // an additional string.

   // A little bit of confusion here -- the lParam member of bi
   // repappears as the lpData (rather than the lParam) parameter
   // to BrowseCallbackProc.
   BROWSEINFO bi = {
      *this, /* root */ 0, szPath, pszTitle,
      BIF_RETURNONLYFSDIRS, BrowseCallbackProc,
      reinterpret_cast< LPARAM >( strDefault.c_str() ),
   };

   AutoShellObject< ITEMIDLIST > 
      pItemIdList( SHBrowseForFolder( &bi ) );
   if ( !pItemIdList.isNull() ) {
      if ( SHGetPathFromIDList( pItemIdList, szPath ) ) {
         String strPath( szPath );
         if ( IDC_PATH == id ) {
            appendProgramName( &strPath );
         }
         setDlgItemText( id, strPath.c_str() );
         SendDlgItemMessage( *this, id, EM_SETSEL, 0, -1 );
         gotoDlgItem( id );
      }
   }
}


void InstallDlg1::onDlgCommand( 
   int id, HWND hwndCtl, UINT codeNotify ) 
{
   switch ( id ) {
   case IDC_BROWSE: 
      onBrowse( IDC_PATH, loadString( IDS_CHOOSE_FOLDER ).c_str() );
      break;

   case IDC_BROWSEDOCPATH:
      onBrowse( IDC_DOC_PATH, 
         loadString( IDS_CHOOSE_DATA_FOLDER ).c_str() );
      break;

   case IDC_INSTALL:
      m_strInstallDir = getDlgItemText( IDC_PATH     );
      m_strDataDir    = getDlgItemText( IDC_DOC_PATH );
      getList();
      exitLanguageComboBox( *this );

      //*** FALL THROUGH

   case IDCANCEL:
      ListView_SetImageList( 
         getDlgItem( IDC_FILETYPES ), 0, LVSIL_SMALL );
      verify( EndDialog( *this, id ) );
      break;
   }
}


InstallDlg1::InstallDlg1( HINSTANCE hinst, 
   const String& strInstallDir, const String& strDataDir )
   : m_hinst        ( hinst         ) 
   , m_strInstallDir( strInstallDir )
   , m_strDataDir   ( strDataDir    )
{
}

// end of file
