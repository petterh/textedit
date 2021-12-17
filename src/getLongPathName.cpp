/*
 * $Header: /Book/getLongPathName.cpp 11    11.07.01 14:48 Oslph312 $
 */

#include "precomp.h"
#include "AutoComReference.h"
#include "AutoShellObject.h"
#include "utils.h"
#include "os.h"
#include <shlobj.h>


// LATER: Create a Filename class; handle named streams properly.
String getLongPathName( const String& strShort ) {

   String strLong;

   AutoComReference< IShellFolder > pShellFolder;
   HRESULT hres = SHGetDesktopFolder( &pShellFolder );
   if ( SUCCEEDED( hres ) ) {
      ULONG ulEaten = 0;
      ULONG ulAttributes = 0;
      AutoShellObject< ITEMIDLIST > pidl;
      hres = pShellFolder->ParseDisplayName(
          HWND_DESKTOP,
          0,
          const_cast< LPWSTR >(strShort.c_str()),
          &ulEaten,
          &pidl,
          &ulAttributes );
      if ( SUCCEEDED( hres ) ) {
         strLong = getPathFromIDList( pidl );
      }
   }

   // In case of failure, keep the short name:
   // TODO: Check error code -2147467259 (0x80004005) (Unspecified error)
   if ( !SUCCEEDED( hres ) || strLong.empty() ) {
      strLong = strShort;
   }

   return strLong;
}

// end of file
