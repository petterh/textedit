/*
 * $Header: /Cleaner/fileUtils.cpp 21    22.03.02 12:51 Oslph312 $
 */

#include "precomp.h"
#include "Exception.h"
#include "AutoArray.h"
#include "AutoHandle.h"
#include "AutoShellObject.h"
#include "ClientDC.h"
#include "SilentErrorMode.h"
#include "fileUtils.h"
#include "os.h"


#pragma comment( lib, "shlwapi.lib"  )


// nonstandard extension used: nameless struct/union
#pragma warning( disable: 4201 ) 
#include <winioctl.h>
#pragma warning( default: 4201 ) 


/**
 * Simplified version of CreateFile, so as not to 
 * clutter up the functional code with irrelevancies.
 * Discuss: Loop on critical error conditions!
 */
HANDLE openFile( 
   LPCTSTR pszFile, DWORD dwDesiredAccess, DWORD dwShareMode ) 
{
   HANDLE hFile = INVALID_HANDLE_VALUE;
   for ( ;; ) {
      hFile = CreateFile( pszFile, 
         dwDesiredAccess, dwShareMode, 0, OPEN_EXISTING, 0, 0 );
      if ( INVALID_HANDLE_VALUE != hFile ) {
         break;
      }
      const DWORD win_error = GetLastError();
      if ( ERROR_NOT_READY != win_error ) {
         break;
      }
#ifdef IDS_DEVICE_NOT_READY
      const UINT uiResult = messageBox(
         HWND_DESKTOP, // TODO -- window handle parm? Don't do this while silent!
         MB_RETRYCANCEL | MB_ICONINFORMATION,
         IDS_DEVICE_NOT_READY, pszFile );
#else
      const UINT uiResult = messageBox(
         HWND_DESKTOP, // TODO -- window handle parm? Don't do this while silent!
         MB_RETRYCANCEL | MB_ICONINFORMATION,
         _T( "Device not ready reading file %s" ), pszFile );
#endif
      if ( IDCANCEL == uiResult ) {
         break;
      }
   }
   return hFile;
}


// TODO: Return 'not ready'
bool fileExists( LPCTSTR pszFile ) {

   assert( isGoodStringPtr( pszFile ) );

   SilentErrorMode sem;
   AutoHandle hFile( 
      openFile( pszFile, GENERIC_READ, FILE_SHARE_READ_WRITE ) );
   return INVALID_HANDLE_VALUE != hFile;
}


bool modifyAttribs( 
   const String& strFile, DWORD dwAdd, DWORD dwRemove ) 
{
   SilentErrorMode sem;

#ifdef _DEBUG
   const DWORD FILE_ATTRIB_MASK =
      FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN   | 
      FILE_ATTRIBUTE_SYSTEM   | FILE_ATTRIBUTE_ARCHIVE  ; 
   assert( 0 == (dwAdd    & ~FILE_ATTRIB_MASK) );
   assert( 0 == (dwRemove & ~FILE_ATTRIB_MASK) );
#endif // _DEBUG

   DWORD dwAttribs = GetFileAttributes( strFile.c_str() );
   if ( (DWORD) -1 == dwAttribs ) {
      trace( _T( "%s\n" ), getError( 
         _T( "modifyAttribs: GetFileAttributes failed" ) ).c_str() );
      return false;
   }

   dwAttribs |= dwAdd;
   dwAttribs &= ~dwRemove;

   if ( !SetFileAttributes( strFile.c_str(), dwAttribs ) ) {
      trace( _T( "%s\n" ), getError( 
         _T( "modifyAttribs: SetFileAttributes failed" ) ).c_str() );
      return false;
   }

   return true;
}


/**
 * CopyFile or CopyFileEx instead?
 */
void copyFile( HANDLE hSrc, HANDLE hDst, DWORD *pdwBytes ) {

   const DWORD uiBufLen = getGoodIOBufferSize();
   AutoArray< BYTE > pBuf( new BYTE[ uiBufLen ] );

   if ( 0 != pdwBytes ) {
      *pdwBytes = 0;
   }

   for ( ;; ) {
      DWORD dwBytesRead = 0;
      DWORD dwBytesWritten = 0;

      if ( !ReadFile( hSrc, pBuf, uiBufLen, &dwBytesRead, 0 ) ) {
         const DWORD win_error = GetLastError();
         if ( ERROR_BROKEN_PIPE != win_error ) {
            throwException( _T( "Failed to read standard input" ) );
         }
      }

      if ( 0 == dwBytesRead ) {
         break; // *** LOOP EXIT POINT
      }

      if ( 0 != pdwBytes ) {
         *pdwBytes += dwBytesRead;
      }

      const BOOL bOK =
         WriteFile( hDst, pBuf, dwBytesRead, &dwBytesWritten, 0 );
      if ( !bOK ) {
         throwException( _T( "WriteFile failed" ) );
      }
      assert( dwBytesRead == dwBytesWritten );
   }
}


PRIVATE String getWinInit( void ) {

   PATHNAME szWinInit = { 0 };
   GetWindowsDirectory( szWinInit, sizeof szWinInit );
   int nLength = _tcsclen( szWinInit );
   if ( 0 < nLength ) {
      if ( !isPathSeparator( szWinInit[ nLength - 1 ] ) ) {
         _tcscat( szWinInit, _T( "\\" ) );
      }
      _tcscat( szWinInit, _T( "WININIT.INI" ) );
   }

   return szWinInit;
}


typedef BOOL (WINAPI *MOVEFILEEXPROC)(LPCTSTR, LPCTSTR, DWORD );

/**
 * If this function fails, consult GetLastError.
 */
bool delayedRemove( const String& strPath ) {

   if ( isWindowsNT() ) {
      HMODULE hmodule = GetModuleHandle( _T( "KERNEL32" ) );
      MOVEFILEEXPROC fMoveFileEx = (MOVEFILEEXPROC) GetProcAddress( 
         hmodule, 

         // Find the correct name for ANSI or Unicode.
         // Note that GetProcAddress is *never* a Unicode function;
         // the function name is an LPCSTR rather than an LPCTSTR.
#ifdef UNICODE
         "MoveFileExW"
#else
         "MoveFileExA"
#endif
         );
      if ( 0 == fMoveFileEx ) {
         trace( _T( "Failed to get MoveFileEx proc address\n" ) );
         return false;
      }

      assert( fMoveFileEx == MoveFileEx );
      return 0 != fMoveFileEx( 
         strPath.c_str(), 0, MOVEFILE_DELAY_UNTIL_REBOOT );
   }

   // Windows 9x:
   // We can't, unfortunately, use the WritePrivateProfileString
   // function, as all the NUL= strings will be overwritten.

   PATHNAME szPath = { 0 };
   const DWORD dwLength = GetShortPathName( 
      strPath.c_str(), szPath, dim( szPath ) );
   if ( 0 == dwLength || dim( szPath ) < dwLength ) {
      return false;
   }
   assert( dwLength == _tcsclen( szPath ) );

   String strWinInit = getWinInit();
   if ( strWinInit.empty() ) {
      return false;
   }

   PATHNAME szTempPath = { 0 };
   if ( 0 == GetTempPath( dim( szTempPath ), szTempPath ) ) {
      return false;
   }

   PATHNAME szTempFile = { 0 };
   const UINT uiUniqueNumber = 
      GetTempFileName( szTempPath,  _T( "te" ), 0, szTempFile );
   if ( 0 == uiUniqueNumber ) {
      return false;
   }

   if ( !CopyFile( strWinInit.c_str(), szTempFile, FALSE ) ) {
      const DWORD win_error = GetLastError();
      if ( ERROR_FILE_NOT_FOUND != win_error ) {
         return false;
      }
      // Otherwise, continue. We'll be OK.
   }

   bool bInserted = false;
   FILE *fileOut = _tfopen( strWinInit.c_str(), _T( "w" ) );
   if ( 0 != fileOut ) {
      FILE *fileIn = _tfopen( szTempFile, _T( "r" ) );
      if ( 0 != fileIn ) {
         // Assume not Unicode:
         char szLine[ 5000 ] = { 0 };
         while ( fgets( szLine, sizeof szLine, fileIn ) ) {
            const bool bFound = 0 != strstr( szLine, "[rename]" );
            fputs( szLine, fileOut );
            if ( !bInserted && bFound ) {
               fprintf( fileOut, "NUL=%s\n", szPath );
               bInserted = true;
            }
         }
         fclose( fileIn );
         reset_pointer( fileIn );
      }
      if ( !bInserted ) {
         fprintf( fileOut, "[rename]\nNUL=%s\n", szPath );
         bInserted = true;
      }
      fclose( fileOut );
      reset_pointer( fileOut );
   }

   verify( DeleteFile( szTempFile ) );
   return bInserted;
}


/**
 * Parses a stream name, if one exists:
 * "C:\Streams\demo.txt:MyStream" 
 *    -> "C:\Streams\demo.txt" and "MyStream"
 */
bool extractStream( 
   const String& strPath, String *pstrBase, String *pstrStream ) 
{
   assert( isGoodPtr( pstrBase ) );
   assert( isGoodPtr( pstrStream ) );

   // Assume no stream component:
   *pstrBase = strPath;
   pstrStream->empty();

   int nColon = strPath.find( _T( ':' ), 2 );
   if ( nColon < 2 ) {
      return false;
   }

   pstrBase->assign( strPath, 0, nColon );
   pstrStream->assign( strPath, nColon + 1, strPath.length() );

   return true;
}


String getRootDir( const String& strPath ) {

   PATHNAME szRootDir = { 0 };

   verify( _tcscpy( szRootDir, strPath.c_str() ) );
   if ( !PathStripToRoot( szRootDir ) ) {
      // NOTE: This fails with named streams.
      trace( _T( "PathStripToRoot( %s ) failed: %s\n" ), 
         strPath.c_str(), getError().c_str() );

      // _tsplitpath is no good for UNC path names!
      // _tsplitpath( pszPath, szRootDir, 0, 0, 0 );
      // StrCat( szRootDir, _T( "\\" ) );
      
      _tcscpy( szRootDir, strPath.c_str() );
      LPTSTR psz = _tcsstr( szRootDir, _T( "\\\\" ) );
      if ( psz == szRootDir ) {
         psz = _tcschr( psz + 2, _T( '\\' ) );
         if ( 0 != psz ) {
            psz = _tcschr( psz + 1, _T( '\\' ) );
         }
      } else {
         psz = _tcschr( szRootDir, _T( '\\' ) );
      }
      if ( 0 != psz ) {
         assert( _T( '\\' ) == *psz );
         psz[ 1 ] = 0;
      }
   }

   return szRootDir;
}


DWORD getClusterSize( const String& strPath ) {

   BOOL bPathOK;
   DWORD dwBytesPerSector = 0;
   DWORD dwSectorsPerCluster = 0;
   
   const String strRoot = getRootDir( strPath );

   DWORD dwNumberOfFreeClusters = 0;
   DWORD dwTotalNumberOfClusters = 0;
   bPathOK = GetDiskFreeSpace( strRoot.c_str(), 
      &dwSectorsPerCluster, &dwBytesPerSector, 
      &dwNumberOfFreeClusters, &dwTotalNumberOfClusters );
   
   if ( !bPathOK ) {
      trace( _T( "Can't get sector size on %s\n" ), strRoot.c_str() );
   }
   
#ifdef _DEBUG
   trace( _T( "dwBytesPerSector = %lu\n" )
      _T( "dwSectorsPerCluster = %lu\n" ), 
      dwBytesPerSector, dwSectorsPerCluster );
#endif
   
   return dwBytesPerSector * dwSectorsPerCluster;
}


void addPathSeparator( String *pstrPath ) {

   assert( 0 != pstrPath );
   assert( !pstrPath->empty() );

   const int nLength = pstrPath->length();
   if ( 0 < nLength && 
        !isPathSeparator( (*pstrPath)[ nLength - 1 ] ) ) 
   {
      *pstrPath += _T( '\\' );
   }
}


String& appendPathComponent( String *pPath, LPCTSTR component ) {

    addPathSeparator( pPath );
    *pPath += component;
    return *pPath;
}


String compactPath( LPCTSTR pszPath, int nWidth, HFONT hfont ) {

   // PathCompactPath has a bug that causes it to trash
   // up to three bytes in front of the string under some
   // circumstances. This gives it some elbow room:
   const int nBugProtection = 4;

   TCHAR szCompactPath[ nBugProtection + MAX_PATH + 1 ] = { 0 };
   LPTSTR pszCompactPath = szCompactPath + nBugProtection;
   _tcscpy( pszCompactPath, pszPath );

   ClientDC dc;
   if ( dc.isValid() ) {
      HFONT hfontSaved = SelectFont( dc, hfont );
      PathCompactPath( dc, pszCompactPath, nWidth );
      SelectFont( dc, hfontSaved );
   }

#if defined( _DEBUG ) && 0
   for ( int i = 0; i < nBugProtection; ++i ) {
      if ( 0 != szCompactPath[ i ] ) {
         trace( _T( "Bug in PathCompactPath: %d = %c\n" ), 
            i, szCompactPath[ i ] );
      }
   }
#endif

   return pszCompactPath;
}


/**
 * Check for write-protected floppy. There is,
 * unfortunately, no good way to do this, except
 * to attempt to create a file. Opening a file
 * for writing does not fail--until you actually
 * write to it...
 */
bool isWriteProtectedDisk( LPCTSTR pszPath ) {

   assert( isGoodStringPtr( pszPath ) );

   PATHNAME szDir  = { 0 };
   _tsplitpath( pszPath, szDir, 0, 0, 0 );

   PATHNAME szTest = { 0 };

   SilentErrorMode sem;
   if ( !GetTempFileName( szDir,  _T( "te" ), 0, szTest ) ) {
      const DWORD win_error = GetLastError();
      switch ( win_error ) {
      case ERROR_WRITE_PROTECT:
         return true;
      case ERROR_NOT_READY:
         trace( _T( "isWriteProtectedDisk called when drive not ready [%s]\n" ), szDir );
         throwException( win_error );
      case ERROR_DIRECTORY:
         trace( _T( "isWriteProtectedDisk called on invalid directory name [%s]\n" ), szDir );
         throwException( win_error );
      }
   }
   verify( DeleteFile( szTest ) );
   return false;
}


/**
 * This will fail if an UNC file name and a non-UNC file name
 * refers to the same file. What about shortcuts?
 * LATER: Compare PIDLs? Is it possible to use nFileIndexLow and 
 * High in the BY_HANDLE_FILE_INFORMATION structure to determine if 
 * two files  are the same, e.g., whether the names "G:\\file.txt" 
 * and "\\server\share\file.txt" actually refer to the same file?
 */
bool areFileNamesEqual( 
   const String& strFile1, const String& strFile2 )
{
   PATHNAME szFile1 = { 0 };
   GetShortPathName( strFile1.c_str(), szFile1, dim( szFile1 ) );
   PATHNAME szFile2 = { 0 };
   GetShortPathName( strFile2.c_str(), szFile2, dim( szFile2 ) );
   return 0 == _tcsicmp( szFile1, szFile2 );
}


/**
 * SHGetSpecialFolderPath needs version 4.71 of the common controls.
 */
String getSpecialFolderLocation( int nFolder ) {

   String strSpecialFolder;
   AutoShellObject< ITEMIDLIST > pidl;
   HRESULT hres = SHGetSpecialFolderLocation(
      HWND_DESKTOP, nFolder, &pidl );
   if ( SUCCEEDED( hres ) ) {
      strSpecialFolder = getPathFromIDList( pidl );
   }

   return strSpecialFolder;
}


bool supportsCompression( const String& strPath ) {

   const SilentErrorMode sem;
   const String strRoot = getRootDir( strPath );
   DWORD dwMaximumComponentLength = 0;
   DWORD dwFileSystemFlags = 0;
   const BOOL bOK = GetVolumeInformation(
      strRoot.c_str(), 0, 0, 0,
      &dwMaximumComponentLength,
      &dwFileSystemFlags, 0, 0 );
   return bOK && (FS_FILE_COMPRESSION & dwFileSystemFlags);
}


bool compressFile( const LPCTSTR pszFile, bool bCompress ) {

   HANDLE hFile = CreateFile( pszFile, 
      GENERIC_READ_WRITE, FILE_SHARE_READ, 0, 
      OPEN_EXISTING, 0, 0 );
   if ( INVALID_HANDLE_VALUE == hFile ) {
      return false;
   }

   USHORT usCompression = bCompress ? 
      COMPRESSION_FORMAT_DEFAULT : COMPRESSION_FORMAT_NONE;
   DWORD dwBytesReturned = 0;
   
   const bool bOK = 0 != DeviceIoControl(
      hFile, FSCTL_SET_COMPRESSION, 
      &usCompression,sizeof usCompression, 0, 0,
      &dwBytesReturned, 0 );
   
   assert( 0 == dwBytesReturned );
   assert( bOK );
   verify( CloseHandle( hFile ) );
   
   return bOK;
}

// end of file
