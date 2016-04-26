/*
 * $Header: /Book/resolveName.cpp 15    16.07.04 10:42 Oslph312 $
 */

#include "precomp.h"
#include "os.h"
#include "utils.h"
#include "AutoComReference.h"


#define FILE_NOT_FOUND   MAKE_HRESULT( 1, FACILITY_WIN32, ERROR_FILE_NOT_FOUND )
#define PATH_NOT_FOUND   MAKE_HRESULT( 1, FACILITY_WIN32, ERROR_PATH_NOT_FOUND )
#define DEVICE_NOT_READY MAKE_HRESULT( 1, FACILITY_WIN32, ERROR_NOT_READY      )
#define ACCESS_DENIED    MAKE_HRESULT( 1, FACILITY_WIN32, ERROR_ACCESS_DENIED  )
#define INVALID_NAME     MAKE_HRESULT( 1, FACILITY_WIN32, ERROR_INVALID_NAME   )

/**
 * If pszSrc refers to a link, pszDst will be the file that 
 * the link refers to. If pszSrc does not refer to a link, 
 * pszDst will be equal to pszSrc. It is OK if these point 
 * to the same buffer. pszDst should have a length of at least 
 * MAX_PATH + 1 characters.
 */
bool resolveName( LPTSTR pszDst, LPCTSTR pszSrc ) {

   assert( isGoodStringPtr( pszDst ) );
   assert( isGoodStringPtr( pszSrc ) );

   PATHNAME szFullPathName = { 0 };
   LPTSTR pszFilePart = 0;
   const DWORD dwChars = GetFullPathName( pszSrc, dim( szFullPathName ), szFullPathName, &pszFilePart );
   if ( 0 < dwChars ) {
      pszSrc = szFullPathName;
   }

   if ( pszDst != pszSrc ) {
      _tcscpy_s( pszDst, MAX_PATH + 1, pszSrc ); // assume failure, use org file name
   }
   
   bool isShortcut = false;
   try {
      // Get a pointer to the IShellLink interface. 
      AutoComReference< IShellLink > psl( CLSID_ShellLink, IID_IShellLink );
      AutoComReference< IPersistFile > ppf( IID_IPersistFile, psl );
      
      PATHNAMEW wsz = { 0 };
      
      _tcscpy_s( wsz, pszSrc );
      
      // Load the shortcut. Fails if non-existent or non-link.
      // Succeeds if empty file, so we check for this below.
      HRESULT hres = ppf->Load( wsz, STGM_READ );
      if ( !SUCCEEDED( hres ) ) {
         // TODO: Handle DEVICE_NOT_READY!
         // Possibly conditionally; a message box may not be appropriate
         // under all circumstances.
         assert(
             E_FAIL           == hres ||
             FILE_NOT_FOUND   == hres ||
             PATH_NOT_FOUND   == hres ||
             DEVICE_NOT_READY == hres ||
             INVALID_NAME     == hres ||
             ACCESS_DENIED    == hres );
      } else {
         // Resolve the link. 
         hres = psl->Resolve( HWND_DESKTOP, SLR_ANY_MATCH ); 
         if ( SUCCEEDED( hres ) ) {
            // Get the path to the link target. 
            PATHNAME szGotPath = { 0 };
            WIN32_FIND_DATA wfd = { 0 };
            psl->GetPath( szGotPath, MAX_PATH, &wfd, SLGP_SHORTPATH );
            
            // Happens if file not found.
            if ( 0 != szGotPath[ 0 ] ) {
               isShortcut = true;
               _tcscpy_s( pszDst, MAX_PATH + 1, szGotPath );
            }
         }
      }
   }
   catch ( const ComException& x ) {
      trace( _T( "resolveName: ComException: %s\n" ), x.what() );
   }

   return isShortcut;
}

// end of file
