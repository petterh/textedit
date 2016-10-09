/*
 * $Header: /Book/Document.cpp 27    16.07.04 10:42 Oslph312 $
 */

#include "precomp.h"
#include "Document.h"
#include "Exception.h"
#include "AutoArray.h"
#include "AutoHandle.h"
#include "MRU.h"
#include "FileMapping.h"
#include "Registry.h"
#include "SilentErrorMode.h"
#include "FileNotFoundDlg.h"
#include "formatMessage.h"
#include "fileUtils.h"
#include "winUtils.h"
#include "openFileDlg.h"
#include "saveFile.h"
#include "persistence.h"
#include "utils.h"
#include "os.h"
#include "resource.h"
#include <aclapi.h>
#include <time.h>


bool Document::s_bEndSession = false; // Set to true on WM_ENDSESSION.

#define FILE_TYPE _T( "File Types" )

bool Document::modifyAttribs( 
   const String& strFile, DWORD dwAdd, DWORD dwRemove ) 
{
   assertValid();
   return ::modifyAttribs( strFile, dwAdd, dwRemove );
}


void Document::createOrgCopy( HANDLE hIn ) {

   assertValid();
   assert( 0 != hIn );

   String strOrgCopy = getOrgCopy();
   if ( !strOrgCopy.empty() && fileExists( strOrgCopy.c_str() ) ) {
      return;
   }

   strOrgCopy = getTempFileName();
   setOrgCopy( strOrgCopy );
   verify( modifyAttribs( strOrgCopy, FILE_ATTRIBUTE_HIDDEN ) );

   // This block provides a scope for the AutoHandle, so that 
   // it closes before we modify the file attributes.
   {
      AutoHandle hOrgCopy( 
         CreateFile( strOrgCopy.c_str(), GENERIC_WRITE,
            FILE_SHARE_NONE, 0, OPEN_EXISTING, 0, 0 ) );
      if ( INVALID_HANDLE_VALUE == hOrgCopy ) {
         throwException( String( 
            _T( "Can't open temp file for original copy " ) ) + 
            strOrgCopy );
      }
      
      copyFile( hIn, hOrgCopy );
   }
   verify( modifyAttribs( strOrgCopy, FILE_ATTRIBUTE_READONLY ) );

   // NOTE: The alleged documentation says nothing about what kind
   // of string is expected -- ANSI or Unicode! If you look at the
   // definition of SHCNF_PATH, though, you will find the answer.
   SHChangeNotify( SHCNE_CREATE | SHCNE_FREESPACE, 
      SHCNF_PATH, strOrgCopy.c_str(), 0 );
}


// TODO: In case of device not ready, options are:
// retry, cancel, select a different file
HANDLE Document::openFile( HWND hwnd )
{
   assertValid();
   DWORD dwErr = 0;
   HANDLE hIn = ::openFile( m_strFileName.c_str(), GENERIC_READ_WRITE, FILE_SHARE_READ );
   if ( INVALID_HANDLE_VALUE == hIn )
   {
      dwErr = GetLastError();
      if ( ERROR_INVALID_NAME == dwErr )
      {
         messageBox( hwnd, MB_OK | MB_ICONINFORMATION, IDS_WILDCARDS, m_strFileName.c_str() );
         PATHNAME szNewPath = { 0 };
         const bool bOpenOK = ::openFileDlg( hwnd, szNewPath, dim( szNewPath ), 0, false );
         if ( bOpenOK )
         {
            m_strFileName = szNewPath;
            hIn = ::openFile( m_strFileName.c_str(), GENERIC_READ_WRITE, FILE_SHARE_READ );
         }
         else
         {
            return INVALID_HANDLE_VALUE;
         }
      }
      else if (ERROR_FILE_NOT_FOUND == dwErr)
      {
         String strFileNameWithExtension = formatMessage(_T( "%1.txt" ), m_strFileName.c_str());
         hIn = ::openFile( strFileNameWithExtension.c_str(), GENERIC_READ_WRITE, FILE_SHARE_READ );
         dwErr = GetLastError();
         if ( ERROR_FILE_NOT_FOUND != dwErr )
         {
            m_strFileName = strFileNameWithExtension;
         }
      }
      else if ( ERROR_DEVICE_DOOR_OPEN == dwErr )
      {
         // ERROR_DEVICE_NOT_AVAILABLE ERROR_NOT_READY
      }
   }

   if ( INVALID_HANDLE_VALUE == hIn )
   {
      // Try to open as read-only.
      // What other errors could reasonably be handled here?
      if ( ERROR_ACCESS_DENIED == dwErr || ERROR_SHARING_VIOLATION == dwErr )
      {
         hIn = ::openFile( m_strFileName.c_str(), GENERIC_READ, FILE_SHARE_READ );
         if ( INVALID_HANDLE_VALUE == hIn )
         {
             hIn = ::openFile( m_strFileName.c_str(), GENERIC_READ, FILE_SHARE_READ_WRITE );
         }

         if ( INVALID_HANDLE_VALUE == hIn )
         {
            dwErr = GetLastError();
         }
         else
         {
            m_isReadOnly = true;
            const DWORD dwAttribs = GetFileAttributes( m_strFileName.c_str() );
            if ( 0 == ( dwAttribs & FILE_ATTRIBUTE_READONLY ) )
            {
               m_bAccessDenied = true;
            }
         }
      }
   }

   if ( INVALID_HANDLE_VALUE == hIn && ERROR_FILE_NOT_FOUND == dwErr )
   {
       hIn = getNewFile( hwnd, &m_strFileName );
   }

   if ( INVALID_HANDLE_VALUE == hIn )
   {
      throwException( m_strFileName.c_str(), dwErr ); 
   }

   return hIn;
}


Document::Document( HWND hwnd, LPCTSTR pszFile )
   : m_strFileName( _T( "" ) )
   , m_isReadOnly( false )
   , m_hasUnixLineFeeds( false )
   , m_bAccessDenied( false )
   , m_bLockedByOtherProcess( false )
   , m_bDirty( false )
{
   assertValid();
   assert( isGoodStringPtr( pszFile ) );

   m_strFileName = getLongPathName( pszFile );
   m_uiDriveType = getDriveType( m_strFileName );
   // Returns DRIVE_NO_ROOT_DIR for UNC paths...?!?

   if ( DRIVE_CDROM == m_uiDriveType ) {
      m_isReadOnly = true;
      m_bAccessDenied = true;
   } else if ( DRIVE_REMOVABLE == m_uiDriveType ) {
      if ( isWriteProtectedDisk( m_strFileName.c_str() ) ) {
         m_isReadOnly = true;
         m_bAccessDenied = true;
      }
   }

   AutoHandle hIn( openFile( hwnd ) );
   assert( INVALID_HANDLE_VALUE != hIn );

   BY_HANDLE_FILE_INFORMATION fileInfo = { 0 };
   const BOOL bOK = GetFileInformationByHandle( hIn, &fileInfo );
   if ( !bOK && !m_strFileName.empty() ) {
      throwException( _T( "GetFileInformationByHandle failed" ) );
   }

   createOrgCopy( hIn );
   setRunning( 1 );
}


PRIVATE HANDLE openExistingFileOrCreateNew( const String& strFile ) {

   HANDLE hOut = CreateFile( strFile.c_str(), 
      GENERIC_READ_WRITE, FILE_SHARE_READ, 0, 
      TRUNCATE_EXISTING, 0, 0 );
   if ( INVALID_HANDLE_VALUE == hOut ) {
      const DWORD dwErr = GetLastError();
      if ( ERROR_FILE_NOT_FOUND == dwErr ) { // TODO: Handle access denied... (5)
         hOut = CreateFile( strFile.c_str(), 
            GENERIC_READ_WRITE, FILE_SHARE_READ, 0, 
            CREATE_NEW, 0, 0 );
      }
   }
   return hOut;
}


int countNewLinesLackingCRs( LPCTSTR psz ) {
   
   assert( isGoodStringPtr( psz ) );

#ifdef _DEBUG
   const DWORD dwStartTime = GetTickCount();
#endif

   int nCount = 0;
   LPCTSTR pszSrc = psz;
   while ( 0 != (pszSrc = _tcschr( pszSrc, _T( '\n' ) ) ) ) {
      const LPTSTR pszPrev = charPrev( psz, pszSrc );
      if ( psz == pszSrc || _T( '\r' ) != *pszPrev ) {
         ++nCount;
      }
      pszSrc = charNext( pszSrc );
   }

#ifdef _DEBUG
   const DWORD dwSearchTime = GetTickCount() - dwStartTime;
   trace( _T( "countNewLinesLackingCRs: %u.%03u seconds (%s) \n" ),
      dwSearchTime / 1000, dwSearchTime % 1000, _T( "MS_DOS" ) );
#endif

   return nCount;
}


/**
 * Translate line feeds and remember what they were, 
 * Unix-style (single \n) or DOS-style (\r\n).
 * This is not really necessary for the Rich Edit control.
 */
void Document::addCRs( LPTSTR *ppsz ) throw( MemoryException ) {

#if 0
   bool bHadNewLine   = false;
   bool bHadReturn    = false;
   bool bMaybeUnix    = false;
   bool bMaybeNotUnix = false;
#endif

   assertValid();
   assert( 0 != ppsz );
   assert( 0 != *ppsz );
   assert( isGoodStringPtr( *ppsz ) );

   int nOldLength = _tcslen( *ppsz );
   int nFixes = countNewLinesLackingCRs( *ppsz );
   m_hasUnixLineFeeds = 0 < nFixes;
   if ( 0 == nFixes ) {
      return;
   }

   const int nNewLength = nOldLength + nFixes;
   LPTSTR pszNew = new TCHAR[ nNewLength + 1 ];
   LPTSTR pszDst = pszNew + nNewLength;
   *pszDst = 0;

   LPCTSTR pszStart = *ppsz + nOldLength;
   LPCTSTR pszSrc = pszStart;

#ifdef _DEBUG
   const DWORD dwStartTime = GetTickCount();
#endif

   for ( ;; ) {
      // This invariant may not hold for multi-byte character sets.
      assert( pszDst - pszNew == pszSrc - *ppsz + nFixes );
      do {
         pszSrc = charPrev( *ppsz, pszSrc );
         pszDst = charPrev( pszNew, pszDst );
         if ( pszSrc < *ppsz ) {
            assert( false );
            break;
         }
      } while ( _T( '\n' ) != *pszSrc );
      
      const LPTSTR pszPrev = charPrev( *ppsz, pszSrc );
      if ( *ppsz == pszSrc || _T( '\r' ) != *pszPrev ) {
         const int nLength = pszStart - pszSrc;
         //pszDst -= nLength;
         memcpy( pszDst, pszSrc, nLength * sizeof( TCHAR ) );
         pszDst = charPrev( pszNew, pszDst );
         *pszDst = _T( '\r' );
         --nFixes;
         pszStart = pszSrc;
         if ( nFixes <= 0 ) {
            assert( 0 == nFixes );
            break; //*** LOOP EXIT POINT
         }
      }
   }

   if ( *ppsz < pszSrc ) {
      const int nLength = pszStart - *ppsz;
      assert( pszNew + nLength == pszDst );
      memcpy( pszNew, *ppsz, nLength * sizeof( TCHAR ) );
   }

   // This invariant may not hold for multi-byte character sets.
   assert( pszDst - pszNew == pszSrc - *ppsz );

   delete[] *ppsz;
   *ppsz = pszNew;

#ifdef _DEBUG
   const DWORD dwReplaceTime = GetTickCount() - dwStartTime;
   trace( _T( "addCRs timing: %u.%03u seconds (%s) \n" ),
      dwReplaceTime / 1000, dwReplaceTime % 1000, _T( "UNIX" ) );
#endif
}


/**
 * The size of the buffer remains the same, so if n CRs are removed,
 * there will be n unused characters at the end of the string.
 */
void Document::removeCRs( LPTSTR psz ) {

   assertValid();

#ifdef _DEBUG
   DWORD dwTime = GetTickCount();
#endif

   LPCTSTR pszSrc = psz;
   LPTSTR  pszDst = psz;
   LPCTSTR pszCR;
   while ( 0 != (pszCR = _tcschr( pszSrc, _T( '\r' ) ) ) ) {
      const int nLineLength = pszCR - pszSrc + 1;
      memmove( pszDst, pszSrc, nLineLength * sizeof( TCHAR ) );
      pszDst += nLineLength;
      pszSrc += nLineLength;
      if ( _T( '\n') == *pszSrc ) {
         assert( _T( '\r' ) == *charPrev( psz, pszSrc ) );
         LPTSTR pszPrev = charPrev( psz, pszDst );
         assert( pszPrev < pszDst );
         assert( _T( '\r' ) == *pszPrev );
         *pszPrev = _T( '\n' );
         pszSrc = charNext( pszSrc );
      }
   }

   const int nLength = _tcslen( pszSrc ) + 1;
   assert( 1 <= nLength ); // Includes terminator.
   memmove( pszDst, pszSrc, nLength * sizeof( TCHAR ) );

#ifdef _DEBUG
   dwTime = GetTickCount() - dwTime;
   trace( _T( "removeCRs: %u.%03u seconds\n" ),
         dwTime / 1000, dwTime % 1000 );
#endif
}


/**
 * Saves text to disk, and is therefore critical.
 */
void Document::save( 
   HWND hwnd, const void *pRawContents, int nBytes ) throw()
{
   assertValid();
   assert( isGoodReadPtr( pRawContents, nBytes ) );

   SilentErrorMode sem;

   AutoHandle hOut( 
      openExistingFileOrCreateNew( m_strFileName ) );

LABEL_Retry:
   if ( INVALID_HANDLE_VALUE == hOut ) {
      debugBreak();
      String strFileName( m_strFileName );
      while ( saveFile( GetLastActivePopup( hwnd ), &strFileName, 
                        IDS_SAVEFILE, IDD_SAVE_CHILD ) ) 
      {
         hOut = openExistingFileOrCreateNew( strFileName );
         if ( INVALID_HANDLE_VALUE != hOut ) {
            setPath( hwnd, strFileName );
            break;
         }
      }
      if ( INVALID_HANDLE_VALUE == hOut ) {
         throwException( _T( "Unable to save file." ) );
      }
   }

   if ( 0 < nBytes ) {
      try {
         FileMapping fileMapping( hOut, nBytes );
         memcpy( fileMapping, pRawContents, nBytes );
      }
      catch ( const WinException& x ) {
         verify( CloseHandle( hOut ) );
         hOut = INVALID_HANDLE_VALUE;
         x.resetLastError();
         goto LABEL_Retry;
      }
      catch ( ... ) {
         verify( CloseHandle( hOut ) );
         hOut = INVALID_HANDLE_VALUE;
         messageBox( 
            hwnd, MB_ICONERROR | MB_OK, IDS_SAVE_EXCEPTION );
         goto LABEL_Retry;
      }
   }
}


/**
 * NOTE: nLength may be larger than the string length.
 * Optimizations are possible; if no conversions are required,
 * we could even getText directly to the memory-mapped file.
 * NOTE: Buffer sizes etc. have not been tested with multi-byte
 * character sets.
 * NOTE: Abandon changes can be simplified 
 * if no changes were saved to disk.
 */
void Document::update(
   HWND hwnd, LPTSTR pszNewContents, int nLength )
{
   assertValid();
   assert( isGoodStringPtr( pszNewContents ) );
   assert( (int) _tcsclen( pszNewContents ) <= nLength );

   trace( _T( "saving [%s]...\n" ), getPath().c_str() );
   if ( hasUnixLineFeeds() ) {
      removeCRs( pszNewContents );
   }
   
   const int nChars = _tcsclen( pszNewContents );
   const int nBytesPerChar = // Warning: Not OK for multi-byte char sets!
      isUnicode() ? sizeof( WCHAR ) : sizeof( char );
   const int nBytes = nChars * nBytesPerChar;

   if ( m_isUnicode ) {
      save( hwnd, pszNewContents, nBytes );
   } else {
      AutoStringA pszBuffer( new char[ nChars + 1 ] );
      wideCharToMultiByte( pszNewContents, pszBuffer, nChars );
      save( hwnd, pszBuffer, nBytes );
   }
}


String Document::getTitle( void ) const {

   assertValid();
   PATHNAME szTitle = { 0 };
   const LPCTSTR pszPath = getPath().c_str();
   verify( 0 == GetFileTitle( pszPath, szTitle, dim( szTitle ) ) );
   return szTitle;
}

bool Document::setPath( HWND hwnd, const String& strNewPath ) {
   assertValid();
   bool bSuccess = true;
   if ( !areFileNamesEqual( getPath(), strNewPath ) ) {
   
      // The string arrays are needed for SHFILEOPSTRUCT.
      // The pFrom and pTo members are actually lists of
      // null-terminated file names; the list itself must be
      // doubly null-terminated.

      TCHAR szOldPath[ MAX_PATH + 2 ] = { 0 };
      _tcscpy_s( szOldPath, getPath().c_str() );

      TCHAR szNewPath[ MAX_PATH + 2 ] = { 0 };
      _tcscpy_s( szNewPath, strNewPath.c_str() );

      SHFILEOPSTRUCT shFileOpStruct = {
         hwnd, FO_MOVE, szOldPath, szNewPath, FOF_SIMPLEPROGRESS,
      };
      const int nErr = SHFileOperation( &shFileOpStruct );
      if ( 0 == nErr ) {
         MRU mru;
         mru.renameFile( m_strFileName, strNewPath );
         m_strFileName = strNewPath;
      } else {
         bSuccess = false;
      }
   }
   return bSuccess;
}


/**
 * Sets the original copy to 'readable' and delete it.
 * If modifyAttribs fails, there's not a lot we can do
 * about it, so we just try to delete the file anyway.
 */
void Document::deleteOrgCopy( void ) {

   assertValid();
   const String strOrgCopy = getOrgCopy();
   modifyAttribs( strOrgCopy, 0, FILE_ATTRIBUTE_READONLY );

   if ( DeleteFile( strOrgCopy.c_str() ) ) {
      SHChangeNotify( SHCNE_DELETE | SHCNE_FREESPACE, 
         SHCNF_PATH, strOrgCopy.c_str(), 0 );
   } else {
      trace( _T( "deleteOrgCopy( %s ) failed: %s\n" ), 
         strOrgCopy.c_str(), getError().c_str() );
   }
   setOrgCopy( _T( "" ) );
}


Document::~Document() {

   assertValid();
   setRunning( s_bEndSession );
   if ( !s_bEndSession ) {
      deleteOrgCopy();
   }
}


/**
 * The dwBytes count does *not* include the terminating 0.
 */
LPTSTR Document::convert( const LPVOID pbRawFile, DWORD dwBytes ) {

   assertValid();
   int nFlags = IS_TEXT_UNICODE_UNICODE_MASK;
   m_isUnicode = 0 != isTextUnicode( pbRawFile, dwBytes, &nFlags );

   trace(L"file is %s\n", m_isUnicode ? L"UNICODE" : L"ANSI");

   m_hasUnicodeTranslationError = false;
   m_bBinary = m_isUnicode && 0 != dwBytes % 2;
   if ( m_bBinary ) {
      m_isUnicode = false;
   }

   DWORD dwChars = dwBytes;
   if ( m_isUnicode ) {
      assert( 0 == dwBytes % 2 );
      dwChars = dwBytes / 2;
   }

   LPTSTR pszConvertedContents = new TCHAR[ dwChars + 1 ];
   assert( 0 != pszConvertedContents );

   pszConvertedContents[ dwChars ] = 0;

   if ( m_isUnicode ) {
      memcpy( pszConvertedContents, pbRawFile, dwBytes );
   } else {
      multiByteToWideChar(
         reinterpret_cast< LPCSTR >( pbRawFile ),
         pszConvertedContents, dwChars );
   }

   for ( int iChar = 0; iChar < (int) dwChars; ++iChar ) {
      if ( 0 == pszConvertedContents[ iChar ] ) {
         m_bBinary = true;
         pszConvertedContents[ iChar ] = NULL_REPLACEMENT;
      }
   }

   assert( 0 == pszConvertedContents[ dwChars ] );
   pszConvertedContents[ dwChars ] = 0;
   return pszConvertedContents;
}


/**
 * This function allocates memory that the caller must release.
 * The AutoString template can be used to automate this.
 * This function never returns 0, but throws an exception on failure.
 */
LPTSTR Document::getContents( 
   const String& strFile, int *pnBytes = 0 ) 
{
   assertValid();
   assert( 0 == pnBytes || isGoodPtr( pnBytes ) );

   AutoHandle hFile( CreateFile( strFile.c_str(), GENERIC_READ,
      FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0 ) );
   if ( INVALID_HANDLE_VALUE == hFile ) {
      hFile = CreateFile( strFile.c_str(), GENERIC_READ,
         FILE_SHARE_READ_WRITE, 0, OPEN_EXISTING, 0, 0 );
   }
   if ( INVALID_HANDLE_VALUE == hFile ) {
      throwException( _T( "Unable to open " ) + strFile );
   }

   DWORD dwHiBytes = 0;
   const DWORD dwLoBytes = GetFileSize( hFile, &dwHiBytes );
   const DWORD dwErr = GetLastError();
   if ( 0xFFffFFff == dwLoBytes && NO_ERROR != dwErr ) {
      throwException( _T( "Unable to get file size" ), dwErr );
   }
   if ( 0 < dwHiBytes || INT_MAX < dwLoBytes ) {
      throwException( 
         _T( "Cannot edit files larger than 2GB" ), dwErr );
   }

   LPTSTR pszContents = 0;
   if ( 0 == dwLoBytes ) {
      pszContents = convert( _T( "" ), 0 );
   } else {
      const FileMapping fileMapping( hFile );
      pszContents = convert( fileMapping, dwLoBytes );
   }

   if ( 0 != pnBytes ) {
      *pnBytes = dwLoBytes;
   }

   assert( isGoodStringPtr( pszContents ) );
   return pszContents;
}


LPTSTR Document::getContents( int *pnBytes ) {

   assertValid();
   assert( 0 == pnBytes || isGoodPtr( pnBytes ) );

   return getContents( getPath(), pnBytes );
}


LPTSTR Document::getOrgContents( int *pnBytes ) {

   assertValid();
   assert( 0 == pnBytes || isGoodPtr( pnBytes ) );

   return getContents( getOrgCopy(), pnBytes );
}


/**
 * Create a unique registry path for a document. 
 * To avoid confusion with the registry's
 * backslash usage, all path delimiters are replaced by bars.
 * The bar (|) is not a valid file name character, 
 * but it is OK in a registry key.
 */
String Document::createRegistryPath( const String& strRealPath ) {

   assert( isGoodConstPtr( &strRealPath ) );
   String strPath( strRealPath );
   int iPos = -1;
   for ( ;; ) {
      iPos = strPath.find_first_of( _T( "\\/" ), iPos + 1 );
      if ( iPos < 0 ) {
         break; // ** LOOP EXIT POINT
      }
      strPath.replace( iPos, 1, 1, _T( '|' ) );
   }
   return strPath;
}


/**
 * Private function to create a unique registry path for
 * this document. See the static getRegistryPath above.
 */
String Document::getRegistryPath( void ) const {
   
   assertValid();
   const String strRegistryPath = createRegistryPath( getPath() );
   return formatMessage( _T( "Files\\%1" ), strRegistryPath.c_str() );
}

String Document::getFileTypeDescription( bool bDisplay ) const {

   assertValid();
   TCHAR szExt[ _MAX_EXT ] = { 0 };
   _tsplitpath_s( getPath().c_str(), 0, 0, 0, 0, 0, 0, szExt, _MAX_EXT );
   String strFileTypeDescription = Registry::fileTypeDescriptionFromExtension( szExt );
   if ( !bDisplay && strFileTypeDescription.empty() ) {
      strFileTypeDescription.assign( 0 == szExt[ 0 ] ? _T( "empty" ) : szExt );
   }
   return strFileTypeDescription;
}


String Document::getRegistryFileTypePath( void ) const {

   assertValid();
   return formatMessage( 
      FILE_TYPE _T( "\\%1" ), getFileTypeDescription().c_str() );
}


int Document::getPersistentInt( 
   LPCTSTR pszName, int nDefault, bool bType ) const 
{
   assertValid();
   if ( bType ) {
      const String strTypeKey( getRegistryFileTypePath() );
      nDefault = Registry::getInt( 
         HKEY_CURRENT_USER, strTypeKey.c_str(), pszName, nDefault );
   }
   const String strFileKey( getRegistryPath() );
   return Registry::getInt( 
      HKEY_CURRENT_USER, strFileKey.c_str(), pszName, nDefault );
}


void Document::setPersistentInt( 
   LPCTSTR pszName, int nValue, bool bType ) 
{
   assertValid();
   const String strFileKey( getRegistryPath() );
   Registry::setInt( 
      HKEY_CURRENT_USER, strFileKey.c_str(), pszName, nValue );
   if ( bType ) {
      const String strTypeKey( getRegistryFileTypePath() );
      Registry::setInt( 
         HKEY_CURRENT_USER, strTypeKey.c_str(), pszName, nValue );
   }
}



String Document::getPersistentString( LPCTSTR pszName ) const {
   
   assertValid();
   const String strFileKey( getRegistryPath() );
   return Registry::getString( 
      HKEY_CURRENT_USER, strFileKey.c_str(), pszName );
}


void Document::setPersistentString( 
   LPCTSTR pszName, const String& str ) 
{
   assertValid();
   const String strFileKey( getRegistryPath() );
   Registry::setString( 
      HKEY_CURRENT_USER, strFileKey.c_str(), pszName, str.c_str() );
}

bool Document::deleteFile( HWND hwnd ) {

   assertValid();
   FILEOP_FLAGS fileOpFlags = FOF_SIMPLEPROGRESS;
   if ( getSendToWasteBasket() ) {
      fileOpFlags |= FOF_ALLOWUNDO;
   }

   // The pFrom member of SHFILEOPSTRUCT is actually a list
   // of null-terminated file names; the list itself must be
   // doubly null-terminated. This is the reason for using the 
   // szFileName buffer rather than getPath().c_str.
   TCHAR szFileName[ MAX_PATH + 2 ] = { 0 };
   size_t charsToCopy = std::min( dim( szFileName ),  1 + getPath().length() );
   _tcscpy_s( szFileName, charsToCopy, getPath().c_str() );
   SHFILEOPSTRUCT shFileOpStruct = {
      hwnd, FO_DELETE, szFileName, _T( "\0" ), fileOpFlags,
   };
   const int nErr = SHFileOperation( &shFileOpStruct );
   if ( NOERROR != nErr ) {
      throw WinException( _T( "Unable to delete file" ), nErr );
   }
   if ( shFileOpStruct.fAnyOperationsAborted ) {
      trace( _T( "Didn't delete file %s\n" ), getPath().c_str() );
      return false;
   } 

   m_bDirty = false;
   MRU mru;
   mru.removeFile( getPath() );

   trace( _T( "Deleted file %s, and removed from MRU list\n" ), 
      getPath().c_str() );

   return true;
}

// end of file
