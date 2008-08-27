/*
 * $Header: /Book/reboot.cpp 12    6-09-01 12:52 Oslph312 $
 */

#include "precomp.h"
#include "reboot.h"
#include "utils.h"
#include "formatMessage.h"
#include "startInstance.h"
#include "fileUtils.h"
#include "winUtils.h"
#include "MRU.h"
#include "Registry.h"
#include "Document.h"
#include "Exception.h"


typedef std::map< int, String > StringMap;


inline bool enumKeyNames( DWORD dwIndex, String *pstrKey ) {
   return Registry::enumKeyNames( 
      HKEY_CURRENT_USER, _T( "Files" ), dwIndex, pstrKey );
}


/**
 * Starts a TextEdit instance for any files marked as Running.
 */
PRIVATE void startInstances( void ) {

   const MRU mru;
   const int nFiles = mru.getCount();
   for ( int iFile = 0; iFile < nFiles; ++iFile ) {
      const String strFile = mru.getFile( iFile + ID_MRU_1 );
      const LPCTSTR pszFile = strFile.c_str();
      const String strEntry = Document::createRegistryPath( strFile );
      const String strFileKey = formatMessage( 
         _T( "Files\\%1" ), strEntry.c_str() );
      const bool bRestart = 0 != Registry::getInt( 
         HKEY_CURRENT_USER, strFileKey.c_str(), _T( "Running" ), 0 );
      if ( bRestart ) {
         if ( fileExists( pszFile ) ) {
            startInstance( formatMessage( _T( "\"%1\"" ), pszFile ) );
         }
      // No reasonable way to handle a startInstance failure;
      // we're running silently now, and would prefer not to
      // bother the user over a minor matter such as not 
      // being able to restart TextEdit.
      }
   }
}


// TODO: This interferes with file opening in case -last has
// a device not ready or something. 
// Discussion: Starting new instances is a crude way of
// doing multithreading.
PRIVATE void checkExistence( void ) {

   MRU mru;

   int nFile = 0;
   while ( nFile < mru.getCount() ) {
      const String strFile = mru.getFile( nFile + ID_MRU_1 );
      if ( fileExists( strFile.c_str() ) ) {
         ++nFile;
      } else {
         mru.removeFile( strFile );
      }
   }
}


PRIVATE void deleteUnusedEntries( void ) {

   const MRU mru;
   const int nFiles = mru.getCount();

   // Delete entries in the Files registry subtree 
   // if they're missing from the MRU list:
   int nFilesToDelete = 0;
   StringMap mapOfFilesToDelete;
   String strFileEntry;
   for ( DWORD dwIndex = 0;
         enumKeyNames( dwIndex, &strFileEntry );
         ++dwIndex )
   {
      bool bExistsInMRU = false;
      for ( int iFile = 0; iFile < nFiles; ++iFile ) {
         const String strFile = mru.getFile( iFile + ID_MRU_1 );
         const String strMruEntry = 
            Document::createRegistryPath( strFile );
         if ( 0 == 
            _tcsicmp( strFileEntry.c_str(), strMruEntry.c_str() ) )
         {
            bExistsInMRU = true;
            break; //*** LOOP EXIT POINT
         }
      }
      if ( !bExistsInMRU ) {
         mapOfFilesToDelete[ nFilesToDelete++ ] = strFileEntry;
      }
   }

   for ( int iFile = 0; iFile < nFilesToDelete; ++iFile ) {
      StringMap::iterator iter = mapOfFilesToDelete.find( iFile );
      assert( mapOfFilesToDelete.end() != iter );
      if ( mapOfFilesToDelete.end() != iter ) {
         const String strKey = formatMessage( 
            _T( "Files\\%1" ), iter->second.c_str() );
         Registry::deleteEntry( HKEY_CURRENT_USER, strKey.c_str() );
      }
   }

   mapOfFilesToDelete.clear();
}


/**
 * Cleans nonexistent files from the MRU list and cleans the
 * registry of files no longer in the MRU list.
 */
void clean( void ) {
   checkExistence();
   deleteUnusedEntries();
}


/**
 * Starts a TextEdit instance for any files marked as Running,
 * then cleans up.
 */
void reboot( void ) {

   startInstances();
   clean();
}

// end of file
