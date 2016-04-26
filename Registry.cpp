/*
 * $Header: /Book/Registry.cpp 22    16.07.04 10:42 Oslph312 $
 * 
 * This is really a get/setProfile kind of thing.
 */

#include "precomp.h"
#include "RegKey.h"
#include "Registry.h"
#include "RegRename.h"
#include "String.h"
#include "Exception.h"
#include "formatMessage.h"


#define OLD_REGKEY_BASE1 _T( "Software\\Andersen Consulting" )
#define OLD_REGKEY_BASE2 _T( "Software\\Accenture" )
#define REGKEY_BASE      _T( "Software\\Hesselberg Consulting" )
#define APP_NAME         _T( "TextEdit" )

#define WIN_SETTINGS     _T( "Microsoft\\Windows\\CurrentVersion" )
#define CPL_SETTINGS     _T( "Control Panel\\Desktop\\WindowMetrics" ) 


// TODO: Rebrand only on installation?
class Rebrander {
public:
    Rebrander( String oldRegKeyBase, String newRegKeyBase ) {
		oldRegKeyBase += _T( "\\" ) APP_NAME;
		newRegKeyBase += _T( "\\" ) APP_NAME;
        renameRegistryItem( HKEY_CURRENT_USER, // User-specific settings
            oldRegKeyBase, newRegKeyBase );
        renameRegistryItem( HKEY_LOCAL_MACHINE, // Uninstall stuff
            oldRegKeyBase, newRegKeyBase );
    }
};


static Rebrander rebrander1( OLD_REGKEY_BASE1, REGKEY_BASE );
static Rebrander rebrander2( OLD_REGKEY_BASE2, REGKEY_BASE );


String Registry::formatKey( HKEY hkRoot, LPCTSTR pszKey ) {

   assert( isGoodStringPtr( pszKey ) );

   String strKey;
   if ( HKEY_CURRENT_USER == hkRoot || HKEY_LOCAL_MACHINE == hkRoot ) {
      if ( 0 == _tcsstr( pszKey, WIN_SETTINGS ) &&
           0 == _tcsstr( pszKey, CPL_SETTINGS ) )
      {
         strKey = REGKEY_BASE _T( "\\" ) APP_NAME;
         if ( 0 != *pszKey ) {
            strKey += _T( "\\" );
         }
      }
   }

   if ( 0 != *pszKey ) {
      strKey += pszKey;
   }
   return strKey;
}


HKEY Registry::createKey( HKEY hkRoot, LPCTSTR pszKey ) {

   const String strKey = formatKey( hkRoot, pszKey );
   HKEY hk = openFormattedKey( hkRoot, strKey.c_str(), KEY_WRITE );
   if ( 0 != hk ) {
      return hk;
   }

   DWORD dwDisposition = 0;
   const long lResult = RegCreateKeyEx( hkRoot, strKey.c_str(), 0, 0, 
      REG_OPTION_NON_VOLATILE, KEY_WRITE, 
      reinterpret_cast< LPSECURITY_ATTRIBUTES >( 0 ),
      &hk, &dwDisposition );
   return NOERROR == lResult ? hk : 0;
}


/**
 * This functions exists merely to avoid calling formatKey 
 * twice when the first openKey fails in createKey.
 */
HKEY Registry::openFormattedKey( HKEY hkRoot, LPCTSTR pszKey, DWORD dwMode ) {
   HKEY hk = 0;
   const long lResult = RegOpenKeyEx( hkRoot, pszKey, 0, dwMode, &hk );
   if ( NOERROR != lResult ) {
      trace( _T( "Unable to open registry key %s: %s\n" ), pszKey, WinException( lResult ).what() );
   }
   return NOERROR == lResult ? hk : 0;
}


HKEY Registry::openKey( HKEY hkRoot, LPCTSTR pszKey, DWORD dwMode ) {
   const String strKey = formatKey( hkRoot, pszKey );
   return openFormattedKey( hkRoot, strKey.c_str(), dwMode );
}


/**
 * @param hkRoot  HKEY_CURRENT_USER or HKEY_LOCAL_MACHINE
 * @param pszKey  Key name, e.g., "Settings\\RunMaximized"
 */
int Registry::getInt( HKEY hkRoot, LPCTSTR pszKey, LPCTSTR pszName, int nDefault ) {
   int nValue = nDefault;

   RegKey hk( openKey( hkRoot, pszKey ) );
   if ( hk.isValid() ) {
      DWORD dwSize = sizeof nValue;
      DWORD dwType = 0;
#ifdef _DEBUG
      const long lResult = 
#endif
      RegQueryValueEx( hk, pszName, 0, &dwType, reinterpret_cast< BYTE * >( &nValue ), &dwSize );
      // Check type and size for sanity:
      assert( 4 == dwSize );
      assert( REG_DWORD == dwType || ERROR_FILE_NOT_FOUND == lResult );
   }

   return nValue;
}


void Registry::setInt( 
   HKEY hkRoot, LPCTSTR pszKey, LPCTSTR pszName, int nValue ) 
{
   assert( isGoodStringPtr( pszKey  ) );
   assert( isGoodStringPtr( pszName ) );

   RegKey hk( createKey( hkRoot, pszKey ) );
   if ( hk.isValid() ) {
#ifdef _DEBUG
      const long lResult = 
#endif
      RegSetValueEx( hk, pszName, 0, REG_DWORD, reinterpret_cast< CONST BYTE * >( &nValue ), sizeof nValue );
      assert( NOERROR == lResult );
   }
}


void __cdecl Registry::setString( 
   HKEY hkRoot, LPCTSTR pszKey, LPCTSTR pszName, LPCTSTR pszFmt, ...   ) 
{
   assert( isGoodStringPtr( pszKey  ) );
   assert( isGoodStringPtr( pszName ) );
   assert( isGoodStringPtr( pszFmt  ) );

   va_list vl;
   va_start( vl, pszFmt );
   String str = formatMessageV( pszFmt, vl );
   va_end( vl );

   RegKey hk( createKey( hkRoot, pszKey ) );
   if ( hk.isValid() ) {
#ifdef _DEBUG
      const long lResult = 
#endif
      RegSetValueEx( hk, pszName, 0, REG_SZ, 
         reinterpret_cast< CONST BYTE * >( str.c_str() ), 
         (str.length() + 1) * sizeof( TCHAR ) );
      assert( NOERROR == lResult );
   }
}


void __cdecl Registry::setString2(
    HKEY hkRoot, LPCTSTR pszKey, LPCTSTR pszName, LPCTSTR pszValue)
{
    assert(isGoodStringPtr(pszKey));
    assert(isGoodStringPtr(pszName));
    assert(isGoodStringPtr(pszValue));

    String str(pszValue);

    RegKey hk(createKey(hkRoot, pszKey));
    if (hk.isValid()) {
#ifdef _DEBUG
        const long lResult =
#endif
            RegSetValueEx(hk, pszName, 0, REG_SZ,
            reinterpret_cast< CONST BYTE * >(str.c_str()),
            (str.length() + 1) * sizeof(TCHAR));
        assert(NOERROR == lResult);
    }
}


static void __cdecl setString2(
    HKEY hkRoot,
    LPCTSTR pszKey,
    LPCTSTR pszName,
    LPCTSTR pszValue);

String Registry::getString( 
   HKEY hkRoot, LPCTSTR pszKey, LPCTSTR pszName, LPCTSTR pszDefault ) 
{
   PATHNAME sz = { 0 };
   if ( 0 != pszDefault ) {
      assert( isGoodStringPtr( pszDefault ) );
      assert( isGoodStringPtr( sz  ) );
      assert( _tcsclen( pszDefault ) < dim( sz ) );
	  verify( NOERROR == _tcsncpy_s( sz, dim( sz ), pszDefault, _tcslen( pszDefault ) ) );
   }

   RegKey hk( openKey( hkRoot, pszKey ) );
   if ( hk.isValid() ) {
      DWORD dwSize = sizeof sz; // Bytes, not characters.
      DWORD dwType = 0;
#ifdef _DEBUG
      const long lResult = 
#endif
      RegQueryValueEx( hk, pszName, 0, &dwType, 
         reinterpret_cast< BYTE * >( sz ), &dwSize );
      // Note that ERROR_MORE_DATA is one possible return value.
      // REG_EXPAND_SZ?
      assert( REG_EXPAND_SZ == dwType || REG_SZ == dwType || ERROR_FILE_NOT_FOUND == lResult );
   }
   return sz;
}


bool Registry::getBlob(
   HKEY hkRoot, LPCTSTR pszKey, 
   LPCTSTR pszName, LPVOID pBlob, UINT cb ) 
{
   RegKey hk( openKey( hkRoot, pszKey ) );
   if ( hk.isValid() ) {
      DWORD dwSize = cb; // Bytes, not characters.
      DWORD dwType = 0;  // REG_BINARY expected
      const long lResult = RegQueryValueEx( hk, pszName, 0, &dwType, reinterpret_cast< BYTE * >( pBlob ), &dwSize );
      // Note that ERROR_MORE_DATA is one possible return value.
      assert( REG_BINARY == dwType );
      return NOERROR == lResult;
   }
   return false;
}


String Registry::fileTypeDescriptionFromExtension( 
   LPCTSTR pszExtension ) 
{ 
   const String strClass = 
      Registry::getString( HKEY_CLASSES_ROOT, pszExtension );
   const String strDescr = 
      Registry::getString( HKEY_CLASSES_ROOT, strClass.c_str() );
   return strDescr;
}


bool enumOpenedKeyNames( 
   HKEY hk, LPCTSTR pszKey, DWORD dwIndex, String *pstrName ) 
{
   assert( isGoodStringPtr( pszKey ) );
   assert( isGoodPtr( pstrName ) );

   PATHNAME szName  = { 0 };
   PATHNAME szClass = { 0 };
   DWORD dwNameSize = dim( szName ); // Characters, not bytes.
   DWORD dwClassSize = dim( szClass );
   FILETIME ftLastWriteTime = { 0 };
   const long lResult = RegEnumKeyEx( hk, dwIndex, szName, &dwNameSize, 0, szClass, &dwClassSize, &ftLastWriteTime );
   if ( NOERROR == lResult ) {
      assert( 0 != pstrName );
      pstrName->assign( szName );
   } else if ( ERROR_NO_MORE_ITEMS != lResult ) {
      trace( _T( "RegEnumKeyEx %s: %s\n " ), pszKey, WinException( lResult ).what() );
   }
   return NOERROR == lResult;
}


bool Registry::enumKeyNames( 
   HKEY hkRoot, LPCTSTR pszKey, DWORD dwIndex, String *pstrName ) 
{
   bool bFound = false;
   RegKey hk( openKey( hkRoot, pszKey ) );
   if ( hk.isValid() ) {
      bFound = enumOpenedKeyNames( hkRoot, pszKey, dwIndex, pstrName );
   } 
   return bFound;
}


PRIVATE bool enumOpenedValues(
   HKEY hk, LPCTSTR pszKey, DWORD dwIndex, String *pstrName ) 
{
   PATHNAME szName  = { 0 };
   DWORD dwNameSize = dim( szName ); // Characters, not bytes.
   const long lResult = RegEnumValue( 
      hk, dwIndex, szName, &dwNameSize, 0, 0, 0, 0 );
   if ( NOERROR == lResult ) {
      assert( 0 != pstrName );
      pstrName->assign( szName );
   } else if ( ERROR_NO_MORE_ITEMS != lResult ) {
      trace( _T( "RegEnumValue %s: %s\n " ), 
         pszKey, WinException( lResult ).what() );
   }
   return NOERROR == lResult;
}


bool Registry::enumValues( 
   HKEY hkRoot, LPCTSTR pszKey, DWORD dwIndex, String *pstrName ) 
{
   bool bFound = false;

   RegKey hk( openKey( hkRoot, pszKey ) );
   if ( hk.isValid() ) {
      bFound = enumOpenedValues( hkRoot, pszKey, dwIndex, pstrName );
   } 
   return bFound;
}


PRIVATE bool deleteRecursive( HKEY hkRoot, LPCTSTR pszKey ) {

    { // TODO new func -- needs this scope to close RegKey
        String strName;
        DWORD dwIndex = 0;
        RegKey hk( Registry::openFormattedKey( hkRoot, pszKey ) );
        if ( !hk.isValid() ) {
            return false;
        }
        while ( enumOpenedKeyNames( hk, pszKey, dwIndex, &strName ) ) {
            const String strKey( 
                formatMessage( _T( "%1\\%2" ), pszKey, strName.c_str() ) );
            if ( !deleteRecursive( hkRoot, strKey.c_str() ) ) {
                ++dwIndex;
            }
        }
    }

    const long lResult = RegDeleteKey( hkRoot, pszKey );
    if ( NOERROR != lResult ) {
        trace( _T( "Error deleting registry key %s: %s\n" ),
            pszKey, WinException( lResult ).what() );
    }
    return NOERROR == lResult;
}


bool Registry::deleteEntry( 
   HKEY hkRoot, LPCTSTR pszKey, LPCTSTR pszName ) 
{
   if ( 0 == pszName ) {
      const String strKey = formatKey( hkRoot, pszKey );
      return deleteRecursive( hkRoot, strKey.c_str() ); //*** EXIT PT
   }

   RegKey hk( openKey( hkRoot, pszKey, KEY_ALL_ACCESS ) );
   bool bOK = hk.isValid();
   if ( bOK ) {
      const long lResult = RegDeleteValue( hk, pszName );
      if ( NOERROR != lResult ) {
         trace( _T( "Error deleting registry value %s in %s: %s\n" ),
            pszName, pszKey, WinException( lResult ).what() );
         bOK = false;
      }
   }

   return bOK;
}

// end of file
