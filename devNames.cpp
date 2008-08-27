/*
 * $Header: /Book/devNames.cpp 13    3.06.02 10:44 Oslph312 $
 *
 * Consider including this, or some of it, in Document class? 
 * Would give document-specific printer selection.
 */

#include "precomp.h"
#include "persistence.h"
#include "devNames.h"


DEFINE_PERSISTENT_STRING( "Printer", Driver, "" );
DEFINE_PERSISTENT_STRING( "Printer", Device, "" );
DEFINE_PERSISTENT_STRING( "Printer", Port  , "" );


HGLOBAL getDevNames( 
   LPCTSTR pszPrinter, LPCTSTR pszDriver, LPCTSTR pszPort )
{  
   String strDriver = getDriver();
   String strDevice = getDevice();
   String strPort   = getPort  ();

   if ( 0 != pszPrinter ) {
      strDevice = pszPrinter;
   }
   if ( 0 != pszDriver ) {
      strDriver = pszDriver;
   }
   if ( 0 != pszPort ) {
      strPort = pszPort;
   }

   const int nChars = 
      strDriver.length() + strDevice.length() + strPort.length();
   if ( 0 == nChars ) {
      return 0; // This is OK; will get default initialization.
   }
   HGLOBAL hDevNames = GlobalAlloc( GHND, 
      sizeof DEVNAMES + sizeof( TCHAR ) * ( nChars + 4 ) );
   DEVNAMES *pDevNames = 
      reinterpret_cast< DEVNAMES * >( GlobalLock( hDevNames ) );
   if ( 0 == pDevNames ) {
      return 0; // This is OK; will get default initialization.
   }

#ifdef trace
   trace( _T( "sizeof( DEVNAMES ) = %d\n" ), sizeof( DEVNAMES ) );
#endif

   LPTSTR psz = reinterpret_cast< LPTSTR >( pDevNames ) + 
      sizeof( DEVNAMES ) / sizeof( TCHAR );
   _tcscpy( psz, strDriver.c_str() );
   pDevNames->wDriverOffset = 
      psz - reinterpret_cast< LPTSTR >( pDevNames );
   
   psz += _tcsclen( psz ) + 1;
   _tcscpy( psz, strDevice.c_str() );
   pDevNames->wDeviceOffset = 
      psz - reinterpret_cast< LPTSTR >( pDevNames );

   psz += _tcsclen( psz ) + 1;
   _tcscpy( psz, strPort.c_str() );
   pDevNames->wOutputOffset = 
      psz - reinterpret_cast< LPTSTR >( pDevNames );

   GlobalUnlock( hDevNames );
   return hDevNames;
}


void setDevNames( HGLOBAL hDevNames ) {

   if ( 0 != hDevNames ) { 
      DEVNAMES *pDevNames = 
         reinterpret_cast< DEVNAMES * >( GlobalLock( hDevNames ) );
      LPCTSTR psz = reinterpret_cast< LPCTSTR >( pDevNames ) + 
         pDevNames->wDriverOffset;
      setDriver( reinterpret_cast< LPCTSTR >( psz ) );
      psz = reinterpret_cast< LPCTSTR >( pDevNames ) + 
         pDevNames->wDeviceOffset;
      setDevice( reinterpret_cast< LPCTSTR >( psz ) );
      psz = reinterpret_cast< LPCTSTR >( pDevNames ) + 
         pDevNames->wOutputOffset;
      setPort( reinterpret_cast< LPCTSTR >( psz ) );
      GlobalUnlock( hDevNames );
   }
}

// end of file
