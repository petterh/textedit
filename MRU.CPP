/*
 * $Header: /Book/MRU.cpp 16    17.12.02 9:41 Oslph312 $
 */

#include "precomp.h"
#include "Registry.h"
#include "menuUtils.h"
#include "fileUtils.h"
#include "MenuFont.h"
#include "resource.h"
#include "MRU.h"
#include "persistence.h"
#include "utils.h"


DEFINE_PERSISTENT_STRING_EX( "MRU", FileName );


#ifdef _DEBUG
int MRU::s_nRefCount = 0;
#endif


MRU::MRU() : m_bDirty( false ), m_nCount( 0 ) {

   // Sanity check -- these *must* be consecutive:
   assert( ID_MRU_2  == ID_MRU_1 + 1 );
   assert( ID_MRU_3  == ID_MRU_2 + 1 );
   assert( ID_MRU_4  == ID_MRU_3 + 1 );
   assert( ID_MRU_5  == ID_MRU_4 + 1 );
   assert( ID_MRU_6  == ID_MRU_5 + 1 );
   assert( ID_MRU_7  == ID_MRU_6 + 1 );
   assert( ID_MRU_8  == ID_MRU_7 + 1 );
   assert( ID_MRU_9  == ID_MRU_8 + 1 );
   assert( ID_MRU_10 == ID_MRU_9 + 1 );

   m_nCount = 0;
   while ( m_nCount < FILES_TO_REMEMBER ) {
      m_astrEntries[ m_nCount ] = getFileName( m_nCount );
      if ( m_astrEntries[ m_nCount ].empty() ) {
         break; //*** LOOP EXIT POINT
      }
      ++m_nCount;
   }

   assert( 1 == ++s_nRefCount );
}


MRU::~MRU() {

   assert( m_nCount <= FILES_TO_REMEMBER );
   if ( m_bDirty ) {
      for ( int iFile = 0; iFile < FILES_TO_REMEMBER; ++iFile ) {
         const LPCTSTR pszEntry = iFile < m_nCount 
            ? m_astrEntries[ iFile ].c_str() : _T( "" );
         setFileName( iFile, pszEntry );
      }
   }
   assert( 0 == --s_nRefCount );
}


bool MRU::findFile( const String& strFile, int *pnIndex ) const {

   assert( isGoodPtr( pnIndex ) );

   const LPCTSTR pszFile = strFile.c_str();
   for ( *pnIndex = 0; *pnIndex < m_nCount; ++*pnIndex ) {
      const LPCTSTR pszEntry = m_astrEntries[ *pnIndex ].c_str();
      if ( 0 == _tcsicmp( pszFile, pszEntry ) ) {
         return true; //*** FUNCTION EXIT POINT
      }
   }

   return false; // Not found.
}


void MRU::addFile( const String& strFile ) {

   m_bDirty = true;

   // If the file is already in the list, move it to the top:
   int nFile = 0;
   if ( findFile( strFile, &nFile ) ) {
      // Move preceding items one step down:
      for ( int iFile = nFile; 0 < iFile; --iFile ) {
         m_astrEntries[ iFile ] = m_astrEntries[ iFile - 1 ];
      }
   } else { // It was not in the list, so append it.
      assert( m_nCount <= FILES_TO_REMEMBER ); 
      for ( int iFile = m_nCount; 0 < iFile; --iFile ) {
         
         // Move all items one step down, letting the last  item
         // fall off the bottom if the list was already full:
         if ( iFile < FILES_TO_REMEMBER - 1 ) {
            m_astrEntries[ iFile ] = m_astrEntries[ iFile - 1 ];
         }
      }
      
      if ( m_nCount < FILES_TO_REMEMBER ) {
         ++m_nCount;
      }
   }

   m_astrEntries[ 0 ] = strFile;
   SHAddToRecentDocs( SHARD_PATH, strFile.c_str() );
}


void MRU::removeFile( const String& strFile ) {
   
   int nFile = 0;
   if ( findFile( strFile, &nFile ) ) {
      m_bDirty = true;
      assert( 0 < m_nCount );
      assert( m_nCount <= FILES_TO_REMEMBER );
      for ( int iFile = nFile; iFile < m_nCount - 1; ++iFile ) {
         assert( iFile + 1 < FILES_TO_REMEMBER );
         m_astrEntries[ iFile ] = m_astrEntries[ iFile + 1 ];
      }
      --m_nCount;
   }
}


void MRU::renameFile( const String& strOld, const String& strNew ) {

   int nFile = 0;
   if ( findFile( strOld, &nFile ) ) {
      m_bDirty = true;
      assert( nFile < m_nCount );
      assert( nFile < FILES_TO_REMEMBER );
      m_astrEntries[ nFile ] = strNew;
   }
}


PRIVATE inline LPCTSTR getFormatString( 
   int iFile, bool bShowAccelerators ) 
{
   assert( 0 <= iFile && iFile < 19 );
   return 
      1 == iFile && bShowAccelerators ? _T( "&%d. %s\tCtrl+F6" ) :
      9 <= iFile ? _T( "1&%d. %s" ) : _T( "&%d. %s" );
}


PRIVATE inline bool startsWith( 
   LPCTSTR pszString, LPCTSTR pszTarget ) 
{
   assert( isGoodConstPtr( pszString ) );
   assert( isGoodConstPtr( pszTarget ) );
   return 0 == _tcsncmp( pszString, pszTarget, _tcsclen( pszTarget ) );
}


PRIVATE bool pathRelativeToCurrent( 
   LPTSTR pszRelativePath, LPCTSTR pszPath ) 
{
   PATHNAME szCurrPath = { 0 };
   verify( 0 != _tgetdcwd( 0, szCurrPath, sizeof szCurrPath ) );
   return 0 != PathRelativePathTo( pszRelativePath, szCurrPath, 
      FILE_ATTRIBUTE_DIRECTORY, pszPath, 0 );
}


String MRU::getFileTitle( int iCmdID ) const {

   String strFile = getFile( iCmdID );

   PATHNAME szRelativePath = { 0 };
   pathRelativeToCurrent( szRelativePath, strFile.c_str() );
   if ( 0 == szRelativePath[ 0 ] ) {
      _tcscpy( szRelativePath, strFile.c_str() );
   } else if ( startsWith( szRelativePath, _T( ".\\" ) ) ) {
      // Remove current directory from path
      const int nLength = _tcslen( szRelativePath + 2 );
      memmove( szRelativePath, szRelativePath + 2, 
         sizeof( TCHAR ) * ( nLength + 1 ) );
   } else if ( startsWith( szRelativePath, _T( "..\\" ) ) ) {
      // If path is above current dir, display complete path
      _tcscpy( szRelativePath, strFile.c_str() );
   }

   return szRelativePath;
}


void MRU::addFilesToMenu( HMENU hmenu, bool bShowAccelerators ) {

   const int nCount = __min( m_nCount, FILES_TO_SHOW );

   for ( int iFile = 0; iFile < nCount; ++iFile ) {
      String strFileTitle = getFileTitle( iFile + ID_MRU_1 );
      doubleAmpersands( &strFileTitle );
      const String strCompactedFileTitle = 
         compactPath( strFileTitle.c_str(), STRING_WIDTH );
      assert( strCompactedFileTitle.length() < MAX_PATH );

      TCHAR szMenuItem[ MAX_PATH + 6 ];
      wsprintf( szMenuItem, 
         getFormatString( iFile, bShowAccelerators ), 
         (iFile + 1) % 10, 
         strCompactedFileTitle.c_str() );
      appendMenuItem( hmenu, ID_MRU_1 + iFile, szMenuItem );
   }
}

// end of file
