/*
 * $Header: /Book/devMode.cpp 8     5.09.99 13:06 Oslph312 $
 *
 * Consider including this, or some of it, in Document class? 
 * Would give document-specific printer setup.
 */

#include "precomp.h"
#include "persistence.h"
#include "devMode.h"


DEFINE_PERSISTENT_INT(    "Printer", Orientation, DMORIENT_PORTRAIT );
DEFINE_PERSISTENT_INT(    "Printer", Paper      , 0                 );
DEFINE_PERSISTENT_INT(    "Printer", Resolution , 0                 );
DEFINE_PERSISTENT_STRING( "Printer", DeviceName , ""                );


HGLOBAL getDevMode( void ) {

   HGLOBAL hDevMode = GlobalAlloc( GHND, sizeof( DEVMODE ) );
   DEVMODE *pDevMode = 
      reinterpret_cast< DEVMODE * >( GlobalLock( hDevMode ) );
   if ( 0 == pDevMode ) {
      return 0; // This is fairly OK; will get default initialization.
   }

   pDevMode->dmSize = sizeof( DEVMODE );

   const String strDeviceName = getDeviceName();
   _tcsncpy( (LPTSTR) pDevMode->dmDeviceName, 
      strDeviceName.c_str(), CCHDEVICENAME );

   pDevMode->dmOrientation = getOrientation();
   if ( DMORIENT_PORTRAIT <= pDevMode->dmOrientation && 
        pDevMode->dmOrientation <= DMORIENT_LANDSCAPE ) 
   {
      pDevMode->dmFields = DM_ORIENTATION;
   }

   pDevMode->dmPaperSize = getPaper();
   if ( DMPAPER_FIRST <= pDevMode->dmPaperSize && 
      pDevMode->dmPaperSize <= DMPAPER_LAST ) 
   {
      pDevMode->dmFields |= DM_PAPERSIZE;
   }

   pDevMode->dmYResolution = getResolution();
   if ( 0 < pDevMode->dmYResolution ) {
      pDevMode->dmFields |= DM_YRESOLUTION;
   }

   GlobalUnlock( hDevMode );
   return hDevMode;
}


void setDevMode( HGLOBAL hDevMode ) {
   
   if ( 0 != hDevMode ) {
      DEVMODE *pDevMode = 
         reinterpret_cast< DEVMODE * >( GlobalLock( hDevMode ) );
      assert( isGoodPtr( pDevMode ) );
      setDeviceName( (LPTSTR) pDevMode->dmDeviceName );
      if ( pDevMode->dmFields & DM_ORIENTATION) {
         setOrientation( pDevMode->dmOrientation );
      }
      if ( pDevMode->dmFields & DM_PAPERSIZE ) {
         setPaper( pDevMode->dmPaperSize );
      }
      if ( pDevMode->dmFields & DM_YRESOLUTION ) {
         setResolution( pDevMode->dmYResolution );
      }
      GlobalUnlock( hDevMode );
   }
}

// end of file
