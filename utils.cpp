/*
 * $Header: /Book/utils.cpp 23    16.07.04 10:42 Oslph312 $
 */

#include "precomp.h"
#include "Exception.h"
#include "formatMessage.h"
#include "os.h"
#include "utils.h"
#include <time.h>

#ifdef _WINDOWS
#include "ClientDC.h"
#endif //_WINDOWS


void comFreeImpl( LPVOID pObject ) {
   LPMALLOC lpMalloc = 0;
   const HRESULT hResult = CoGetMalloc( 1, &lpMalloc );
   if ( SUCCEEDED( hResult ) ) {
      lpMalloc->Free( pObject );  
      lpMalloc->Release();
   } else {
      trace( _T( "CoGetMalloc failed\n" ) );
   }
}


typedef BOOL (WINAPI *GETPATHFROMIDLISTWPROC) ( \
   LPCITEMIDLIST pidl, LPWSTR pwszPath);


PRIVATE BOOL getPathFromIDListA( 
   LPCITEMIDLIST pidl, LPWSTR pwszPath ) 
{
   PATHNAMEA szAnsiPath = { 0 };
   const BOOL bOK = SHGetPathFromIDListA( pidl, szAnsiPath );
   if ( bOK ) {
      multiByteToWideChar( szAnsiPath, pwszPath, MAX_PATH );
   }
   return bOK;
}


PRIVATE BOOL getPathFromIDListW( 
   LPCITEMIDLIST pidl, LPWSTR pwszPath ) 
{
   HMODULE hmodule = GetModuleHandle( _T( "SHELL32" ) );
   GETPATHFROMIDLISTWPROC fGetGetPathFromIDListW = 
      reinterpret_cast< GETPATHFROMIDLISTWPROC >(
         GetProcAddress( hmodule, "SHGetPathFromIDListW" ) );
   if ( 0 == fGetGetPathFromIDListW ) {
      trace( 
         _T( "Unable to GetProcAddress(SHGetPathFromIDListW): %s\n" ),
         WinException().what() );
      return getPathFromIDListA( pidl, pwszPath );
   }
   return fGetGetPathFromIDListW( pidl, pwszPath );
}


/**
 * Wraps the GetPathFromIDLIst function, because this
 * gives us problems when running a Unicode application
 * under Windows 95.
 * The caller is responsible for the pidl.
 */
String getPathFromIDList( ITEMIDLIST *pidl ) {

   PATHNAME szPath = { 0 };
   bool bOK = 0 != getPathFromIDListW(pidl, szPath);

   if ( !bOK ) {
      throwException( _T( "Unable to get path from ID list" ) );
   }

   return szPath;
}


String loadString( UINT uiID ) {

   TCHAR szString[ 512 ] = { 0 };

   const int numChars = LoadString( 
      getModuleHandle(), uiID, szString, dim( szString ) );
   if ( 0 == numChars ) {
      trace( _T( "loadString: missing ID %u [%#x]\n" ), uiID, uiID );
#ifdef _DEBUG
      wsprintf( szString, _T( "%u [%#x]" ), uiID, uiID );
#endif
   }
   if ( dim( szString ) <= numChars ) {
	   trace( _T( "loadString: not enough room: Requires %d characters\n" ), numChars );
	   szString[ numChars - 1 ] = 0;
   }

   return szString;
}


String loadToolTip( UINT uiID ) {
   
   String strToolTip = loadString( uiID );
   const int nCR = strToolTip.find( _T( '\n' ) );
   if ( 0 <= nCR ) {
      strToolTip.erase( 0, nCR + 1 );
   }
   return strToolTip;
}


String __cdecl loadMenuDescription( UINT uiID, ... ) {

   String strMenuDescription = loadString( uiID );
   const int nCR = strMenuDescription.find( _T( '\n' ) );
   if ( 0 <= nCR ) {
      strMenuDescription.erase( nCR );
   }

   va_list vl;
   va_start( vl, uiID );
   String str = formatMessageV( strMenuDescription.c_str(), vl );
   va_end( vl );

   return str;
}


#ifdef _WINDOWS
SIZE measureString( HDC hdc, LPCTSTR psz ) {

   RECT rc = { 0 };

#ifdef _DEBUG
   const int nHeight = 
#endif
   DrawText( hdc, psz, -1, &rc, 
      DT_CALCRECT | DT_LEFT | DT_SINGLELINE );
   assert( nHeight == rc.bottom - rc.top );

   SIZE size = { rc.right - rc.left, rc.bottom - rc.top };
   return size;
}


SIZE measureString( LPCTSTR psz, HFONT hfont ) {

   RECT rc = { 0 };

   ClientDC dc;
   if ( dc.isValid() ) {
      HFONT hfontSaved = SelectFont( dc, hfont );

#ifdef _DEBUG
      const int nHeight = 
#endif
         DrawText( dc, psz, -1, &rc, 
         DT_CALCRECT | DT_LEFT | DT_SINGLELINE );
      assert( nHeight == rc.bottom - rc.top );
      SelectFont( dc, hfontSaved );
   }

   SIZE size = { rc.right - rc.left, rc.bottom - rc.top };
   return size;
}


/**
 * The computation performed here jibes with the 
 * point sizes displayed by ChooseFont.
 */
int devPointsFromPrinterPoints( int nPixels, HDC hdc ) {

   assert( 0 != hdc );
   const int nDPI = GetDeviceCaps( hdc, LOGPIXELSY );
   nPixels = MulDiv( abs( nPixels ), nDPI, 72 );
   return -nPixels;
}


int devPointsFromPrinterPoints( int nPixels ) {
   
   ClientDC dc;
   assert( dc.isValid() );
   return devPointsFromPrinterPoints( nPixels, dc );
}


/**
 * The computation performed here jibes with the 
 * point sizes displayed by ChooseFont.
 */
int printerPointsFromDevPoints( int nPoints, HDC hdc ) {

   assert( 0 != hdc );
   const int nDPI = GetDeviceCaps( hdc, LOGPIXELSY );
   nPoints = MulDiv( abs( nPoints ), 72, nDPI );
   return nPoints;
}


int printerPointsFromDevPoints( int nPoints ) {
   
   ClientDC dc;
   assert( dc.isValid() );
   return printerPointsFromDevPoints( nPoints, dc );
}
#endif // _WINDOWS


String getModuleFileName( void ) {

   PATHNAME szModuleFileName = { 0 };
   const DWORD dwLength = GetModuleFileName( 
      0, szModuleFileName, dim( szModuleFileName ) );
   if ( 0 == dwLength ) {
      throwException( _T( "Failure in GetModuleFileName" ) );
   }

   assert( dwLength == _tcsclen( szModuleFileName ) );

   return szModuleFileName;
}


void multiByteToWideChar( LPCSTR pszIn, LPWSTR pwszOut, int numChars ) {
   
   const int nLength = MultiByteToWideChar( 
      CP_ACP, 0, pszIn, numChars, pwszOut, numChars );
   if ( 0 == nLength && 0 != pszIn[ 0 ] ) {
      throwException( _T( "multiByteToWideChar" ) );
   }
   assert( nLength <= numChars );
   pwszOut[ nLength ] = 0;
}


/**
 * If the return value is false, at least 
 * one character was incompletely translated.
 */
bool wideCharToMultiByte( 
   LPCWSTR pwszIn, LPSTR pszOut, int numChars, CHAR chDefault ) 
{
   LPCSTR pszDefault = 0 == chDefault ? 0 : &chDefault;
   BOOL nDefaultUsed = FALSE;
   const int nLength = WideCharToMultiByte( 
      CP_ACP, 0, pwszIn, numChars, pszOut, numChars, 
      pszDefault, &nDefaultUsed );
   if ( 0 == nLength && 0 != pwszIn[ 0 ] ) {
      throwException( _T( "wideCharToMultiByte" ) );
   }
   assert( nLength <= numChars );
   pszOut[ nLength ] = 0;

   return 0 != nDefaultUsed;
}


int random( int nCeiling ) {
   srand( time( 0 ) );
   return rand() % nCeiling;
}


void doubleAmpersands( String *pstr ) {
   
   assert( isGoodPtr( pstr ) );

   int nPos = 0;
   while ( 0 <= (nPos = pstr->find( _T( '&' ), nPos ) ) ) {
      pstr->insert( nPos, 1, _T( '&' ) );
      nPos += 2; // Skip two &s.
   }
}


void replace( String *pstr, LPCTSTR pszPattern, LPCTSTR pszReplacement ) {

   assert( isGoodPtr( pstr ) );
   assert( 0 != _tcscmp( pszPattern, pszReplacement ) );

   const int nLength = _tcslen( pszPattern );
   int nPos = 0;
   while ( 0 <= (nPos = pstr->find( pszPattern, nPos ) ) ) {
      pstr->erase( nPos, nLength );
      pstr->insert( nPos, pszReplacement );
      nPos += _tcslen(pszReplacement);
   }
}

// end of file
