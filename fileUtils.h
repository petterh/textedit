/*
 * $Header: /Cleaner/fileUtils.h 13    22.03.02 13:03 Oslph312 $
 *
 * LATER: File name class.
 */

#pragma once

#include "String.h"
#include "MenuFont.h"
#include "resource.h"
#include "winUtils.h"


#define GENERIC_READ_WRITE    (GENERIC_READ | GENERIC_WRITE)
#define FILE_SHARE_NONE 0
#define FILE_SHARE_READ_WRITE (FILE_SHARE_READ | FILE_SHARE_WRITE)

String getRootDir( const String& strPath );
bool compressFile( LPCTSTR pszFile, bool bCompress );

inline boolean isPathSeparator( TCHAR ch ) {
   return _T( '\\' ) == ch || _T( '/' ) == ch;
}

inline LPCTSTR getDefaultExtension( void ) {
   return _T( ".txt" );
}

inline UINT getDriveType( const String& strPath ) {
   const String strRootDir = getRootDir( strPath );
   return GetDriveType( strRootDir.c_str() );
}

inline bool compressFile( const String& strFile, bool bCompress ) {
   return compressFile( strFile.c_str(), bCompress );
}

HANDLE openFile( LPCTSTR pszFile, DWORD dwDesiredAccess, DWORD dwShareMode );

bool fileExists( LPCTSTR pszFile );
bool modifyAttribs( const String& strFile, DWORD dwAdd, DWORD dwRemove = 0 );
String getLongPathName( const String& strShort );
void copyFile( HANDLE hSrc, HANDLE hDst, DWORD *pdwBytes = 0 );
bool delayedRemove( const String& strPath );
DWORD getClusterSize( const String& strPath );
void addPathSeparator( String *pPath );
String& appendPathComponent( String *pPath, LPCTSTR component );
String compactPath( LPCTSTR pszPath, int nWidth, HFONT hfont = MenuFont::getFont() );
bool areFileNamesEqual( const String& strFile1, const String& strFile2 );
bool isWriteProtectedDisk( LPCTSTR pszPath );
String getSpecialFolderLocation( int nFolder );
bool supportsCompression( const String& strPath );
String getTempFileName( void );

// end of file
