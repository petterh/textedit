/*
 * $Header: /Monitor/VersionInfo.cpp 10    14.09.99 17:03 Oslph312 $
 */

#include "precomp.h"
#include "VersionInfo.h"
#include <winver.h>


#pragma comment( lib, "version.lib" )


/**
 * The pszFile parameter is non-const because the arg to 
 * GetFileVersionInfoSize is non-const. For some unfathomable reason.
 */
void VersionInfo::init( LPCTSTR pszFile ) {

   assert( isGoodStringPtr( pszFile ) );

   // VersionInfo does not have a base class, so this is OK:
   memset( this, 0, sizeof *this );

   DWORD dwVerHnd = 0;
   const DWORD dwVerInfoSize = GetFileVersionInfoSize( 
      const_cast< LPTSTR >( pszFile ), &dwVerHnd );
   if ( 0 == dwVerInfoSize ) {
      m_nErr = GetLastError();
      return;
   }

   // Allocate memory and load version information block:
   m_pInfo = GlobalAllocPtr( GPTR, dwVerInfoSize );
   if ( 0 == m_pInfo ) {
      m_nErr = ERROR_OUTOFMEMORY;
      return;
   }

   GetFileVersionInfo( const_cast< LPTSTR >( pszFile ), 
      dwVerHnd, dwVerInfoSize, m_pInfo );

   // Get number of supported languages:
   UINT uiLen = 0;
   if ( VerQueryValue( m_pInfo, _T( "\\VarFileInfo\\Translation" ),
         (void **) &m_pLanguages, &uiLen )
         && sizeof( DWORD ) <= uiLen && 0 != m_pLanguages )
   {
      m_nLanguages = (int) uiLen / sizeof( m_pLanguages[ 0 ] );
   }
}


VersionInfo::VersionInfo ( HINSTANCE hinst ) {

   PATHNAME szModuleFileName = { 0 };
   const DWORD dwLength = GetModuleFileName( 
      hinst, szModuleFileName, dim( szModuleFileName ) );
   if ( 0 < dwLength ) {
      init( szModuleFileName );
   } else {
      m_nErr = GetLastError();
   }
}


VersionInfo::~VersionInfo ( void ) {
   
   assert( 0 != m_pInfo || !isValid() );
   if ( 0 != m_pInfo ) {
      GlobalFreePtr( m_pInfo );
   }
}


LPCTSTR VersionInfo::internalGetStringFileInfo (
   LPCTSTR pszItem, LANGID uLang, UINT uChar ) const
{
   assert( isGoodStringPtr( pszItem ) );

   TCHAR szGetName[ 128 ];
   wsprintf ( szGetName, _T( "\\StringFileInfo\\%04x%04x\\%s" ),
      uLang, uChar, reinterpret_cast< LPCTSTR >( pszItem ) );

   LPTSTR psz = 0;
   UINT uiLen = 0;

   const BOOL bOK = VerQueryValue( 
      m_pInfo, szGetName, (void **) &psz, &uiLen );
   if ( !bOK ) {
      return 0;
   }

   return psz;
}


LPCTSTR VersionInfo::getStringFileInfo (
   LPCTSTR pszItem, LANGID uLang, UINT uChar ) const
{
   assert( isGoodStringPtr( pszItem ) );

   LPCTSTR psz = internalGetStringFileInfo( pszItem, uLang, uChar );
   if ( 0 == psz ) {
      psz = internalGetStringFileInfo( pszItem, 
         uLang, getCharSet() );
   }
   if ( 0 == psz ) {
      psz = internalGetStringFileInfo( pszItem, 
         getLanguage(), getCharSet() );
   }
   if ( 0 == psz ) {
      psz = _T( "?" );
   }

   return psz; 
}


bool VersionInfo::getFileVersion( 
   DWORD *pdwLow, DWORD *pdwHigh ) const 
{
   if ( 0 != pdwLow ) {
      *pdwLow = 0;
   }
   if ( 0 != pdwHigh ) {
      *pdwHigh = 0;
   }

   VS_FIXEDFILEINFO *pFixedFileInfo = 0;
   UINT uiLen = 0;
   const BOOL bOK = VerQueryValue( 
      m_pInfo, _T( "\\" ), (void **) &pFixedFileInfo, &uiLen );
   if ( !bOK ) {
      return false;
   }

   assert( uiLen == sizeof *pFixedFileInfo );
   if ( 0 != pdwLow ) {
      *pdwLow = pFixedFileInfo->dwFileVersionLS;
   }
   if ( 0 != pdwHigh ) {
      *pdwHigh = pFixedFileInfo->dwFileVersionMS;
   }

   return true;
}

// end of file
