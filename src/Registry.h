/*
 * $Header: /Book/Registry.h 15    17.12.02 9:37 Oslph312 $
 */

#pragma once

#ifndef _REGISTRY_H_
#define _REGISTRY_H_

#include "String.h"

class Registry {
private:
   static String formatKey( HKEY hkRoot, LPCTSTR pszKey );
   static HKEY createKey( HKEY hkRoot, LPCTSTR pszKey );
   static HKEY openFormattedKey(
      HKEY hkRoot, LPCTSTR pszKey, DWORD dwMode = KEY_READ );
   static HKEY openKey(
      HKEY hkRoot, LPCTSTR pszKey, DWORD dwMode = KEY_READ );

   Registry( void ) {
   };

   friend PRIVATE bool deleteRecursive( HKEY, LPCTSTR );

public:
   static int getInt( HKEY hkRoot,
      LPCTSTR pszKey, LPCTSTR pszName, int nDefault = 0 );
   static void setInt( HKEY hkRoot,
      LPCTSTR pszKey, LPCTSTR pszName, int nValue );

   static String getString(
      HKEY hkRoot, LPCTSTR pszKey,
      LPCTSTR pszName = _T( "" ), LPCTSTR pszDefault = 0 );
   static String getString(
      HKEY hkRoot, LPCTSTR pszKey,
      LPCTSTR pszName, const String& strDefault );
   static void __cdecl setString(
      HKEY hkRoot, LPCTSTR pszKey,
      LPCTSTR pszName = _T( "" ), LPCTSTR pszFmt = _T( "" ), ... );
   static void __cdecl setString2(
       HKEY hkRoot,
       LPCTSTR pszKey,
       LPCTSTR pszName,
       LPCTSTR pszValue);

   static bool getBlob(
      HKEY hkRoot, LPCTSTR pszKey,
      LPCTSTR pszName, LPVOID pBlob, UINT cb );

   static bool enumKeyNames(
      HKEY hkRoot, LPCTSTR pszKey, DWORD dwIndex, String *pstrName );
   static bool enumValues(
      HKEY hkRoot, LPCTSTR pszKey, DWORD dwIndex, String *pstrName );

   static bool deleteEntry(
      HKEY hkRoot, LPCTSTR pszKey, LPCTSTR pszName = 0 );

   static String fileTypeDescriptionFromExtension(
      LPCTSTR pszExtension );
};


inline String Registry::getString(
   HKEY hkRoot, LPCTSTR pszKey,
   LPCTSTR pszName, const String& strDefault )
{
   return getString( hkRoot, pszKey, pszName, strDefault );
}

#endif // _REGISTRY_H_

// end of file
