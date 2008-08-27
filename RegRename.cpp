/*
 * $Header: /Book/RegRename.cpp 5     16.07.04 10:42 Oslph312 $
 *
 * The original code was posted to UseNet by Ahmed Kandeel.
 * He originally found it on http://www.frogpond.org/~markg/article/renreg.html,
 * but that link appears to be dead.
 *
 * Assumes HKEY_CURRENT_USER, so nothing *drastic* happens.
 *
 * TODO: This code has been cleaned up considerably. An improvement
 * would be to not overwrite existing keys and values.
 */

#include "precomp.h"
#include "String.h"
#include "RegKey.h"
#include "RegRename.h"


static HKEY createKey( HKEY root, LPCTSTR key ) {

    HKEY hk = 0;
    DWORD dwDisposition = 0;
    const long lResult = RegCreateKeyEx(
        root, key, 0, 0, REG_OPTION_NON_VOLATILE, KEY_WRITE,
        reinterpret_cast< LPSECURITY_ATTRIBUTES >( 0 ),
        &hk, &dwDisposition );
    return NOERROR == lResult ? hk : 0;
}


static HKEY openKey( HKEY root, LPCTSTR key ) {

    HKEY hk = 0;
    const long lResult = RegOpenKeyEx( root, key, 0, KEY_ALL_ACCESS, &hk );
    return NOERROR == lResult ? hk : 0;
}


static void moveValues( HKEY hFrom, HKEY hTo ) {

    for ( ;; ) {
        TCHAR name  [  512 ] = { 0 };
        TCHAR buffer[ 8192 ] = { 0 };

        DWORD type = 0;
        DWORD name_size   = sizeof name;
        DWORD buffer_size = sizeof buffer;

        const LONG status = RegEnumValue( hFrom, 0, name, &name_size,
            0, &type, (unsigned char *) buffer, &buffer_size );
        if ( ERROR_SUCCESS != status ) {
            assert( ERROR_NO_MORE_ITEMS == status );
            return;
        }

        verify( ERROR_SUCCESS == RegSetValueEx( hTo, name, 0, type, (unsigned char *) buffer, buffer_size ) );
        verify( ERROR_SUCCESS == RegDeleteValue( hFrom, name ) );
    }
}


static void moveRegistryKey( HKEY hFrom, HKEY hTo );

static void moveKeys( HKEY hFrom, HKEY hTo ) {

    for ( ;; ) {
        TCHAR name  [  512 ] = { 0 };
        TCHAR buffer[ 8192 ] = { 0 };

        DWORD name_size  = sizeof name;
        DWORD buffer_size = sizeof buffer;

        const LONG status = RegEnumKeyEx( hFrom, 0, name, &name_size,
            0, buffer, &buffer_size, 0 );
        if ( ERROR_SUCCESS != status ) {
            assert( ERROR_NO_MORE_ITEMS == status );
            return;
        }

        moveRegistryKey(
            RegKey( openKey  ( hFrom, name ) ),
            RegKey( createKey( hTo  , name ) ) );
        verify( ERROR_SUCCESS == RegDeleteKey( hFrom, name ) );
    }
}


static void moveRegistryKey( HKEY hFrom, HKEY hTo ) {

    if ( 0 != hFrom && 0 != hTo ) {
        moveValues( hFrom, hTo );
        moveKeys  ( hFrom, hTo );
    }
}


void renameRegistryItem( HKEY root, String& from, String& to ) {

    moveRegistryKey(
        RegKey( openKey  ( root, from.c_str() ) ),
        RegKey( createKey( root, to  .c_str() ) ) );

#ifndef NDEBUG
    const LONG status = 
#endif
    RegDeleteKey( root, from.c_str() );
    assert( ERROR_SUCCESS == status || ERROR_FILE_NOT_FOUND == status );
}

// end of file
