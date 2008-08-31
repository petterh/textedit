/*
 * $Header: /Book/PropertiesDlg.cpp 17    5.09.99 13:07 Oslph312 $
 *
 * LATER: Warning if data lost converting from Unicode!
 */

#include "precomp.h"
#include "String.h"
#include "Registry.h"
#include "Exception.h"
#include "SilentErrorMode.h"
#include "Dialog.h"
#include "PropertiesDlg.h"
#include "HTML.h"
#include "WaitCursor.h"
#include "formatMessage.h"
#include "formatNumber.h"
#include "saveFile.h"
#include "fileUtils.h"
#include "resource.h"
#include "utils.h"
#include "os.h"


UINT PropertiesDlg::getResourceID( void ) const {
   return IDD_PROPERTIES;
}


PropertiesDlg::PropertiesDlg( HWND hwndParent, Document *pDocument ) 
   : m_pDocument( pDocument )
{
#ifdef _DEBUG  
   const UINT uiRetCode = 
#endif

   doModal( hwndParent, getResourceID() );
   assert( IDOK == uiRetCode || IDCANCEL == uiRetCode );
}


PRIVATE String formatFileTime( const FILETIME& ft ) {

   FILETIME local = ft;
   verify( FileTimeToLocalFileTime( &ft, &local ) );

   SYSTEMTIME st;
   verify( FileTimeToSystemTime( &local, &st ) );

   TCHAR szTime[ 100 ] = { 0 };
   GetTimeFormat( 
      LOCALE_USER_DEFAULT, 0, &st, 0, szTime, dim( szTime ) );

   TCHAR szDate[ 100 ] = { 0 };
   GetDateFormat( LOCALE_USER_DEFAULT, 
      DATE_LONGDATE, &st, 0, szDate, dim( szDate ) );
   
   return formatMessage( _T( "%1 %2" ), szDate, szTime );
}



String PropertiesDlg::formatBytes( DWORD dwBytes, bool bAddUsed ) {

   TCHAR szFileSize[ 100 ] = { 0 };
   StrFormatByteSize( dwBytes, szFileSize, dim( szFileSize ) );
   
   const String strByteSizeAll = formatNumber( dwBytes );
   wsprintf( szFileSize + _tcsclen( szFileSize ), 
      loadString( IDS_BYTES ).c_str(), strByteSizeAll.c_str() );

   if ( bAddUsed ) {
      const DWORD dwClusterSize = 
         getClusterSize( m_pDocument->getPath() );
      // We may not always get a cluster size -- 
      // can happen on network volumes, for example...
      if ( 0 != dwClusterSize ) {
         const DWORD dwClustersUsed = 
            ( (dwBytes + dwClusterSize - 1) / dwClusterSize );
         const DWORD dwBytesUsed = dwClustersUsed * dwClusterSize;
         const String strBytesUsed = formatNumber( dwBytesUsed );
         wsprintf( szFileSize + _tcsclen( szFileSize ), 
            loadString( IDS_BYTES_USED ).c_str(), 
            strBytesUsed.c_str() );
      }
   }

   return szFileSize;
}


void PropertiesDlg::setInfo( const WIN32_FIND_DATA& fd ) {
   
   const String strTitle = m_pDocument->getTitle();
   const String strDialogTitle = 
      formatMessage( m_strFormat, strTitle.c_str() );
   setWindowText( strDialogTitle );

   setFileName( m_pDocument->getPath() );
   setDlgItemText( IDC_FILETYPE, m_pDocument->getFileTypeDescription( true ) );

   LPCTSTR pszMsDosName = fd.cAlternateFileName;
   if ( 0 == pszMsDosName[ 0 ] ) {
      pszMsDosName = fd.cFileName;
   }
   setDlgItemText( IDC_MSDOS_NAME, pszMsDosName );
   
   // We don't handle files that big :-)
   assert( 0 == fd.nFileSizeHigh );
   const String strFileSize = formatBytes( fd.nFileSizeLow, true );
   setDlgItemText( IDC_FILESIZE, strFileSize );

   DWORD dwCompressedSize = 0;
   const bool isCompressed = 
      (FILE_ATTRIBUTE_COMPRESSED & fd.dwFileAttributes) &&
      getCompressedFileSize( m_pDocument->getPath().c_str(),
      &dwCompressedSize );
   if ( isCompressed ) {
      const String strCompressedSize = formatBytes( dwCompressedSize, false );
      setDlgItemText( IDC_COMPRESSEDFILESIZE, strCompressedSize );
   } else {
      setDlgItemText( IDC_COMPRESSEDFILESIZE, loadString( IDS_FILE_NOT_COMPRESSED ) );
   }

   setDlgItemText( IDC_CREATED , formatFileTime( fd.ftCreationTime   ).c_str() );
   setDlgItemText( IDC_MODIFIED, formatFileTime( fd.ftLastWriteTime  ).c_str() );
   setDlgItemText( IDC_ACCESSED, formatFileTime( fd.ftLastAccessTime ).c_str() );

   SHFILEINFO fileInfo = { 0 };
   UINT uiFlags = SHGFI_ICON | SHGFI_TYPENAME;
   SHGetFileInfo( m_pDocument->getPath().c_str(), 0, &fileInfo, sizeof fileInfo, uiFlags );
   //setDlgItemText( IDC_FILETYPE, fileInfo.szTypeName );
   HWND hwndIcon = getDlgItem( IDC_FILEICON );
   Static_SetIcon( hwndIcon, fileInfo.hIcon );

   #define CHK_ATTR( id, attr) Button_SetCheck( getDlgItem( id ), 0 != (attr & fd.dwFileAttributes) )
   #define CHK_BOOL( id, val) Button_SetCheck(  getDlgItem( id ), val )

   CHK_ATTR( IDC_ARCHIVE   , FILE_ATTRIBUTE_ARCHIVE    );
   CHK_ATTR( IDC_COMPRESSED, FILE_ATTRIBUTE_COMPRESSED );
   CHK_ATTR( IDC_READ_ONLY , FILE_ATTRIBUTE_READONLY   );
   CHK_ATTR( IDC_HIDDEN    , FILE_ATTRIBUTE_HIDDEN     );
   CHK_ATTR( IDC_SYSTEM    , FILE_ATTRIBUTE_SYSTEM     );

   CHK_BOOL( IDC_UNICODE      , m_pDocument->isUnicode()        );
   CHK_BOOL( IDC_UNIXLINEFEEDS, m_pDocument->hasUnixLineFeeds() );

   #undef CHK_ATTR
   #undef CHK_BOOL

   const bool bReadOnly = 0 != (FILE_ATTRIBUTE_READONLY & fd.dwFileAttributes);
   if ( !m_pDocument->isAccessDenied() ) {
      enableDlgItem( IDC_UNICODE      , !bReadOnly );
      enableDlgItem( IDC_UNIXLINEFEEDS, !bReadOnly );
   }
}


BOOL PropertiesDlg::onInitDialog( HWND hwndFocus, LPARAM lParam ) {

   SilentErrorMode sem;

   m_strFormat = getWindowText();
   
   WIN32_FIND_DATA fd = { 0 };
   HANDLE hFind = 
      FindFirstFile( m_pDocument->getPath().c_str(), &fd );
   if ( INVALID_HANDLE_VALUE == hFind ) {
      messageBox( GetParent( *this ), MB_OK | MB_ICONERROR,
         IDS_PROPERTIES_ERROR, m_pDocument->getPath().c_str(), getError().c_str() );
      EndDialog( *this, IDCANCEL );
   } else {
      verify( FindClose( hFind ) );
      setInfo( fd );
      if ( m_pDocument->isAccessDenied() ) {
         enableDlgItem( IDC_FILENAME     , false );
         enableDlgItem( IDC_BROWSEPATH   , false );
         enableDlgItem( IDC_READ_ONLY    , false );
         enableDlgItem( IDC_HIDDEN       , false );
         enableDlgItem( IDC_ARCHIVE      , false );
         enableDlgItem( IDC_SYSTEM       , false );
         enableDlgItem( IDC_COMPRESSED   , false );
         enableDlgItem( IDC_UNICODE      , false );
         enableDlgItem( IDC_UNIXLINEFEEDS, false );
         enableDlgItem( IDOK             , false );
         sendMessage( DM_SETDEFID, IDCANCEL );
         gotoDlgItem( IDCANCEL );
      } else {
         gotoDlgItem( IDC_FILENAME );
      }
      if ( !supportsCompression( m_pDocument->getPath() ) ) {
         enableDlgItem( IDC_COMPRESSED, false );
      }
      // TODO: Disable unix/unicode if read-only
   }

   return FALSE; // We DID set the focus.
}


// Is this sufficiently rigorous?
void PropertiesDlg::setFileName( const String& strPathName ) {

   PATHNAME szTitle = { 0 };
   verify( 0 == GetFileTitle( strPathName.c_str(), szTitle, dim( szTitle ) ) );

   int nPathLength = strPathName.length() - _tcslen( szTitle );
   const String strFile = strPathName.substr( nPathLength );

   // We want to display the terminating backslash
   // iff we're at the root directory:
   if ( 3 == nPathLength && _T( ':' ) == strPathName[ 1 ] ) {
      ;
   } else if ( 3 < nPathLength && _T( '\\' ) == strPathName[ nPathLength - 1 ] ) 
   {
      --nPathLength;
   }
   const String strPath = strPathName.substr( 0, nPathLength );

   setDlgItemText( IDC_FILENAME, strFile );
   setDlgItemText( IDC_PATH    , strPath );
}

// TODO: Unit test new safe string API
String PropertiesDlg::getFileName( void ) {

   const String strFile = getDlgItemText( IDC_FILENAME );
   const String strPath = getDlgItemText( IDC_PATH );
   PATHNAME szFullPath = { 0 };
   _tmakepath_s( szFullPath, 0, strPath.c_str(), strFile.c_str(), 0 );
   return szFullPath;
}


bool PropertiesDlg::onBrowse( void ) {

   String strFullPath = getFileName();
   const bool bRetVal = saveFile( *this, &strFullPath, IDS_MOVE, IDD_MOVE_CHILD );
   if ( bRetVal ) {
      setFileName( strFullPath );
   }

   return bRetVal;
}


// TODO: Editor as parm rather than document?
bool PropertiesDlg::applyChanges( void ) {

   WaitCursor waitCursor( _T( "save.ani" ) );

   bool bSuccess = true;

   assert( isGoodPtr( this ) );
   assert( isGoodPtr( m_pDocument ) );
   
   m_pDocument->setUnicode      ( 0 != Button_GetCheck( getDlgItem( IDC_UNICODE       ) ) );
   m_pDocument->setUnixLineFeeds( 0 != Button_GetCheck( getDlgItem( IDC_UNIXLINEFEEDS ) ) );

   DWORD dwAdd = 0;
   DWORD dwRemove = 0;

#define SET_ATTRIB( item, attrib ) \
   if ( Button_GetCheck( getDlgItem( item ) ) ) { \
      dwAdd |= attrib; \
   } else { \
      dwRemove |= attrib; \
   }

   SET_ATTRIB( IDC_ARCHIVE   , FILE_ATTRIBUTE_ARCHIVE    );
// SET_ATTRIB( IDC_COMPRESSED, FILE_ATTRIBUTE_COMPRESSED );
   SET_ATTRIB( IDC_READ_ONLY , FILE_ATTRIBUTE_READONLY   );
   SET_ATTRIB( IDC_HIDDEN    , FILE_ATTRIBUTE_HIDDEN     );
   SET_ATTRIB( IDC_SYSTEM    , FILE_ATTRIBUTE_SYSTEM     );

   if ( !m_pDocument->setPath( *this, getFileName() ) ) {
      bSuccess = false;
   }

   if ( supportsCompression( m_pDocument->getPath() ) ) {
      const bool bCompress = 0 != Button_GetCheck( getDlgItem( IDC_COMPRESSED ) );
      if ( !compressFile( m_pDocument->getPath(), bCompress ) ) {
         bSuccess = false;
      }
   }

   // TODO: Method in Document, or rather Editor, to do this; 
   // clear whistleclean flag!
   // Move whistleclean to Document, perhaps?
   if ( !m_pDocument->modifyAttribs( dwAdd, dwRemove ) ) {
      bSuccess = false;
   }

   HWND hwndParent = GetParent( *this );
   assert( IsWindow( hwndParent ) );
   FORWARD_WM_COMMAND( hwndParent, ID_COMMAND_PROPSCHANGED, 0, 0, SNDMSG );
   
   WIN32_FIND_DATA fd = { 0 };
   HANDLE hFind = FindFirstFile( m_pDocument->getPath().c_str(), &fd );
   if ( INVALID_HANDLE_VALUE != hFind ) {
      verify( FindClose( hFind ) );
      setInfo( fd );
   }

   return bSuccess;
}


void PropertiesDlg::onDlgCommand( 
   int id, HWND hwndCtl, UINT codeNotify ) 
{
   assert( isGoodPtr( m_pDocument ) );

   switch ( id ) {

   case IDC_BROWSEPATH:
      if ( BN_CLICKED == codeNotify ) {
         assert( !m_pDocument->isAccessDenied() );
         if ( !m_pDocument->isAccessDenied() && onBrowse() ) {
            enableDlgItem( IDC_APPLY, TRUE );
         }
      }
      break;

   case IDOK:
      if ( BN_CLICKED == codeNotify ) {
         if ( !m_pDocument->isAccessDenied() && applyChanges() ) {
            verify( EndDialog( *this, IDOK ) );
         }
      }
      break;

   case IDCANCEL:
      verify( EndDialog( *this, IDCANCEL ) );
      break;

   case IDC_FILENAME:
      if ( IsWindowVisible( *this ) && EN_CHANGE == codeNotify ) {
         assert( !m_pDocument->isAccessDenied() );
         if ( !m_pDocument->isAccessDenied() ) {
            enableDlgItem( IDC_APPLY, TRUE );
         }
      }
      break;

   case IDC_ARCHIVE:
   case IDC_COMPRESSED:
   case IDC_READ_ONLY:
   case IDC_HIDDEN:
   case IDC_SYSTEM:
   case IDC_UNICODE:
   case IDC_UNIXLINEFEEDS:
      assert( !m_pDocument->isAccessDenied() );
      if ( BN_CLICKED == codeNotify ) {
         if ( !m_pDocument->isAccessDenied() ) {
            enableDlgItem( IDC_APPLY, TRUE );
         }
      }
      break;

   case IDC_APPLY:
      if ( BN_CLICKED == codeNotify ) {
         assert( !m_pDocument->isAccessDenied() );
         if ( m_pDocument->isAccessDenied() ) {
            enableDlgItem( IDC_APPLY, false );
            MessageBeep( MB_ICONWARNING );
         } else if ( applyChanges() ) {
            setDlgItemText( IDCANCEL, loadString( IDS_CLOSE ) );
            if ( GetFocus() == getDlgItem( IDC_APPLY ) ) {
               assert( IsWindowEnabled( getDlgItem( IDOK ) ) );
               gotoDlgItem( IDOK );
            }
            enableDlgItem( IDC_APPLY, false );
         }
      }
      break;
   }
}

// end of file
