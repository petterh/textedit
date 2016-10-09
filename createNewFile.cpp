/*
 * $Header: /Book/createNewFile.cpp 14    6.11.01 11:29 Oslph312 $
 */

#include "precomp.h"
#include "createNewFile.h"
#include "Exception.h"
#include "AutoHandle.h"
#include "AutoShellObject.h"
#include "formatMessage.h"
#include "resource.h"
#include "persistence.h"
#include "utils.h"
#include "fileUtils.h"
#include "winUtils.h"


/**
 * Callback function for SHBrowseForFolder.
 * If this continues to be used several places, make it common!
 */
PRIVATE int CALLBACK bff_callback( 
   HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData ) 
{
   switch ( uMsg ) {
   case BFFM_INITIALIZED:
      if ( 0 != lpData ) {
         SNDMSG( hwnd, BFFM_SETSELECTION, 1, lpData );
      }
      break;
      
   case BFFM_SELCHANGED: 
      adjustDefaultButtonStyle( GetDlgItem( hwnd, IDOK ) );
      break;
   }

   return 0;
}


/**
 * If this function fails, there is no place whatsoever to put a file!
 */
String getDefaultPath( void ) {

   return getSpecialFolderLocation( CSIDL_PERSONAL );
}


PRIVATE String getNewDocumentPath( HWND hwndParent ) {

   const String strMessage = loadString( IDS_FIND_FOLDER );
   TCHAR szDisplayName[ MAX_PATH ] = { 0 }; 
   BROWSEINFO browseInfo = {
      hwndParent, 0, szDisplayName, strMessage.c_str(), 
      BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT,
      bff_callback,
   };

   AutoShellObject< ITEMIDLIST > pidl( SHBrowseForFolder( &browseInfo ) );
   if (pidl.isNull())
   {
      trace( _T( "No default directory\n" ) );
      return String();
   }

   const String strNewDocumentPath = getPathFromIDList( pidl );
   return strNewDocumentPath;
}


// TODO -- check for writability of directory/disk!
String createNewFile( 
   HWND hwndParent, 
   create_new_file_args what_kind, 
   HANDLE hIn,
   const String& path_for_new_file,
   const String& name_of_org_file )
{
   String strNewPath;
   if ( file_for_copy == what_kind ) {
      strNewPath = path_for_new_file;
   } else {
      strNewPath = getDocumentPath( getDefaultPath().c_str() );
      if ( strNewPath.empty() ) {
         strNewPath = getNewDocumentPath( hwndParent );
         if (strNewPath.empty())
         {
             return strNewPath;
         }
      }
      assert( 0 < strNewPath.length() );

      if ( !CreateDirectory( strNewPath.c_str(), 0 ) ) {
         const DWORD win_error = GetLastError();
         if ( NOERROR != win_error && ERROR_ALREADY_EXISTS != win_error ) {
            throwException( _T( "Unable to create directory" ), win_error );
         }
      }
      setDocumentPath( strNewPath.c_str() );
   }
   addPathSeparator( &strNewPath );

   DWORD win_error = NOERROR + 1; // Anything but NOERROR.
   for ( int file_index = 1; NOERROR != win_error; ++file_index ) {

      static int anStringID[ 3 ][ 2 ] = {
         { IDS_NEW_FILE_1, IDS_NEW_FILE_N },
         { IDS_STDIN_1   , IDS_STDIN_N    },
         { IDS_COPY_1    , IDS_COPY_N     },
      };

      const String strFmt =
         loadString( anStringID[ what_kind ][ 1 < file_index ] );
      String strNewFile( strNewPath );
      strNewFile += formatMessage( strFmt, file_index, name_of_org_file.c_str() );
      
      if ( file_for_copy != what_kind ) {
         strNewFile += getDefaultExtension();
      }

      if ( file_for_copy == what_kind ) { // Don't actually create the thing
         return strNewFile; //*** FUNCTION EXIT POINT
      }

      SetLastError( NOERROR );   // For the benefit of Windows 95
      AutoHandle hNewFile( CreateFile( strNewFile.c_str(), 
         GENERIC_WRITE, FILE_SHARE_NONE, 0, 
         CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 0 ) );
      win_error = GetLastError();
      if ( INVALID_HANDLE_VALUE != hNewFile ) {
         assert( NOERROR == win_error );
#ifdef _DEBUG
         if ( NOERROR != win_error ) {
            messageBox( hwndParent, MB_OK | MB_ICONERROR, 
               _T( "Can't create %1 (%2!d!)\n'cause...: %3" ),
               strNewFile.c_str(), 
               hNewFile, 
               getError( win_error ).c_str() );
         }
#endif
         if ( 0 != hIn ) {
            copyFile( hIn, hNewFile );
         }
         strNewPath = strNewFile;
         break; //*** LOOP EXIT POINT
      } else {
         switch ( win_error ) {
         case ERROR_FILE_EXISTS:
         case ERROR_ALREADY_EXISTS:
            break; // This is the 'normal' case, where we simply
                   // try another file name.

         case ERROR_FILE_NOT_FOUND:
         case ERROR_PATH_NOT_FOUND:
         case ERROR_ACCESS_DENIED:
         case ERROR_NETWORK_ACCESS_DENIED:
         case ERROR_TOO_MANY_OPEN_FILES:
            // ...and so forth...
         default:
            throwException( _T( "Unable to create new file" ), win_error );
            assert( false ); //*** NEVER GETS HERE...
            return 0;
         }
      }
   }
   assert( NOERROR              == win_error || 
           ERROR_ALREADY_EXISTS == win_error || 
           ERROR_FILE_EXISTS    == win_error );

   return strNewPath;
}

// end of file
